// user/login.c
#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX_ATTEMPTS 3

// Helper function to trim the newline and carriage return characters.
void trim_newline(char *s) {
  int i = 0;
  while(s[i] != '\0'){
    if(s[i] == '\n' || s[i] == '\r'){
      s[i] = '\0';
      break;
    }
    i++;
  }
}

int main(void) {
  char username[32], password[32];
  int attempts = 0;
  int authenticated = 0;

  while(attempts < MAX_ATTEMPTS && !authenticated) {
    printf(1, "Enter Username: ");
    gets(username, sizeof(username));
    trim_newline(username);
    
    if(strcmp(username, USERNAME) != 0){
      printf(1, "Incorrect username.\n");
      attempts++;
      continue;
    }
    
    printf(1, "Enter Password: ");
    gets(password, sizeof(password));
    trim_newline(password);
    
    if(strcmp(password, PASSWORD) != 0) {
      printf(1, "Incorrect password.\n");
      attempts++;
    } else {
      authenticated = 1; 
      printf(1,"Login successful\n");
    }
  }

  if(!authenticated) {
    printf(1, "Maximum attempts reached. Login disabled.\n");
    exit();
  }
  
  // If authenticated, start the shell.
  char *args[] = { "sh", 0 };
  exec("sh", args);
  exit();
}
