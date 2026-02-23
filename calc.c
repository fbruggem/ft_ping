#include "ft_ping.h"

float timeval_diff(struct timeval *start, struct timeval *end) {

  end->tv_usec += 1000000;
  end->tv_sec -= 1;

  struct timeval diff = {0};
  diff.tv_sec = end->tv_sec - start->tv_sec;
  diff.tv_usec = end->tv_usec - start->tv_usec;

  float diff_usec = diff.tv_usec + diff.tv_sec * 1000000;
  diff_usec /= 1000;
  return diff_usec;
}
