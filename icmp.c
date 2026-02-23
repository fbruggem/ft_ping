#include "bits/types.h"
#include "ft_ping.h"

__uint16_t checksum(void *buf, size_t len) {
  assert(buf != NULL);
  __uint16_t *buf_16 = (__uint16_t *)buf;
  int checksum = 0;

  for (int i = 0; i < len / 2; i++) {
    checksum += buf_16[i];
  }
  while (checksum > 0XFFFF)
    checksum = (checksum & 0xFFFF) + (checksum >> 16);

  return (__uint16_t)~checksum;
}

int packet_icmp_echo_create(struct icmp_echo *icmp_echo, __uint16_t sequence) {
  assert(icmp_echo != NULL);

  icmp_echo->type = ICMP_ECHO;
  icmp_echo->code = 0;
  icmp_echo->checksum = 0;
  icmp_echo->identifier = getpid() & 0xFFFF;
  icmp_echo->sequence = sequence;
  if (gettimeofday(&icmp_echo->sent_at, 0)) {
    perror("gettimeofday: ");
    return -1;
  }

  icmp_echo->checksum = checksum(icmp_echo, sizeof(struct icmp_echo));

  return 0;
};
