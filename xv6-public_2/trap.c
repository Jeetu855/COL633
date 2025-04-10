

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Define a magic return address constant for signal returns.
#define MAGIC_SIGRET 0xDEADBEEF

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[]; // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

void tvinit(void)
{
  int i;

  for (i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE << 3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE << 3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void idtinit(void)
{
  lidt(idt, sizeof(idt));
}

// PAGEBREAK: 41
void trap(struct trapframe *tf)
{
  if (tf->trapno == T_SYSCALL)
  {
    if (myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if (myproc()->killed)
      exit();
    return;
  }

  switch (tf->trapno)
  {
  case T_IRQ0 + IRQ_TIMER:
    if (cpuid() == 0)
    {
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE + 1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;
  // Step 9: Special handling for page faults.
  case T_PGFLT:
  {
    uint fault_addr = rcr2(); // Get the faulting address.
    // If the faulting address equals our magic signal-return value,
    // restore the saved trapframe and resume execution.
    if (fault_addr == MAGIC_SIGRET)
    {
      struct proc *p = myproc();
      if (p && p->saved_tf)
      {
        memmove(p->tf, p->saved_tf, sizeof(struct trapframe));
        kfree((char *)p->saved_tf);
        p->saved_tf = 0;
        break; // Resume execution.
      }
    }
    // Otherwise, handle this fault as a normal error (kill the process).
    cprintf("pid %d %s: trap %d err %d on cpu %d eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno, tf->err,
            cpuid(), tf->eip, fault_addr);
    myproc()->killed = 1;
    break;
  }
  default:
    if (myproc() == 0 || (tf->cs & 3) == 0)
    {
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  if (myproc() && myproc()->killed && (tf->cs & 3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  if (myproc() && myproc()->state == RUNNING &&
      tf->trapno == T_IRQ0 + IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded.
  if (myproc() && myproc()->killed && (tf->cs & 3) == DPL_USER)
    exit();

  // Step 8: Deliver the signal in trap() (just before returning to user space).
  struct proc *curproc = myproc();
  if (curproc && curproc->pending_signal && curproc->signal_handler)
  {
    // Step 8.1: Save the current trapframe into kernel memory.
    curproc->saved_tf = (struct trapframe *)kalloc();
    if (curproc->saved_tf != 0)
    {
      memmove(curproc->saved_tf, curproc->tf, sizeof(struct trapframe));
    }
    else
    {
      cprintf("send_sigcustom: kalloc failed\n");
    }

    // Step 8.2: Adjust the user stack.
    // Push MAGIC_SIGRET as the "return address".
    curproc->tf->esp -= 4;
    uint magic = MAGIC_SIGRET;
    if (copyout(curproc->pgdir, curproc->tf->esp, (char *)&magic, sizeof(uint)) < 0)
      cprintf("copyout failed in sigcustom delivery\n");

    // Step 8.3: Set EIP to point to the user-registered signal handler.
    curproc->tf->eip = (uint)curproc->signal_handler;

    // Clear the pending signal flag.
    curproc->pending_signal = 0;
  }
}