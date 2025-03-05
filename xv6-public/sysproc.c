#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"

// asignement----------
#include "syscall.h"
#include "history_struct.h" 
extern struct history_struct hist_arr[MAX_LIMIT]; // Declare the global history array
extern int hist_count;  // Declare history count
extern struct spinlock hist_lock;  // Declare history lock

int block_actual(int);
int unblock_actual(int);

extern int chmod(char *file, uint mode); // Defined in exec.c
// ---------------




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

// asignment -------------------------
int
sys_chmod(void)
{
  char *file;
  int mode;

 
  if(argstr(0, &file) < 0)
    return -1;

  if(argint(1, &mode) < 0)
    return -1;
  
    cprintf(file);



  return chmod_actual(file, mode);
}


// int
// sys_gethistory(void)
// {
//   char *user_buffer;
//   // The user passes a pointer to a buffer big enough to hold MAX_LIMIT history_struct entries.
//   if(argptr(0, &user_buffer, sizeof(struct history_struct) * MAX_LIMIT) < 0)
//       return -1;

//   acquire(&history_lock);
//   int count = historyCount;  // number of history entries recorded
//   int bytes = count * sizeof(struct history_struct);
//   // Copy the kernel's history array to the user-supplied buffer.
//   if(copyout(myproc()->pgdir, (uint)user_buffer, (char*)history_array, bytes) < 0) {
//        release(&history_lock);
//        return -1;
//   }
//   release(&history_lock);
//   return count; // Return the number of history entries copied
// }



// history ---------------------------------
int
sys_gethistory(void)
{
  char *user_buffer;

  // Retrieve the pointer from the user; the buffer must be large enough for MAX_LIMIT entries.
  if(argptr(0, &user_buffer, sizeof(struct history_struct) * MAX_LIMIT) < 0)
    return -1;

  // Call the actual implementation of gethistory.
  return gethistory_actual(user_buffer);
}

// block unblock---------
int sys_block(void) {
  int syscall_id;
  if (argint(0, &syscall_id) < 0)
      return -1;
  
  // Prevent blocking fork/exit/exec
  if (syscall_id == SYS_fork || syscall_id == SYS_exit)
      return -1;

  return block_actual(syscall_id);
}


// sys_unblock wrapper 
int 
sys_unblock(void)
{
  int syscall_id;
  
  // Get syscall ID from user space
  if(argint(0, &syscall_id) < 0)
    return -1;

  return unblock_actual(syscall_id);
}
// 