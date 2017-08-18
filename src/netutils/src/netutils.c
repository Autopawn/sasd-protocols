#include "netutils.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define abort(fd)\
    close(fd); return -1;

int listen_tcp(uint16_t port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        return -1;
    }

    int ruaddr = 1;
    int result = setsockopt(sock,
            SOL_SOCKET,
            SO_REUSEADDR,
            &ruaddr,
            sizeof(ruaddr));
    if (result == -1) {
        abort(sock);
    }

    struct sockaddr_in name;
    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = INADDR_ANY;

    result = bind(sock,
            (struct sockaddr *)&name,
            sizeof(name));
    if (result == -1) {
        abort(sock);
    }

    result = listen(sock, DEFAULT_BACKLOG);
    if (result == -1) {
        abort(sock);
    }

    return sock;
}

int connect_tcp(const char *host, uint16_t port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        return -1;
    }

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    inet_aton(host, &saddr.sin_addr);

    int result = connect(sock,
            (struct sockaddr *)&saddr,
            sizeof(saddr));
    if (result == -1) {
        abort(sock);
    }
    
    return sock;
}
