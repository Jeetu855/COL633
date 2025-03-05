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

#include "history_record.h"

// asignment 1 : ----------------------------
extern int chmod(char *file, uint mode); // Defined in exec.c

extern struct history_record history_log[HISTORY_MAX]; // Global array storing history
extern int history_size;  // Counter for stored entries
extern struct spinlock history_spinlock;  // Lock for synchronization
// ----------------------------------------

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

// asignment 1-------------------------
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

int sys_gethistory(void) {
  acquire(&history_lock);  // Lock access to history

  // Sorting history by creation time (Bubble Sort for simplicity)
  for (int i = 0; i < historyCount - 1; i++) {
      for (int j = 0; j < historyCount - i - 1; j++) {
          if (history_array[j].creationTime > history_array[j + 1].creationTime) {
              struct history_struct temp = history_array[j];
              history_array[j] = history_array[j + 1];
              history_array[j + 1] = temp;
          }
      }
  }

  // Print history in required format
  for (int i = 0; i < historyCount; i++) {
      cprintf("PID: %d | Name: %s | Memory: %d bytes\n",
              history_array[i].pid, history_array[i].name, history_array[i].totalMemory);
  }

  release(&history_lock);  // Unlock history access
  return historyCount;  // Return the number of processes in history
}

// ---------------------------------