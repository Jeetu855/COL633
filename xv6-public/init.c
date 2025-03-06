// init: The initial user-level program

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char *argv[] = { "sh", 0 };


//  part 1 : login---------------------------------
void
login(void)
{
    char uname[32], pword[32];
    int len;
    int attempts = 0;  

    while (attempts < 3) {
        // Ask for username
        printf(1, "Enter username: ");
        gets(uname, sizeof(uname));
        len = strlen(uname);
        if (len > 0 && uname[len - 1] == '\n')
            uname[len - 1] = '\0';

        
        if (strcmp(uname, USERNAME) != 0) {
            printf(1, "Incorrect username.\n");
            attempts++;  
            continue;  
        }

        // Ask for password
        printf(1, "Enter password: ");
        gets(pword, sizeof(pword));
        len = strlen(pword);
        if (len > 0 && pword[len - 1] == '\n')
            pword[len - 1] = '\0';

        
        if (strcmp(pword, PASSWORD) == 0) {
            return;  
        }

        printf(1, "Incorrect password.\n");
        attempts++;  
    }

    
    sleep(100000);
}

// -----------------------------


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
