// init: The initial user-level program

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char *argv[] = { "sh", 0 };
#ifndef USERNAME
#define USERNAME "defaultuser"
#endif

#ifndef PASSWORD
#define PASSWORD "defaultpass"
#endif
#define MAX_ATTEMPTS 3 

// Function to get user input
void get_input(char *buf, int size) {
  gets(buf, size);
  int len = strlen(buf);
  if (len > 0 && buf[len - 1] == '\n') 
      buf[len - 1] = '\0';  // Remove newline character
}

// Function to handle login authentication
void login() {
  char uname[20], pass[20];
  int attempts = MAX_ATTEMPTS;

  while (attempts > 0) {
      printf(1, "Enter username: ");
      get_input(uname, sizeof(uname));
        // Decrease attempt count (username check also counts)

      if (strcmp(uname, USERNAME) != 0) {
         attempts--;
          printf(1, "Invalid Username. Attempts left: %d\n", attempts);
          continue;
      }

      printf(1, "Enter password: ");
      get_input(pass, sizeof(pass));

      if (strcmp(pass, PASSWORD) == 0) {
          printf(1, "Login successful\n");
          return;  // Proceed to shell
      } else {
          attempts--;  // Decrease attempt count for wrong password
          printf(1, "Incorrect password. Attempts left: %d\n", attempts);
      }
  }

  printf(1, "Login failed. Access denied.\n");
  while (1);  // Infinite loop to prevent access
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
  login();
  for(;;){
    printf(1, "init: starting sh\n");
    pid = fork();
    if(pid < 0){
      printf(1, "init: fork failed\n");
      exit();
    }
    if(pid == 0){
      exec("sh", argv);
      printf(1, "init: exec sh failed\n");
      exit();
    }
    while((wpid=wait()) >= 0 && wpid != pid)
      printf(1, "zombie!\n");
  }
}
