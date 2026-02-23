#include "ft_ping.h"

int main(int ac, char **av) {
  struct address addr = {0};
  struct flags flags = {0};
  if (parse(ac, av, &addr, &flags))
    return -1;

  __uint16_t sequence = 0;
  float packets_received[MAX_ICMP_SENDS] = {0};
  if (loop(&addr, &flags, &sequence, packets_received))
    return -1;

  if (print_summary(&addr, sequence, packets_received))
    return -1;
  return 0;
}
