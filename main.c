#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include "utils.h"
#include "scanner.h"

ScanResult scan_result;

void handle_sigint(int sig) {
  log_message(LOG_INFO, "Caught SIGINT, exiting...");
  if (scan_result.port_results) free(scan_result.port_results);
  exit(0);
}

int main(int argc, char *argv[]){
  struct sockaddr_in server_addr;

  if (argc != 2) {
    error("Invalid input. Usage: network-diff-scanner <target_IP>");
  }

  if (signal(SIGINT, handle_sigint) == SIG_ERR) {
    error("Failed to set SIGINT handler");
  }

  // Validate input hostname/IP
  if (resolve_hostname(argv[1], &server_addr) < 0) {
    log_message(LOG_ERROR, "Failed to resolve hostname %s", argv[1]);
    return -1;
  }

  // Initialize scan result
  log_message(LOG_INFO, "Scanning host %s", argv[1]);
  scan_result.port_results = calloc(NUM_PORTS, sizeof(PortScanResult));
  scan_host(server_addr, &scan_result);
  free(scan_result.port_results);

  return 0;
}
