#ifndef PROC_H
#define PROC_H

////////
/* C + G */
typedef void (*sighandler_t)(void);
///////

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

////////////////////
/*  C + B  C + F*/
#define SIGBG  2  // Ctrl+B signal
#define SIGFG  3  // Ctrl+F signal

#define FLAG_CTRLB  0x01  // bit 0 for Ctrl+B
#define FLAG_CTRLF  0x02  // bit 1 for Ctrl+F
///////////////////


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


enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE,CUSTOM_WAIT};



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
  ///////////////////
  /* C + B   C + F*/
  int b;
  //////////////////

  /////////////
  /* C + G */
  int pending_signal;
  sighandler_t signal_handler;
  struct trapframe *saved_tf;
  ////////////


  //////////////sched
// **** New fields for custom fork ****
int custom;          // Set to 1 if process was created using custom_fork
  
int start_later;     // If 1, do not schedule until sys_scheduler_start is called
int exec_time;       // Number of ticks the process should run (-1 means run indefinitely)
int ticks_run;       // Number of ticks the process has run so far

  // **** Scheduler profiler fields ****
  int creation_time;      // Tick when process was created
  int first_cpu_time;     // Tick when process first got the CPU (-1 if not yet scheduled)
  int finish_time;        // Tick when process finished execution
  int rtime;              // Total CPU run time (accumulated during RUNNING state)
  int context_switches;   // Number of context switches incurred
  ///////////////////////////////////////////part2.2
   int init_priority; 
   int waiting_time;
    

};

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap

#endif // PROC_H
