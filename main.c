#include "bits/types.h"
#include "netinet/in.h"
#include "strings.h"
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

struct address {
  char *hostname;
  char *ipv4;
  struct in_addr binary;
};

struct flags {
  struct {
    bool present;
    int value;
  } ttl;
  struct {
    bool present;
  } help;
  struct {
    bool present;
  } verbose;
};

int parse(int ac, char **av, struct address *addr, struct flags *flags) {
  assert(addr != NULL);
  assert(flags != NULL);

  addr->ipv4 = "127.0.0.1";
  inet_aton(addr->ipv4, &addr->binary);
  return 0;
}

struct icmp_echo {
  __uint8_t type;
  __uint8_t code;
  __uint16_t checksum;
  __uint16_t identifier;
  __uint16_t sequence;
  struct timeval sent_at;
};

int packet_icmp_echo_create(struct icmp_echo *icmp_echo) {
  assert(icmp_echo != NULL);

  icmp_echo->type = ICMP_ECHO;
  icmp_echo->code = 0;
  icmp_echo->checksum = 0;
  icmp_echo->identifier = 0x6969;
  icmp_echo->sequence = 0;
  if (gettimeofday(&icmp_echo->sent_at, 0)) {
    perror("gettimeofday: ");
    return -1;
  }

  return 0;
};

void _alarm(int _a) {};

int main(int ac, char **av) {
  struct address addr = {0};
  struct flags flags = {0};
  if (parse(ac, av, &addr, &flags))
    return -1;

  int socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (socket_fd < 0) {
    perror("socket");
    return -1;
  }

  struct sockaddr_in sock_addr_send = {0};
  sock_addr_send.sin_family = AF_INET;
  sock_addr_send.sin_addr = addr.binary;

  struct sockaddr_in sock_addr_recv = {0};
  sock_addr_recv.sin_family = AF_INET;
  socklen_t sock_addr_recv_len = sizeof(sock_addr_recv);

  __uint8_t buf_icmp_echo[sizeof(struct icmp_echo)];
  struct icmp_echo *icmp_echo = (struct icmp_echo *)buf_icmp_echo;

  struct sigaction sa = {0};
  sa.sa_handler = _alarm;
  sigemptyset(&sa.sa_mask);
  // do NOT set SA_RESTART — we want read to be interrupted
  sa.sa_flags = 0;
  sigaction(SIGALRM, &sa, NULL);
  while (1) {
    int bytes_to_be_send = sizeof(struct icmp_echo);
    memset(icmp_echo, 0, bytes_to_be_send);

    if (packet_icmp_echo_create(icmp_echo)) {
      close(socket_fd);
      return -1;
    }

    int bytes_send = sendto(socket_fd, icmp_echo, bytes_to_be_send, 0,
                            (const struct sockaddr *)&sock_addr_send,
                            sizeof(sock_addr_send));

    if (bytes_send < bytes_to_be_send) {
      perror("sendto");
      close(socket_fd);
      return -1;
    }

    alarm(1);
    __uint8_t buf_ip_recv[IP_MAXPACKET];
    while (1) {
      memset(buf_ip_recv, 0, IP_MAXPACKET);
      int bytes_read =
          recvfrom(socket_fd, buf_ip_recv, IP_MAXPACKET, 0,
                   (struct sockaddr *)&sock_addr_recv, &sock_addr_recv_len);

      if (bytes_read < 0) {
        if (errno == EINTR) {
          break;
        }
        perror("recvfrom");
        close(socket_fd);
        return -1;
      }

      const struct iphdr *iphdr = (const struct iphdr *)buf_ip_recv;

      const struct icmphdr *icmp =
          (const struct icmphdr *)(buf_ip_recv + iphdr->ihl * 4);

      printf("ICMP packet: type=%d code=%d, from \n", icmp->type, icmp->code);
    }
  }
}
