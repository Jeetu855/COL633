#ifndef HISTORY_STRUCT_H
#define HISTORY_STRUCT_H

#define MAX_LIMIT 64  


struct history_struct {
  int pid;
  char name[16];      // Process name 
  int totalMemory;    // Total memory usage in B
  int creationTime;   // increase count at process creation
};

extern struct history_struct hist_arr[MAX_LIMIT];
extern int hist_count;
extern struct spinlock hist_lock;

#endif  // HISTORY_STRUCT_H
