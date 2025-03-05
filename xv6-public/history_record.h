#ifndef HISTORY_RECORD_H
#define HISTORY_RECORD_H

#define HISTORY_MAX 64  // Maximum recorded history entries

// Structure to hold executed process history
struct history_record {
  int process_id;
  char process_name[16];  // Keeping name length consistent with proc structure
  int memory_usage;       // Total memory usage (approximated by proc->sz)
  int start_time;         // Tick count at process creation
};

#endif  // HISTORY_RECORD_H
