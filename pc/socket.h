/* An emulation for socket.h header for pc/ systems.  */

#ifndef GAWK_SOCKET_H
#define GAWK_SOCKET_H

#if defined(__MINGW32__) || defined(_MSC_VER)

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

#ifdef _MSC_VER

int ws_socket_dup(int oldfd);
#define socket_dup(oldfd) ws_socket_dup(oldfd)

typedef struct socket_file {
  unsigned buf_size;
  int filled;
  void *buf;
} socket_file_t;

#define socket_file_ferror(sf)	((sf)->filled < 0)

socket_file_t *socket_file_alloc(void);
void socket_file_free(socket_file_t *sf);
int socket_file_fflush(socket_file_t *sf, fd_t fd);
size_t socket_file_fwrite(socket_file_t *sf, fd_t fd, const void *ptr, size_t size, size_t nmemb);
void socket_file_clearerr(socket_file_t *sf);

#endif /* _MSC_VER */

#endif	/* __MINGW32__ || _MSC_VER */

#endif	/* GAWK_SOCKET_H */
