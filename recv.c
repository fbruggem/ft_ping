#include "ft_ping.h"
#include "netinet/in.h"
#include "netinet/ip_icmp.h"
#include "sys/socket.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>

int packet_recv(int socket_fd, struct sockaddr_in *sock_addr,
                float *packets_received) {
  socklen_t socket_addr_len = sizeof(struct sockaddr_in);

  while (1) {
    __uint8_t buf_ip_recv[IP_MAXPACKET];
    memset(buf_ip_recv, 0, IP_MAXPACKET);
    int bytes_read = recvfrom(socket_fd, buf_ip_recv, IP_MAXPACKET, 0,
                              (struct sockaddr *)sock_addr, &socket_addr_len);

    struct timeval recieved_at = {0};
    if (gettimeofday(&recieved_at, 0)) {
      perror("gettimeofday: ");
      return -1;
    }

    if (bytes_read < 0) {
      const bool timeout = errno == EINTR;
      if (timeout) {
        return 0;
      }
      perror("recvfrom");
      close(socket_fd);
      return -1;
    }

    const struct iphdr *iphdr = (const struct iphdr *)buf_ip_recv;

    struct icmp_echo *icmp = (struct icmp_echo *)(buf_ip_recv + iphdr->ihl * 4);

    const bool packet_from_differen_proc =
        icmp->identifier != (getpid() & 0xFFFF);
    if (packet_from_differen_proc)
      continue;
    if (icmp->type == ICMP_ECHO)
      continue;

    __uint16_t cksum = icmp->checksum;
    icmp->checksum = 0;
    const bool checksum_wrong = checksum(icmp, sizeof(struct icmp)) != cksum;
    if (checksum_wrong)
      continue;

    packets_received[icmp->sequence] =
        timeval_diff(&icmp->sent_at, &recieved_at);
    if (icmp->type == ICMP_ECHOREPLY)
      icmp_print_echo_reply(iphdr, (struct icmp_echo *)icmp, bytes_read,
                            &recieved_at);
    else
      icmp_print_err(iphdr, (struct icmp_echo *)icmp, bytes_read);
  }

  return 0;
}
