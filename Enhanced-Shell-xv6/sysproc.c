#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "syscall.h"
#include "fs.h"
#include "file.h"
#include "stat.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int sys_gethistory(void) {
  //cprintf("sys_gethistory() called\n");  // Debug message
  int i;
  // Print each history entry in the order they were recorded
  for(i = 0; i < history_count; i++){
    cprintf("%d %s %d\n",
            history[i].pid,
            history[i].name,
            history[i].mem_usage);
  }
  return 0;
}

int sys_block(void) {
  int syscall_id;
  if(argint(0, &syscall_id) < 0)
      return -1;
  // Ensure that critical syscalls are not blocked:
  if(syscall_id == SYS_fork || syscall_id == SYS_exit)
      return -1;
  myproc()->blocked[syscall_id] = 1;
  return 0;
}

int sys_unblock(void) {
  int syscall_id;
  if(argint(0, &syscall_id) < 0)
      return -1;
  myproc()->blocked[syscall_id] = 0;
  return 0;
}
# include "perm.h"
int
sys_chmod(void)
{
  char *path;
  int mode;
  struct inode *ip;

  if(argstr(0, &path) < 0 || argint(1, &mode) < 0)
    return -1;
  
  begin_op();
  if((ip = namei(path)) == 0){
    end_op();
    return -1;
  }
  ilock(ip);

  // Update the in-memory inode's mode.
  ip->mode = mode & 7;  // Only the lower 3 bits are used.
  
  //cprintf("DEBUG in chmod: %d",ip->mode);
  iupdate(ip);
  iunlockput(ip);
  end_op();

  return 0;
}

