#ifndef HISTORY_STRUCT_H
#define HISTORY_STRUCT_H

#define MAX_LIMIT 64  // Maximum number of history entries

// Define the history tracking structure
struct history_struct {
  int pid;
  char name[16];      // Process name (16 characters for consistency with proc names)
  int totalMemory;    // Total memory usage (in bytes, approximated by proc->sz)
  int creationTime;   // Tick count at process creation
};

extern struct history_struct hist_arr[MAX_LIMIT];
extern int hist_count;
extern struct spinlock hist_lock;

#endif  // HISTORY_STRUCT_H
