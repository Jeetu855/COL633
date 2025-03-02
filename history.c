#include "user.h"

int main(void) {
  int count = gethistory();
  if(count < 0)
    printf(1, "Error retrieving history\n");
  // The gethistory system call already prints the history entries.
  // Optionally, you could print "Total entries: %d\n", count.
  exit();
}
