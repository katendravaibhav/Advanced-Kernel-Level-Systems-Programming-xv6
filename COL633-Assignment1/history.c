#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  // Call the system call. Our kernel function prints the history.
  int ret = gethistory();
  if(ret < 0)
    printf(2, "Error getting history\n");
  exit();
}
