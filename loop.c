
#include "bits/types.h"
#include "ft_ping.h"
#include <signal.h>
#include <stdio.h>

sig_atomic_t received_sig_int = 0;

void _alarm(int a) {
  if (a == SIGINT)
    received_sig_int = 1;
};

void signal_interrup_init() {
  struct sigaction sa = {0};
  sa.sa_handler = _alarm;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGALRM, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);
}

int loop(struct address *addr, struct flags *flags, __uint16_t *sequence,
         float *packets_received) {
  struct sockaddr_in sock_addr_send = {0};
  sock_addr_send.sin_family = AF_INET;
  sock_addr_send.sin_addr = addr->binary;

  struct sockaddr_in sock_addr_recv = {0};
  sock_addr_recv.sin_family = AF_INET;
  socklen_t sock_addr_recv_len = sizeof(sock_addr_recv);

  int socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (socket_fd < 0) {
    perror("socket");
    return -1;
  }

  if (flags->ttl.present) {
    if (setsockopt(socket_fd, IPPROTO_IP, IP_TTL, &flags->ttl.value,
                   sizeof(&flags->ttl.value)) < 0) {
      perror("setsockopt IP_TTL");
      close(socket_fd);
      return 1;
    }
  }

  msg_start_ping(addr, flags);

  signal_interrup_init();

  while (*sequence < MAX_ICMP_SENDS) {
    if (received_sig_int)
      return 0;
    alarm(1);
    if (packet_send(socket_fd, *sequence, &sock_addr_send))
      return -1;
    (*sequence)++;

    if (packet_recv(socket_fd, &sock_addr_recv, packets_received))
      return -1;
  }
  close(socket_fd);
  return 0;
}
