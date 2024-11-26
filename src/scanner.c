#include "scanner.h"
#include "utils.h"
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/if.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#define POLL_TIMEOUT 10  // milliseconds
#define TARGET_PORT 9765 // TODO change udp port
#define BUFFER_SIZE 500  // TODO what is the size of the data??

void uint32_to_sockaddr(uint32_t ip, struct sockaddr_in *addr) {
  addr->sin_addr.s_addr = htonl(ip);
}

uint32_t sockaddr_to_uint32(struct sockaddr_in *addr) {
  return ntohl(addr->sin_addr.s_addr);
}

void handle_payload(char buf[BUFFER_SIZE], struct sockaddr_in *addr) {
  // TODO do something with buffer and addr
}

void receive_from(struct pollfd *pfds) {
  int num_events = poll(pfds, 1, POLL_TIMEOUT);

  CHECK(num_events == -1);

  if (num_events == 0) {
    return;
  }

  if (!(pfds[0].revents & POLLIN)) {
    return;
  }

  char buf[BUFFER_SIZE] = {0};
  struct sockaddr_storage addr;
  socklen_t fromlen;
  ssize_t byte_count;
  byte_count = recvfrom(pfds[0].fd, buf, sizeof buf, MSG_WAITALL,
                        (struct sockaddr *)&addr, &fromlen);

  CHECK(byte_count == -1);

  DEBUG_LOG("Received %zd bytes from %s\n", byte_count,
            inet_ntoa(((struct sockaddr_in *)&addr)->sin_addr));

  handle_payload(buf, (struct sockaddr_in *)&addr);
}

void send_to_subnet(struct sockaddr_in *if_ip, struct sockaddr_in *if_mask) {

  uint32_t ip = sockaddr_to_uint32(if_ip);
  uint32_t mask = sockaddr_to_uint32(if_mask);

  uint32_t lower_bound = ip & mask;
  uint32_t upper_bound = ip | ~mask;

  if (DEBUG) {
    struct sockaddr_in upper, lower;
    uint32_to_sockaddr(lower_bound, &lower);
    uint32_to_sockaddr(upper_bound, &upper);
    DEBUG_LOG("Lower: %s\n", inet_ntoa(lower.sin_addr));
    DEBUG_LOG("Upper: %s\n", inet_ntoa(upper.sin_addr));
  }

  struct sockaddr_in ip_adr = {.sin_port = htons(TARGET_PORT)};
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  int yes = 1;

  CHECK(sockfd == -1);
  CHECK(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1);

  ssize_t bytes_sent;

  // TODO craft payload
  char payload[] = "hello";
  size_t payload_size = strlen(payload) + 1;

  struct pollfd pfds[1];
  pfds[0].fd = sockfd;
  pfds[0].events = POLLIN;

  struct sockaddr_in it_ip;
  for (uint32_t it = lower_bound; it < upper_bound; it++) {
    uint32_to_sockaddr(it, &ip_adr);

    bytes_sent = sendto(sockfd, payload, payload_size, 0,
                        (struct sockaddr *)&ip_adr, sizeof ip_adr);

    receive_from(pfds);

    CHECK(bytes_sent == -1);

    if (DEBUG && it % 10 == 0) {
      uint32_to_sockaddr(it, &it_ip);
      DEBUG_LOG("Send %zi bytes to %s\n", bytes_sent,
                inet_ntoa(it_ip.sin_addr));
    }

    if (bytes_sent < payload_size) {
      // TODO handle fragmented data
    }
  }

  close(sockfd);
}

void scan_net() {
  struct ifaddrs *pIfaddrs;

  CHECK(getifaddrs(&pIfaddrs) == -1);
  int fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  for (struct ifaddrs *ifa = pIfaddrs; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;

    // we only support ipv4
    if (ifa->ifa_addr->sa_family != AF_PACKET)
      continue;

    // skip localhost, docker, qemu interfaces, ...
    // we might need a white list
    if (strcmp(ifa->ifa_name, "lo") == 0 ||
        strstr(ifa->ifa_name, "docker") > 0 || strstr(ifa->ifa_name, "vir") > 0)
      continue;

    struct ifreq ifreq_mask = {0}, ifreq_addr = {0};
    struct sockaddr_in *ipv4, *net_mask;

    strncpy(ifreq_mask.ifr_name, ifa->ifa_name, IFNAMSIZ);
    strncpy(ifreq_addr.ifr_name, ifa->ifa_name, IFNAMSIZ);

    CHECK(ioctl(fd, SIOCGIFADDR, &ifreq_addr) == -1);
    CHECK(ioctl(fd, SIOCGIFNETMASK, &ifreq_mask) == -1);

    ipv4 = (struct sockaddr_in *)&ifreq_addr.ifr_ifru.ifru_addr;
    net_mask = (struct sockaddr_in *)&ifreq_mask.ifr_ifru.ifru_netmask;

    DEBUG_LOG("interface: %s\n", ifreq_mask.ifr_name);
    DEBUG_LOG("address: %s\n", inet_ntoa(ipv4->sin_addr));
    DEBUG_LOG("net_mask: %s\n", inet_ntoa(net_mask->sin_addr));
    send_to_subnet(ipv4, net_mask);
  }

  close(fd);
  freeifaddrs(pIfaddrs);
}
