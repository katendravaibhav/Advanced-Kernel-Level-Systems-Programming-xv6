// Shell.
#include "types.h"
#include "user.h"
#include "fcntl.h"

// Parsed command representation
#define EXEC  1
#define REDIR 2
#define PIPE  3
#define LIST  4
#define BACK  5

#define MAXARGS 10

// #ifndef USERNAME
// #define USERNAME "defaultuser"
// #endif

// #ifndef PASSWORD
// #define PASSWORD "defaultpass"
// #endif
// #define MAX_ATTEMPTS 3 

// // Function to get user input
// void get_input(char *buf, int size) {
//   gets(buf, size);
//   int len = strlen(buf);
//   if (len > 0 && buf[len - 1] == '\n') 
//       buf[len - 1] = '\0';  // Remove newline character
// }

// // Function to handle login authentication
// void login() {
//   char uname[20], pass[20];
//   int attempts = MAX_ATTEMPTS;

//   while (attempts > 0) {
//       printf(1, "Enter username: ");
//       get_input(uname, sizeof(uname));
//         // Decrease attempt count (username check also counts)

//       if (strcmp(uname, USERNAME) != 0) {
//          attempts--;
//           printf(1, "Invalid Username. Attempts left: %d\n", attempts);
//           continue;
//       }

//       printf(1, "Enter password: ");
//       get_input(pass, sizeof(pass));

//       if (strcmp(pass, PASSWORD) == 0) {
//           printf(1, "Login successful\n");
//           return;  // Proceed to shell
//       } else {
//           attempts--;  // Decrease attempt count for wrong password
//           printf(1, "Incorrect password. Attempts left: %d\n", attempts);
//       }
//   }

//   printf(1, "Login failed. Access denied.\n");
//   while (1);  // Infinite loop to prevent access
// }

struct cmd {
  int type;
};

struct execcmd {
  int type;
  char *argv[MAXARGS];
  char *eargv[MAXARGS];
};

struct redircmd {
  int type;
  struct cmd *cmd;
  char *file;
  char *efile;
  int mode;
  int fd;
};

struct pipecmd {
  int type;
  struct cmd *left;
  struct cmd *right;
};

struct listcmd {
  int type;
  struct cmd *left;
  struct cmd *right;
};

struct backcmd {
  int type;
  struct cmd *cmd;
};

int fork1(void);  // Fork but panics on failure.
void panic(char*);
struct cmd *parsecmd(char*);

// Execute cmd.  Never returns.
void
runcmd(struct cmd *cmd)
{
  int p[2];
  struct backcmd *bcmd;
  struct execcmd *ecmd;
  struct listcmd *lcmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;

  if(cmd == 0)
    exit();

  switch(cmd->type){
  default:
    panic("runcmd");

  case EXEC:
    ecmd = (struct execcmd*)cmd;
    if(ecmd->argv[0] == 0)
      exit();
    exec(ecmd->argv[0], ecmd->argv);
    printf(2, "exec %s failed\n", ecmd->argv[0]);
    break;

  case REDIR:
    rcmd = (struct redircmd*)cmd;
    close(rcmd->fd);
    if(open(rcmd->file, rcmd->mode) < 0){
      printf(2, "open %s failed\n", rcmd->file);
      exit();
    }
    runcmd(rcmd->cmd);
    break;

  case LIST:
    lcmd = (struct listcmd*)cmd;
    if(fork1() == 0)
      runcmd(lcmd->left);
    wait();
    runcmd(lcmd->right);
    break;

  case PIPE:
    pcmd = (struct pipecmd*)cmd;
    if(pipe(p) < 0)
      panic("pipe");
    if(fork1() == 0){
      close(1);
      dup(p[1]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->left);
    }
    if(fork1() == 0){
      close(0);
      dup(p[0]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->right);
    }
    close(p[0]);
    close(p[1]);
    wait();
    wait();
    break;

  case BACK:
    bcmd = (struct backcmd*)cmd;
    if(fork1() == 0)
      runcmd(bcmd->cmd);
    break;
  }
  exit();
}

int
getcmd(char *buf, int nbuf)
{
  printf(2, "$ ");
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}
void builtin_history(void) {
  int ret = gethistory();
  if(ret < 0)
      printf(2, "Error getting history\n");
}
// Function to handle the built-in block command
void builtin_block(char *buf) {
  // Expected command format: "block <syscall_id>"
  // Skip the first 6 characters ("block ") to get the syscall id as a string
  int syscall_id = atoi(buf + 6);
  int ret = block(syscall_id);  // Call the user-level wrapper for sys_block
  if(ret == 0)
      printf(1, "Blocked syscall %d\n", syscall_id);
  else
      printf(2, "Error blocking syscall %d\n", syscall_id);
}

