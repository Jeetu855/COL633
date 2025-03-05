// init: The initial user-level program

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char *argv[] = { "sh", 0 };

void
login(void)
{
  char uname[32], pword[32];
  int i, len;

  // Loop indefinitely until a successful login.
  while(1){
    // Prompt for username.
    printf(1, "Enter username: ");
    gets(uname, sizeof(uname));
    len = strlen(uname);
    if(len > 0 && uname[len-1] == '\n')
      uname[len-1] = '\0';

    // Only proceed if username matches.
    if(strcmp(uname, USERNAME) != 0){
      printf(1, "Incorrect username.\n");
      continue;  // Restart the loop: re-prompt username.
    }

    // Username is correct; now allow three tries for the password.
    for(i = 0; i < 3; i++){
      printf(1, "Enter password: ");
      gets(pword, sizeof(pword));
      len = strlen(pword);
      if(len > 0 && pword[len-1] == '\n')
        pword[len-1] = '\0';

      if(strcmp(pword, PASSWORD) == 0){
        // printf(1, "Login successful.\n");
        return;
      }
      printf(1, "Incorrect password.\n");
    }
    
    // After three wrong password attempts, pause for 3 seconds.
    printf(1, "Maximum password attempts reached. Pausing for 3 seconds...\n");
    // sleep(30000);
    exit();
  }
}



int
main(void)
{
  int pid, wpid;

  if(open("console", O_RDWR) < 0){
    mknod("console", 1, 1);
    open("console", O_RDWR);
  }
  dup(0);  // stdout
  dup(0);  // stderr

  for(;;){
    printf(1, "init: starting sh\n");
    pid = fork();
    if(pid < 0){
      printf(1, "init: fork failed\n");
      exit();
    }
    if(pid == 0){

      // part 1 : login --------------------------
      login();
      // ----------------------------------------
      exec("sh", argv);
      printf(1, "init: exec sh failed\n");
      exit();
    }
    while((wpid=wait()) >= 0 && wpid != pid)
      printf(1, "zombie!\n");
  }
}
