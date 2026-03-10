#include "ft_ping.h"
#include "unistd.h"
#include <stdio.h>

#define MSG_START "PING %s (%s): %li data bytes"
int msg_start_ping(struct address *addr, struct flags *flags) {
  char *hostname = addr->hostname == NULL ? addr->ipv4 : addr->hostname;
  int res = printf(MSG_START, hostname, addr->ipv4, sizeof(struct icmp_echo));
  if (res < 0) {
    perror("printf");
    return -1;
  }
  if (flags->verbose.present)
    printf(", id %04lx = %i", (long)getpid() & 0xFFFF, getpid() & 0xFFFF);
  printf("\n");

  return 0;
}

#define MSG_LOOP "%i bytes from (%s): icmp_seq=%i ttl=%i time=%.3f ms"
int icmp_print_echo_reply(const struct iphdr *iphdr, struct icmp_echo *icmp,
                          int bytes_read, struct timeval *recieved_at,
                          bool is_duplicate) {
  char str[INET_ADDRSTRLEN];
  if (inet_ntop(AF_INET, &iphdr->saddr, str, sizeof(str)) != NULL) {
  } else {
    perror("inet_ntop");
  }

  float diff = timeval_diff(&icmp->sent_at, recieved_at);

  printf(MSG_LOOP, bytes_read, str, icmp->sequence, iphdr->ttl, diff);
  if (is_duplicate)
    printf(" (DUP!)");
  printf("\n");
  // printf(MSG_LOOP, icmp->type, icmp->code);

  return 0;
}

int icmp_print_err(const struct iphdr *iphdr, struct icmp_echo *icmp,
                   int bytes_read) {

  char str[INET_ADDRSTRLEN];
  if (inet_ntop(AF_INET, &iphdr->saddr, str, sizeof(str)) != NULL) {
  } else {
    perror("inet_ntop");
  }
  printf("%i bytes from %s: ", bytes_read, str);
  switch (icmp->type) {
  case ICMP_UNREACH:
    printf("unreachable\n");
    break;
  case ICMP_SOURCEQUENCH:
    printf("source quenche\n");
    break;
  case ICMP_TIME_EXCEEDED:
    printf("time exceeded\n");
    break;
  default:
    printf("error\n");
  }
  return 0;
}

float sqrtf(float x) {
  if (x <= 0.0f) {
    return 0.0f;
  }

  float guess = x * 0.5f;
  for (int i = 0; i < 10; i++) {
    guess = 0.5f * (guess + x / guess);
  }

  return guess;
}
int print_summary(struct address *addr, __uint16_t packets_sent,
                  float *packets_received, int *duplicates_amount) {
  const char *hostname = addr->hostname ? addr->hostname : addr->ipv4;
  __uint16_t received_count = 0;

  float min = 0.0f, max = 0.0f;
  float sum = 0.0f;
  float sum_sq = 0.0f;

  for (int i = 0; i < packets_sent; i++) {
    float rtt = packets_received[i];
    if (rtt > 0.0f) {
      if (received_count == 0) {
        min = max = rtt;
      } else {
        if (rtt < min)
          min = rtt;
        if (rtt > max)
          max = rtt;
      }

      sum += rtt;
      sum_sq += rtt * rtt;
      received_count++;
    }
  }

  int lost = packets_sent - received_count;
  float loss_percent =
      (packets_sent > 0) ? ((float)lost * 100.0f) / (float)packets_sent : 0.0f;

  printf("\n--- %s ping statistics ---\n", hostname);
  printf("%u packets transmitted, %u packets received, ", packets_sent,
         received_count);

  if (*duplicates_amount != 0)
    printf("+%i duplicates, ", *duplicates_amount);

  printf("%.1f%% packet loss\n", loss_percent);

  if (received_count > 0) {
    float avg = sum / received_count;
    float variance = (sum_sq / received_count) - (avg * avg);
    float stddev = (variance > 0.0f) ? sqrtf(variance) : 0.0f;

    printf("round-trip min/avg/max/stddev = "
           "%.3f/%.3f/%.3f/%.3f ms\n",
           min, avg, max, stddev);
  }
  return 0;
}
