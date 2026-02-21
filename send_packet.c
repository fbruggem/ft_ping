
#include "bits/types.h"
#include "netinet/in.h"
#include "strings.h"
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <poll.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

struct icmp_echo {
  __uint8_t type;
  __uint8_t code;
  __uint16_t checksum;
  __uint16_t identifier;
  __uint16_t sequence;
  struct timeval sent_at;
};
uint16_t icmp_checksum(const void *buf, size_t len) {
  const uint16_t *data = buf;
  uint32_t sum = 0;

  // Sum up 16‑bit words
  while (len > 1) {
    sum += *data++;
    len -= 2;
  }

  // Fold 32‑bit sum to 16 bits: add high bits to low bits
  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  // One’s complement
  return (uint16_t)(~sum);
}
int main() {
  __uint8_t icmp_send_buf[sizeof(struct icmp_echo)] = {0};

  struct icmp_echo *icmp_send = (struct icmp_echo *)icmp_send_buf;

  icmp_send->type = ICMP_ECHO;
  icmp_send->code = 0;
  icmp_send->checksum = 0;
  icmp_send->identifier = 0x4444;
  icmp_send->sequence = 0;
  if (gettimeofday(&icmp_send->sent_at, 0)) {
    perror("gettimeofday: ");
    return -1;
  }

  // __uint16_t checksum = 0;
  // for (int i = 0; i < sizeof(struct icmp_echo) / 2; i++) {
  //   __uint32_t cur = *(((__uint16_t *)(icmp_send)) + i);
  //   checksum += cur;
  // }
  // while (checksum >> 16) {
  //   checksum = (checksum & 0xFFFF) + (checksum >> 16);
  // }

  icmp_send->checksum = icmp_checksum(icmp_send_buf, sizeof(struct icmp_echo));

  printf("icmp_send %016lx\n", *((__uint64_t *)icmp_send));
  printf("icmp_send %016lx\n", *((__uint64_t *)icmp_send + 1));
  int socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (socket_fd < 0) {
    perror("socket: ");
    return -1;
  }

  struct sockaddr_in addr = {0};
  bzero(&addr, sizeof(addr));
  // inet_aton("172.217.20.14", &addr.sin_addr);
  // inet_aton("127.0.0.1", &addr.sin_addr);
  inet_aton("192.0.2.0", &addr.sin_addr);

  addr.sin_family = AF_INET;

  // int ttl = 4;
  // if (setsockopt(socket_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
  //   perror("setsockopt IP_TTL");
  //   return -1;
  // }

  int on = 1;
  setsockopt(socket_fd, IPPROTO_IP, IP_RECVERR, &on, sizeof(on));
  if (connect(socket_fd, (const struct sockaddr *)&addr, sizeof(addr))) {
    perror("connect: ");
    return -1;
  }

  if (write(socket_fd, icmp_send_buf, sizeof(icmp_send_buf)) <
      sizeof(icmp_send_buf)) {
    perror("write: ");
    return -1;
  }

  __uint8_t ip_recv_buf[1024] = {0};
  bzero(ip_recv_buf, sizeof(ip_recv_buf));

  struct pollfd pl;
  pl.fd = socket_fd;
  pl.events = POLLIN | POLLERR | POLLHUP | POLLNVAL | POLLPRI;
  pl.revents = 0;

  printf("%i == pl.revents\n", pl.revents);
  perror("after poll");
  if (read(socket_fd, ip_recv_buf, sizeof(ip_recv_buf)) < 0) {
    perror("read: ");
    return -1;
  }
  // if (read(socket_fd, ip_recv_buf, sizeof(ip_recv_buf)) < 0) {
  //   perror("read: ");
  //   return -1;
  // }

  // if (read(socket_fd, ip_recv_buf, sizeof(ip_recv_buf)) < 0) {
  //   perror("read: ");
  //   return -1;
  // }
  printf("hello %016lx\n", *((__uint64_t *)ip_recv_buf));

  __uint16_t iphl = *(__uint8_t *)ip_recv_buf & 0x0F;
  iphl *= 4;

  struct icmp_echo *icmp_recv = (struct icmp_echo *)(ip_recv_buf + iphl);

  printf("hello %08x\n", *((__uint32_t *)(ip_recv_buf + iphl)));
  printf("hello %08x\n", *((__uint32_t *)(ip_recv_buf + iphl + 4)));
  // printf("iphl %i\n", icmp_recv->type);
  // printf("iphl %i\n", icmp_recv->sequence);
  // printf("iphl %i\n", icmp_recv->identifier);
}
