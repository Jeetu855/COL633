#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

//  for block part 3
#include "syscall.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


int sys_gethistory(void) {
  int i;
  // Acquire the process table lock to safely access shared data.
  acquire(&ptable.lock);

  // Loop over the history entries and print each process's details.
  for (i = 0; i < history_count; i++) {
    cprintf("%d %s %d\n", proc_history[i].pid, proc_history[i].name, proc_history[i].mem_alloc);
  }

  release(&ptable.lock);
  return history_count;  // Return the number of history entries recorded.
}
// This function iterates over a global history array (to be defined next) and prints each record clockwise
// by the order in which processes terminated. Using cprintf ensures that output appears on the kernel console, which
// will be visible as part of the shell’s output.


// block : part 3

int sys_block(void) {
    int syscall_id;
    if (argint(0, &syscall_id) < 0)
        return -1;

    if (syscall_id <= 0)
        return -1;

    // Protect critical syscalls (fork and exit)
    if (syscall_id == SYS_fork || syscall_id == SYS_exit)
        return -1;

    struct proc *p = myproc();
    // Use the origin if it exists; otherwise, use the current process.
    struct proc *target = (p->origin ? p->origin : p);
    target->blocked_mask |= (1 << syscall_id);
    return 0;
}



int sys_unblock(void) {
    int syscall_id;

    if(argint(0, &syscall_id) < 0)
        return -1;

    if(syscall_id < 1)
        return -1;

    // Protect critical syscalls
    if(syscall_id == SYS_fork || syscall_id == SYS_exit)
        return -1;

    // Get the process that owns the blocked mask.
    // If this process is a child, its origin pointer is non-NULL; in that case, use the origin.
    struct proc *p = myproc();
    struct proc *target = (p->origin ? p->origin : p);

    target->blocked_mask &= ~(1 << syscall_id);
    return 0;
}
