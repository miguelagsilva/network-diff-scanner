#include <asm-generic/errno.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "utils.h"
#include "scanner.h"

int make_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int clean_epoll(int epfd, PortScanResult *ports_to_scan) {
  for (int i = 0; i < NUM_PORTS; i++) {
    int sock = ports_to_scan[i].socket;
    if (sock <= 0) continue;
    log_message(LOG_DEBUG, "Cleaned up fd %d", sock);
    epoll_ctl(epfd, EPOLL_CTL_DEL, sock, NULL);
    close(sock);
    ports_to_scan[i].socket = 0;
  }
  return 0;
}

int resolve_hostname(const char *host, struct sockaddr_in *addr) {
  struct addrinfo hints, *result, *rp;
  
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;      // Allow IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM;  // TCP
  
  int status = getaddrinfo(host, NULL, &hints, &result);
  if (status != 0) {
    log_message(LOG_ERROR, "Invalid address: %s", gai_strerror(status));
    return -1;
  }
  
  // Look for the first IPv4 address
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    if (rp->ai_family == AF_INET) {
      memcpy(addr, rp->ai_addr, sizeof(struct sockaddr_in));
      break;
    }
  }
  
  freeaddrinfo(result);
  
  if (rp == NULL) {
    log_message(LOG_ERROR, "No IPv4 address found for %s", host);
    return -1;
  }
  
  return 0;
}

void scan_host(struct sockaddr_in server_addr, ScanResult *scan_result) {
  PortScanResult *ports_to_scan = scan_result->port_results;
  int num_ports_to_scan = 1;
  struct epoll_event events[MAX_EVENTS];

  int epfd = epoll_create1(0); // Create epoll instance
  if (epfd < 0) { 
    log_message(LOG_ERROR, "epoll_create1");
  }

  // Make the scan in batches of MAX_EVENTS
  for (int start = 1; start < NUM_PORTS; start += MAX_EVENTS) {
    int end = start + MAX_EVENTS;
    if (end > NUM_PORTS) end = NUM_PORTS;

    log_message(LOG_DEBUG, "Scanning ports %d to %d", start, end - 1);  

    // Create sockets for a range of ports
    for (int port = start; port < end; port++) {
      int sock = socket(AF_INET, SOCK_STREAM, 0);
      if (sock < 0) continue;

      ports_to_scan[num_ports_to_scan].port = port;
      ports_to_scan[num_ports_to_scan].socket = sock;
      num_ports_to_scan++;

      make_nonblocking(sock);
      server_addr.sin_port = htons(port);

      int res = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
      if (res < 0 && errno != EINPROGRESS) { close(sock); continue; }

      struct epoll_event ev;
      ev.events = EPOLLOUT;
      ev.data.fd = sock;
      epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev); // Add socket to epoll
      log_message(LOG_DEBUG, "Scanning port %d", port);
    }

    // Wait for events with a timeout
    int n = epoll_wait(epfd, events, MAX_EVENTS, TIMEOUT_MS);
    if (n == 0) {
      log_message(LOG_DEBUG, "Timeout reached, some ports may be filtered");
      clean_epoll(epfd, ports_to_scan);
      continue;
    } else if (n < 0) {
      log_message(LOG_ERROR, "epoll_wait");
      continue;
    }

    // Process each triggered event
    for (int i = 0; i < n; i++) {
      int sock = events[i].data.fd;
      int so_error;
      socklen_t len = sizeof(so_error);

      getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);
      struct sockaddr_in addr;
      socklen_t addrlen = sizeof(addr);
      getpeername(sock, (struct sockaddr *)&addr, &addrlen);
      int port = ntohs(addr.sin_port);
      if (so_error == 0) {
        log_message(LOG_INFO, "Port %d is OPEN", port);
      } else if (so_error == ECONNREFUSED) {
        log_message(LOG_DEBUG, "Port %d is CLOSED", port);
      }

      epoll_ctl(epfd, EPOLL_CTL_DEL, sock, NULL);
      close(sock);
      ports_to_scan[port].socket = 0;
    }
  }

  close(epfd);
}

