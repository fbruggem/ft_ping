#include "arpa/inet.h"
#include "ft_ping.h"
#include "unistd.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#define FLAG_TTL ttl
#define FLAG_VERBOSE v
#define FLAG_HELP ?

#define OPTION_PREFIX_SHORT "-"
#define OPTION_PREFIX_LONG "--"

#define OPTION_NAME_SHORT_HELP "?"
#define OPTION_NAME_SHORT_VERBOSE "v"

#define OPTION_NAME_LONG_TTL "ttl="

#define ERROR_MSG_OPTION_WRONG "no option like: "
#define ERROR_MSG_OPTION_ALREADY_EXISTS "option already specified: "
#define ERROR_MSG_OPTION_TTL_INVALID_NUM "ttl allowed range is 1..255"

int parse_addr(char *s, struct address *addr);
int parse_option(char *s, struct flags *flags);
bool str_match_exact(char *haystack, char *needle);
bool str_match_loosly(char *haystack, char *needle);

int parse(int ac, char **av, struct address *addr, struct flags *flags) {
  assert(addr != NULL);
  assert(flags != NULL);

  if (ac < 2) {
    fprintf(stderr, "wrong number of arguments\n");
    return -1;
  }

  for (int i = 1; i < ac; i++) {
    char *s = *(av + i);

    const bool is_option = str_match_loosly(s, OPTION_PREFIX_LONG) |
                           str_match_loosly(s, OPTION_PREFIX_SHORT);

    const bool is_addr = !is_option;

    if (is_option) {
      if (parse_option(s, flags))
        return (-1);
    } else if (is_addr) {
      if (parse_addr(s, addr))
        return (-1);
    }
  }

  return 0;
}

int str_isdigit_and_dots(char *s) {
  while (*s != '\0') {
    if (!isdigit(*s) && *s != '.')
      return 0;
    s++;
  }
  return 1;
}

int str_isdigit(char *s) {
  while (*s != '\0') {
    if (!isdigit(*s))
      return 0;
    s++;
  }
  return 1;
}

int parse_addr(char *s, struct address *addr) {
  if (str_isdigit(s)) {
    long n = atol(s);
    addr->binary.s_addr = htonl(n);
    addr->ipv4 = inet_ntoa(addr->binary);
    addr->hostname = NULL;
  } else if (str_isdigit_and_dots(s)) {
    int res = inet_pton(AF_INET, s, &addr->binary);
    if (res != 1) {
      fprintf(stderr, "could not parse destination\n");
      return -1;
    }
    addr->ipv4 = inet_ntoa(addr->binary);
    addr->hostname = NULL;
  } else {
    struct hostent *he = gethostbyname(s);
    if (he == NULL) {
      printf("error parsing hostname/ip address\n");
      return 1;
    }

    struct in_addr **addr_list = (struct in_addr **)he->h_addr_list;
    for (int i = 0; addr_list[i] != NULL; i++) {
      addr->binary = *addr_list[i];
      addr->ipv4 = inet_ntoa(addr->binary);
      addr->hostname = s;
      return 0;
    }
  }

  return 0;
}

int parse_option(char *s, struct flags *flags) {
  const bool is_option_long = str_match_loosly(s, OPTION_PREFIX_LONG);
  const bool is_option_short =
      !(is_option_long || !str_match_loosly(s, OPTION_PREFIX_SHORT));

  if (is_option_short) {
    s = s + strlen(OPTION_PREFIX_SHORT);
    if (str_match_exact(s, OPTION_NAME_SHORT_VERBOSE)) {
      if (flags->verbose.present) {
        fprintf(stderr, "%s %s\n", ERROR_MSG_OPTION_ALREADY_EXISTS, s);
        return -1;
      }
      flags->verbose.present = true;
    } else if (str_match_exact(s, OPTION_NAME_SHORT_HELP)) {
      if (flags->help.present) {
        fprintf(stderr, "%s %s\n", ERROR_MSG_OPTION_ALREADY_EXISTS, s);
        return -1;
      }
      flags->help.present = true;
    } else {
      fprintf(stderr, "%s %s\n", ERROR_MSG_OPTION_WRONG, s);
      return -1;
    }
  } else if (is_option_long) {
    s = s + strlen(OPTION_PREFIX_LONG);
    if (str_match_loosly(s, OPTION_NAME_LONG_TTL)) {

      if (flags->ttl.present) {
        fprintf(stderr, "%s %s\n", ERROR_MSG_OPTION_ALREADY_EXISTS, s);
        return -1;
      }
      flags->ttl.present = true;
      s += strlen(OPTION_NAME_LONG_TTL);

      while (*s != '\0' && *s == '0')
        s++;

      const bool overflows = strlen(s) > 3;

      int n = atoi(s);
      if (overflows || n < 1 || n > 255) {
        fprintf(stderr, "%s %s\n", ERROR_MSG_OPTION_TTL_INVALID_NUM, s);
        return -1;
      }
      flags->ttl.value = n;
    } else {
      fprintf(stderr, "%s %s", ERROR_MSG_OPTION_WRONG, s);
      return -1;
    }
  }

  return (0);
}

bool str_match_exact(char *haystack, char *needle) {
  assert(haystack != NULL);
  assert(needle != NULL);

  const int haystack_len = strlen(haystack);
  const int needle_len = strlen(needle);

  if (needle_len != haystack_len)
    return false;

  for (int i = 0; i < needle_len; i++) {
    if (haystack[i] != needle[i])
      return false;
  }

  return true;
}

bool str_match_loosly(char *haystack, char *needle) {
  assert(haystack != NULL);
  assert(needle != NULL);

  const int haystack_len = strlen(haystack);
  const int needle_len = strlen(needle);

  if (needle_len > haystack_len)
    return false;

  for (int i = 0; i < needle_len; i++) {
    if (haystack[i] != needle[i])
      return false;
  }

  return true;
}
