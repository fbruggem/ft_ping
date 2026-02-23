#ifndef FT_PING_H
#define FT_PING_H

#include "bits/types.h"
#include "netinet/in.h"
#include "strings.h"
#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
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

struct icmp_echo {
  __uint8_t type;
  __uint8_t code;
  __uint16_t checksum;
  __uint16_t identifier;
  __uint16_t sequence;
  struct timeval sent_at;
};

extern sig_atomic_t received_sig_int;

// icmp.c
int packet_icmp_echo_create(struct icmp_echo *icmp_echo, __uint16_t sequence);

// parse.c
int parse(int ac, char **av, struct address *addr, struct flags *flags);
__uint16_t checksum(void *buf, size_t len);

#define MAX_ICMP_SENDS 0x1000
// loop.c
int loop(struct address *addr, struct flags *flags, __uint16_t *sequence,
         float *packets_received);

// send.c
int packet_send(int socket_fd, __uint16_t sequence,
                const struct sockaddr_in *sock_addr);
// recv.c
int packet_recv(int socket_fd, struct sockaddr_in *sock_addr,
                float *packets_received);

// print.c
int msg_start_ping(struct address *addr, struct flags *flags);
int icmp_print_echo_reply(const struct iphdr *iphdr, struct icmp_echo *icmp,
                          int bytes_read, struct timeval *recieved_at);
int icmp_print_err(const struct iphdr *iphdr, struct icmp_echo *icmp,
                   int bytes_read);
int print_summary(struct address *addr, __uint16_t packets_sent,
                  float *packets_received);

// calc.c
float timeval_diff(struct timeval *start, struct timeval *end);

#endif // !FT_PING_H
