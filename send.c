#include "ft_ping.h"

int packet_send(int socket_fd, __uint16_t sequence,
                const struct sockaddr_in *sock_addr) {

  __uint8_t buf[sizeof(struct icmp_echo)] = {0};
  struct icmp_echo *icmp_echo = (struct icmp_echo *)buf;
  int bytes_to_be_send = sizeof(struct icmp_echo);
  memset(icmp_echo, 0, bytes_to_be_send);

  if (packet_icmp_echo_create(icmp_echo, sequence)) {
    close(socket_fd);
    return -1;
  }

  int bytes_send =
      sendto(socket_fd, icmp_echo, bytes_to_be_send, 0,
             (const struct sockaddr *)sock_addr, sizeof(struct sockaddr));

  if (bytes_send < bytes_to_be_send) {
    perror("sendto");
    close(socket_fd);
    return -1;
  }

  return 0;
}
