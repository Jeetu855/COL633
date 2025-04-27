#include "types.h"
#include "user.h"
#include "stat.h"

#define PAGE_SIZE 4096
#define NUM_CHILDREN 5
#define ITERATIONS 10000

void child_process(int id) {
    printf(1, "Child %d starting...\n", id);
    
    for (int i = 0; i < ITERATIONS; i++) {
        // Allocate 1 page
        char *mem = sbrk(PAGE_SIZE);
        if (mem == (char*)-1) {
            printf(1, "Child %d: sbrk failed at iteration %d\n", id, i);
            break;
        }
        
        // Touch the page
        for (int j = 0; j < PAGE_SIZE; j++) {
            mem[j] = (id + j) % 256;
        }
        
        if (i % 100 == 0) {
            printf(1, "Child %d: allocated %d pages\n", id, i+1);
            sleep(10);  // Sleep for 10 ticks periodically
        }
    }
    
    printf(1, "Child %d exiting\n", id);
    exit();
}

int main() {
    printf(1, "MemSwapTest starting...\n");
    
    for (int i = 0; i < NUM_CHILDREN; i++) {
        int pid = fork();
        if (pid < 0) {
            printf(1, "Fork failed!\n");
            exit();
        }
        if (pid == 0) { // Child
            child_process(i);
        }
    }
    
    // Parent waits for all children
    for (int i = 0; i < NUM_CHILDREN; i++) {
        wait();
    }
    
    printf(1, "MemSwapTest completed\n");
    exit();
}
