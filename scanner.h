#ifndef SCANNER_H
#define SCANNER_H
#include <netinet/in.h>

typedef enum {
    PORT_UNKNOWN = 0,
    PORT_OPEN,
    PORT_CLOSED,
    PORT_FILTERED
} PortStatus;

typedef struct {
   int socket;
   int port;
   PortStatus status;
} PortScanResult;

typedef struct {
    struct sockaddr_in addr;
    PortScanResult *port_results;
} ScanResult;

#define MAX_EVENTS 512
#define NUM_PORTS 65536
#define TIMEOUT_MS 3000

void scan_host(struct sockaddr_in server_addr, ScanResult *scan_result);
int resolve_hostname(const char *host, struct sockaddr_in *addr);

#endif // SCANNER_H
