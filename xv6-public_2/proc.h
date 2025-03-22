#ifndef PROC_H
#define PROC_H

// Per-CPU state
struct cpu {
  uchar apicid;                // Local APIC ID
  struct context *scheduler;   // swtch() here to enter scheduler
  struct taskstate ts;         // Used by x86 to find stack for interrupt
  struct segdesc gdt[NSEGS];   // x86 global descriptor table
  volatile uint started;       // Has the CPU started?
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?
  struct proc *proc;           // The process running on this cpu or null
};

extern struct cpu cpus[NCPU];
extern int ncpu;

//PAGEBREAK: 17
// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
struct context {
  uint edi;
  uint esi;
  uint ebx;
  uint ebp;
  uint eip;
};

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };
/*
EMBRYO : The kernel is in the process of creating a new process.
It hasn't been fully initialized yet.
In allocproc(), right after finding a free slot:
p->state = EMBRYO;
It means : Hey, I’ve grabbed a slot and I’m working on setting up its memory, stack, trapframe, etc

SLEEPING : The process is blocked, waiting for an event to happen (I/O, pipe, timer, child exit...).
It’s not runnable, even if the CPU is free.
When you call sleep(chan, lock) in xv6, your process becomes SLEEPING, and is parked on the chan.
It's woken up by wakeup(chan) when the event occurs.

RUNNABLE : The process is ready to run, but the CPU hasn't picked it yet.
It's in the run queue.When wakeup() is called on a sleeping process, it becomes RUNNABLE.
The scheduler (scheduler() in proc.c) picks a RUNNABLE process and gives it CPU

RUNNING : The process is currently running on the CPU.
The scheduler sets a process’s state to RUNNING before switching to it : p->state = RUNNING;


ZOMBIE : The process has exited, but its parent hasn't called wait() yet.
Its resources (e.g., memory) are freed, but the proc slot is still there so the parent can read its exit status.
In exit(), the process becomes a ZOMBIE.
In wait(), the parent finds child processes in ZOMBIE state and reaps them, then sets them to UNUSED

*/

// Per-process state
struct proc {
  uint sz;                     // Size of process memory (bytes)
  pde_t* pgdir;                // Page table
  char *kstack;                // Bottom of kernel stack for this process
  enum procstate state;        // Process state
  int pid;                     // Process ID
  struct proc *parent;         // Parent process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)

  ////////////////
  /* C+B*/
  int backgrounded;  // 1 if marked for backgrounding, 0 otherwise
  ////////////////

};

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap

#endif // PROC_H
