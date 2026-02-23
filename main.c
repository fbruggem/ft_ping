#include "ft_ping.h"
#include <stdio.h>

int main(int ac, char **av) {
  struct address addr = {0};
  struct flags flags = {0};
  if (parse(ac, av, &addr, &flags))
    return -1;

  if (loop_send(&addr, &flags))
    return -1;

  printf("hehehehe\n");
}
