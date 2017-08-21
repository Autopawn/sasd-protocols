/*
 * netutils.c - Shortcuts for common network operations
 *
 * Author:  Rodolfo Castillo Mateluna
 *          Department of Informatics
 *          U.T.F.S.M.
 * Date:    March 24, 2017 
 */

#ifndef NETUTILS_H
#define NETUTILS_H

#include <stdint.h>

#define DEFAULT_BACKLOG 5

/*
 * Returns TCP/IP socket file descriptor
 * bound to port *port*. Returns -1 on
 * error, and errno is set accordingly
 */
int listen_tcp(uint16_t port);

/*
 * Returns TCP/IP socket file descriptor
 * connected to host *host* in IPv4 dot
 * notation in port *port*. Returns -1 on
 * error and errno is set accordingly
 */
int connect_tcp(const char *host, uint16_t port);

#endif /* end of include guard: NETUTILS_H */
