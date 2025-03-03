#include "types.h"
#include "user.h"
#include "syscall.h"  // For SYS_* macros

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf(2, "Usage: block <syscall_id>\n");
        exit();
    }
    
    int syscall_id = atoi(argv[1]);
    
    // Validate syscall number range
    if(syscall_id <= 0) {
        printf(2, "Invalid syscall ID %d\n", syscall_id);
        exit();
    }
    
    if(block(syscall_id) < 0) {
        printf(2, "Failed to block syscall %d\n", syscall_id);
        printf(2, "Note: fork(1)/exit(2) cannot be blocked\n");
    }
    
    exit();
}
