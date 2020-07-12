/* An emulation for socket.h header for pc/ systems.  */

#ifndef GAWK_SOCKET_H
#define GAWK_SOCKET_H

#ifdef WINDOWS_NATIVE

#include <io.h>

#if !defined _WIN32_WINNT || _WIN32_WINNT < 0x501
# undef _WIN32_WINNT
# define _WIN32_WINNT 0x501
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

#define socket(f,t,p)		w32_socket(f,t,p)
#define setsockopt(f,l,o,v,s)	w32_setsockopt(f,l,o,v,s)
#define bind(f,a,l)		w32_bind(f,a,l)
#define connect(f,a,l)		w32_connect(f,a,l)
#define listen(f,b)		w32_listen(f,b)
#define accept(f,a,l)		w32_accept(f,a,l)
#define closemaybesocket(f)	w32_closemaybesocket(f)
#define recvfrom(f,b,l,fl,fr,ln)	w32_recvfrom(f,b,l,fl,fr,ln)
#define shutdown(f,h)		w32_shutdown(f,h)

#define SOCKET_TO_FD(s)		socket_to_fd(s)
#define FD_TO_SOCKET(fd)	fd_to_socket(fd)

int w32_socket (int, int, int);
int w32_setsockopt (int, int, int, const char *, int);
int w32_bind (int, const struct sockaddr *, int);
int w32_connect (int, const struct sockaddr *, int);
int w32_listen (int, int);
int w32_accept (int, struct sockaddr *, int *);
int w32_closemaybesocket (int);
int w32_recvfrom (int, char *, int, int, struct sockaddr *, int *);
int w32_shutdown (int, int);
int socket_to_fd (SOCKET);
SOCKET fd_to_socket (int);
SOCKET valid_socket (int);

const char *gai_strerror_buf (int ecode, char buf[], unsigned int buf_size);

#include "mscrtx/socket_fd.h"
#include "mscrtx/is_socket.h"
#include "mscrtx/socket_file.h"

#define socket_dup(oldfd) socket_fd_dup(oldfd)

#endif	/* WINDOWS_NATIVE */

#endif	/* GAWK_SOCKET_H */