// Function to handle the built-in unblock command
void builtin_unblock(char *buf) {
  // Expected command format: "unblock <syscall_id>"
  // Skip the first 8 characters ("unblock ") to get the syscall id as a string
  int syscall_id = atoi(buf + 8);
  int ret = unblock(syscall_id);  // Call the user-level wrapper for sys_unblock
  if(ret == 0)
      printf(1, "Unblocked syscall %d\n", syscall_id);
  else
      printf(2, "Error unblocking syscall %d\n", syscall_id);
}
// Simple strtok implementation for xv6.
// Note: This version uses a static pointer to keep track of the next token.
static char *next_token = 0;

char* strtok_xv6(char *s, const char *delim) {
  int i;
  if(s != 0)
    next_token = s;
  if(next_token == 0 || *next_token == '\0')
    return 0;
  
  char *token = next_token;
  while(*next_token != '\0'){
    for(i = 0; delim[i] != '\0'; i++){
      if(*next_token == delim[i]){
        *next_token = '\0';
        next_token++;
        return token;
      }
    }
    next_token++;
  }
  next_token = 0;
  return token;
}

void builtin_chmod(char *buf) {
  char *cmd,*filename, *modeStr;
  int mode;

  cmd = strtok_xv6(buf, " \n");        // first token: "chmod"
  filename = strtok_xv6(0, " \n");       // second token: filename
  modeStr = strtok_xv6(0, " \n");        // third token: mode string

  // Debug prints:
  //printf(1, "cmd = %s, filename = %s, modeStr = %s\n", cmd, filename ? filename : "NULL", modeStr ? modeStr : "NULL");
  cmd=cmd?cmd:"NULL";

  if(filename == 0 || modeStr == 0) {
      printf(2, "Usage: chmod filename mode\n");
      return;
  }
  mode = atoi(modeStr);
  int ret = chmod(filename, mode);
  if(ret < 0)
      printf(2, "Operation failed\n");
  // else
  //     printf(1, "chmod succeeded on %s\n", filename);
}


// A simple implementation of strncmps
int my_strncmp(const char *s1, const char *s2, int n) {
  int i;
  for(i = 0; i < n; i++){
      if(s1[i] != s2[i])
          return (unsigned char)s1[i] - (unsigned char)s2[i];
      if(s1[i] == '\0')
          return 0;
  }
  return 0;
}

int
main(void)
{
  static char buf[100];
  int fd;

  // Ensure that three file descriptors are open.
  while((fd = open("console", O_RDWR)) >= 0){
    if(fd >= 3){
      close(fd);
      break;
    }
  }
 // login();
  // Read and run input commands.
  while(getcmd(buf, sizeof(buf)) >= 0){
    if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
      // Chdir must be called by the parent, not the child.
      buf[strlen(buf)-1] = 0;  // chop \n
      if(chdir(buf+3) < 0)
        printf(2, "cannot cd %s\n", buf+3);
      continue;
    }
    if(my_strncmp(buf, "history", 7) == 0){
      builtin_history();
      continue;
    }
    // Built-in block command: expect usage "block <syscall_id>"
    if(my_strncmp(buf, "block", 5) == 0){
      builtin_block(buf);
      continue;
    }
    // Built-in unblock command: expect usage "unblock <syscall_id>"
    if(my_strncmp(buf, "unblock", 7) == 0){
        builtin_unblock(buf);
        continue;
    }
    if(my_strncmp(buf, "chmod", 5) == 0){
      builtin_chmod(buf);
      continue;
    }
    // if(my_strncmp(buf, "exit", 4) == 0){
    //   exit();
    // }
    if(fork1() == 0)
      runcmd(parsecmd(buf));
    wait();
  }
  exit();
}

void
panic(char *s)
{
  printf(2, "%s\n", s);
  exit();
}

int
fork1(void)
{
  int pid;

  pid = fork();
  if(pid == -1)
    panic("fork");
  return pid;
}

//PAGEBREAK!
// Constructors

struct cmd*
execcmd(void)
{
  struct execcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = EXEC;
  return (struct cmd*)cmd;
}

struct cmd*
redircmd(struct cmd *subcmd, char *file, char *efile, int mode, int fd)
{
  struct redircmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = REDIR;
  cmd->cmd = subcmd;
  cmd->file = file;
  cmd->efile = efile;
  cmd->mode = mode;
  cmd->fd = fd;
  return (struct cmd*)cmd;
}

