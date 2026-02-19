#include "netinet/in.h"
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: sudo %s <IP>\n", argv[0]);
    return 1;
  }

  const char *target_ip = argv[1];

  // Create raw socket
  int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (sock < 0) {
    perror("socket");
    return 1;
  }

  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  inet_pton(AF_INET, target_ip, &addr.sin_addr);

  // Build ICMP packet
  char packet[64] = {0};
  struct icmphdr *icmp = (struct icmphdr *)packet;

  icmp->type = ICMP_ECHO; // echo request (8)
  icmp->code = 0;
  icmp->un.echo.id = getpid() & 0xFFFF;
  icmp->un.echo.sequence = 1;
  icmp->checksum = 0;
  // icmp->checksum = checksum(packet, sizeof(packet));

  printf("Sending ICMP echo to %s...\n", target_ip);

  int ahh = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
  if (ahh) {
    perror("sendto");
    close(sock);
    return 1;
  }

  ssize_t sent = send(sock, packet, sizeof(packet), 0);
  // ssize_t sent = sendto(sock, packet, sizeof(packet), 0,
  //                       (struct sockaddr *)&addr, sizeof(addr));
  if (sent <= 0) {
    perror("sendto");
    close(sock);
    return 1;
  }

  // Receive reply
  char recvbuf[1024];
  socklen_t len = sizeof(addr);
  ssize_t received = recvfrom(sock, recvbuf, sizeof(recvbuf), 0,
                              (struct sockaddr *)&addr, &len);
  if (received <= 0) {
    perror("recvfrom");
    close(sock);
    return 1;
  }

  // Check reply type
  struct icmphdr *reply = (struct icmphdr *)(recvbuf + sizeof(struct ip));
  if (reply->type == ICMP_ECHOREPLY) {
    printf("Echo reply received from %s\n", target_ip);
  } else {
    printf("Got packet type %d (expected echo reply)\n", reply->type);
  }

  close(sock);
  return 0;
  // int fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  // sockaddr_in
  //
  // struct icmp *icmp = NULL;
  // printf("hello\n");
  // return 42;
}
