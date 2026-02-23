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
#include <stddef.h>
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

  // addr->ipv4 = "127.0.0.1";
  addr->ipv4 = "142.250.180.206";
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

int checksum(void *buf, size_t len) {
  assert(buf != NULL);
  __uint16_t *buf_16 = (__uint16_t *)buf;
  int checksum = 0;

  for (int i = 0; i < len / 2; i++) {
    checksum += buf_16[i];
  }
  while (checksum > 0XFFFF)
    checksum = (checksum & 0xFFFF) + (checksum >> 16);

  return ~checksum;
}

int packet_icmp_echo_create(struct icmp_echo *icmp_echo) {
  assert(icmp_echo != NULL);

  icmp_echo->type = ICMP_ECHO;
  icmp_echo->code = 0;
  icmp_echo->checksum = 0;
  icmp_echo->identifier = getpid() & 0xFFFF;
  icmp_echo->sequence = 0;
  if (gettimeofday(&icmp_echo->sent_at, 0)) {
    perror("gettimeofday: ");
    return -1;
  }

  icmp_echo->checksum = checksum(icmp_echo, sizeof(struct icmp_echo));

  return 0;
};

void _alarm(int _a) {};

#define MSG_START "PING %s (%s): %li data bytes\n"

void signal_interrup_init() {
  struct sigaction sa = {0};
  sa.sa_handler = _alarm;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGALRM, &sa, NULL);
}

// #define MSG_LOOP "%i bytes from (%s): icmp_seq=%i ttl=%i time=%f ms"
int icmp_print(const struct iphdr *iphdr, struct icmp_echo *icmp,
               int bytes_read, struct timeval *recieved_at) {
  char str[INET_ADDRSTRLEN];
  if (inet_ntop(AF_INET, &iphdr->saddr, str, sizeof(str)) != NULL) {
  } else {
    perror("inet_ntop");
  }

  recieved_at->tv_usec += 1000000;
  recieved_at->tv_sec -= 1;

  struct timeval diff = {0};
  diff.tv_sec = recieved_at->tv_sec - icmp->sent_at.tv_sec;
  diff.tv_usec = recieved_at->tv_usec - icmp->sent_at.tv_usec;

  float diff_usec = diff.tv_usec + diff.tv_sec * 1000000;
  diff_usec /= 1000;

  printf("%i bytes from %s: icmp_seq=%i ttl=%i time=%.3f ms\n", bytes_read, str,
         icmp->sequence, iphdr->ttl, diff_usec);
  // printf(MSG_LOOP, icmp->type, icmp->code);

  return 0;
}

int loop_send(struct address *addr, struct flags *flags) {
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

  signal_interrup_init();

  char *hostname = addr->hostname == NULL ? addr->ipv4 : addr->hostname;
  printf(MSG_START, hostname, addr->ipv4, sizeof(struct icmp_echo));

  __uint8_t buf_icmp_echo[sizeof(struct icmp_echo)];
  struct icmp_echo *icmp_echo = (struct icmp_echo *)buf_icmp_echo;

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

      struct timeval recieved_at = {0};
      if (gettimeofday(&recieved_at, 0)) {
        perror("gettimeofday: ");
        return -1;
      }

      if (bytes_read < 0) {
        if (errno == EINTR) {
          break;
        }
        perror("recvfrom");
        close(socket_fd);
        return -1;
      }

      const struct iphdr *iphdr = (const struct iphdr *)buf_ip_recv;

      struct icmphdr *icmp = (struct icmphdr *)(buf_ip_recv + iphdr->ihl * 4);

      if (icmp->type == ICMP_ECHOREPLY)
        icmp_print(iphdr, (struct icmp_echo *)icmp, bytes_read, &recieved_at);
    }
  }
}

int main(int ac, char **av) {
  struct address addr = {0};
  struct flags flags = {0};
  if (parse(ac, av, &addr, &flags))
    return -1;

  if (loop_send(&addr, &flags))
    return -1;
}