struct cmd*
pipecmd(struct cmd *left, struct cmd *right)
{
  struct pipecmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = PIPE;
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

struct cmd*
listcmd(struct cmd *left, struct cmd *right)
{
  struct listcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = LIST;
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

struct cmd*
backcmd(struct cmd *subcmd)
{
  struct backcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = BACK;
  cmd->cmd = subcmd;
  return (struct cmd*)cmd;
}
//PAGEBREAK!
// Parsing

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>&;()";

int
gettoken(char **ps, char *es, char **q, char **eq)
{
  char *s;
  int ret;

  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  if(q)
    *q = s;
  ret = *s;
  switch(*s){
  case 0:
    break;
  case '|':
  case '(':
  case ')':
  case ';':
  case '&':
  case '<':
    s++;
    break;
  case '>':
    s++;
    if(*s == '>'){
      ret = '+';
      s++;
    }
    break;
  default:
    ret = 'a';
    while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
      s++;
    break;
  }
  if(eq)
    *eq = s;

  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return ret;
}

int
peek(char **ps, char *es, char *toks)
{
  char *s;

  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return *s && strchr(toks, *s);
}

struct cmd *parseline(char**, char*);
struct cmd *parsepipe(char**, char*);
struct cmd *parseexec(char**, char*);
struct cmd *nulterminate(struct cmd*);

struct cmd*
parsecmd(char *s)
{
  char *es;
  struct cmd *cmd;

  es = s + strlen(s);
  cmd = parseline(&s, es);
  peek(&s, es, "");
  if(s != es){
    printf(2, "leftovers: %s\n", s);
    panic("syntax");
  }
  nulterminate(cmd);
  return cmd;
}

struct cmd*
parseline(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parsepipe(ps, es);
  while(peek(ps, es, "&")){
    gettoken(ps, es, 0, 0);
    cmd = backcmd(cmd);
  }
  if(peek(ps, es, ";")){
    gettoken(ps, es, 0, 0);
    cmd = listcmd(cmd, parseline(ps, es));
  }
  return cmd;
}

struct cmd*
parsepipe(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parseexec(ps, es);
  if(peek(ps, es, "|")){
    gettoken(ps, es, 0, 0);
    cmd = pipecmd(cmd, parsepipe(ps, es));
  }
  return cmd;
}

struct cmd*
parseredirs(struct cmd *cmd, char **ps, char *es)
{
  int tok;
  char *q, *eq;

  while(peek(ps, es, "<>")){
    tok = gettoken(ps, es, 0, 0);
    if(gettoken(ps, es, &q, &eq) != 'a')
      panic("missing file for redirection");
    switch(tok){
    case '<':
      cmd = redircmd(cmd, q, eq, O_RDONLY, 0);
      break;
    case '>':
      cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
      break;
    case '+':  // >>
      cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
      break;
    }
  }
  return cmd;
}

struct cmd*
parseblock(char **ps, char *es)
{
  struct cmd *cmd;

  if(!peek(ps, es, "("))
    panic("parseblock");
  gettoken(ps, es, 0, 0);
  cmd = parseline(ps, es);
  if(!peek(ps, es, ")"))
    panic("syntax - missing )");
  gettoken(ps, es, 0, 0);
  cmd = parseredirs(cmd, ps, es);
  return cmd;
}

struct cmd*
parseexec(char **ps, char *es)
{
  char *q, *eq;
  int tok, argc;
  struct execcmd *cmd;
  struct cmd *ret;

  if(peek(ps, es, "("))
    return parseblock(ps, es);

  ret = execcmd();
  cmd = (struct execcmd*)ret;

  argc = 0;
  ret = parseredirs(ret, ps, es);
  while(!peek(ps, es, "|)&;")){
    if((tok=gettoken(ps, es, &q, &eq)) == 0)
      break;
    if(tok != 'a')
      panic("syntax");
    cmd->argv[argc] = q;
    cmd->eargv[argc] = eq;
    argc++;
    if(argc >= MAXARGS)
      panic("too many args");
    ret = parseredirs(ret, ps, es);
  }
  cmd->argv[argc] = 0;
  cmd->eargv[argc] = 0;
  return ret;
}

// NUL-terminate all the counted strings.
struct cmd*
nulterminate(struct cmd *cmd)
{
  int i;
  struct backcmd *bcmd;
  struct execcmd *ecmd;
  struct listcmd *lcmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;

  if(cmd == 0)
    return 0;

  switch(cmd->type){
  case EXEC:
    ecmd = (struct execcmd*)cmd;
    for(i=0; ecmd->argv[i]; i++)
      *ecmd->eargv[i] = 0;
    break;

  case REDIR:
    rcmd = (struct redircmd*)cmd;
    nulterminate(rcmd->cmd);
    *rcmd->efile = 0;
    break;

  case PIPE:
    pcmd = (struct pipecmd*)cmd;
    nulterminate(pcmd->left);
    nulterminate(pcmd->right);
    break;

  case LIST:
    lcmd = (struct listcmd*)cmd;
    nulterminate(lcmd->left);
    nulterminate(lcmd->right);
    break;

  case BACK:
    bcmd = (struct backcmd*)cmd;
    nulterminate(bcmd->cmd);
    break;
  }
  return cmd;
}
