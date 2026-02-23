#include "ft_ping.h"

void print_usage();

int main(int ac, char **av) {
  struct address addr = {0};
  struct flags flags = {0};
  if (parse(ac, av, &addr, &flags))
    return -1;

  if (flags.help.present) {
    print_usage();
    return 0;
  }

  __uint16_t sequence = 0;
  float packets_received[MAX_ICMP_SENDS] = {0};
  if (loop(&addr, &flags, &sequence, packets_received))
    return -1;

  if (print_summary(&addr, sequence, packets_received))
    return -1;
  return 0;
}

void print_usage() {
  printf("Usage: ping [OPTION...] HOST ...\n"
         "Send ICMP ECHO_REQUEST packets to network hosts.\n\n"
         "Options\n"
         "      --ttl=N                specify N as time-to-live\n"
         "      -v,                    verbose output\n"
         "      -?,                    give this help list\n");
}
