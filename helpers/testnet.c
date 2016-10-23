#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <memory.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif /* O_BINARY */

#ifndef AI_ADDRCONFIG	/* not everyone has this symbol */
#define AI_ADDRCONFIG 0
#endif /* AI_ADDRCONFIG */

#ifndef AF_UNSPEC
#define AF_UNSPEC 0
#endif
#ifndef AF_INET
#define AF_INET 2
#endif
#ifndef AF_INET6
#define AF_INET6 10
#endif

#include <limits.h>

#ifndef SHUT_RD
#define SHUT_RD		0
#endif

#ifndef SHUT_WR
#define SHUT_WR		1
#endif

#ifndef SHUT_RDWR
#define SHUT_RDWR	2
#endif

#define _(str)	str

#define INVALID_HANDLE	(-1)

static int str2mode(const char *mode);


/* socketopen --- open a socket and set it into connected state */

static int
socketopen(int family, int type, const char *localpname,
	const char *remotepname, const char *remotehostname)
{
	struct addrinfo *lres, *lres0;
	struct addrinfo lhints;
	struct addrinfo *rres, *rres0;
	struct addrinfo rhints;

	int lerror, rerror;

	int socket_fd = INVALID_HANDLE;
	int any_remote_host = (strcmp(remotehostname, "0") == 0);

	memset(& lhints, '\0', sizeof (lhints));
	lhints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
	lhints.ai_socktype = type;
	lhints.ai_family = family;

	lerror = getaddrinfo(NULL, localpname, & lhints, & lres);
	if (lerror) {
		if (strcmp(localpname, "0") != 0) {
			fprintf(stderr, _("local port %s invalid in `/inet'"), localpname);
			exit(1);
		}
		lres0 = NULL;
		lres = & lhints;
	} else
		lres0 = lres;

	while (lres != NULL) {
		memset (& rhints, '\0', sizeof (rhints));
		rhints.ai_flags = lhints.ai_flags;
		rhints.ai_socktype = lhints.ai_socktype;
		rhints.ai_family = lhints.ai_family;
		rhints.ai_protocol = lhints.ai_protocol;

		rerror = getaddrinfo(any_remote_host ? NULL : remotehostname,
				remotepname, & rhints, & rres);
		if (rerror) {
			if (lres0 != NULL)
				freeaddrinfo(lres0);
			fprintf(stderr, _("remote host and port information (%s, %s) invalid"), remotehostname, remotepname);
			exit(1);
		}
		rres0 = rres;
		socket_fd = INVALID_HANDLE;
		while (rres != NULL) {
			socket_fd = socket(rres->ai_family,
				rres->ai_socktype, rres->ai_protocol);
			if (socket_fd < 0 || socket_fd == INVALID_HANDLE)
				goto nextrres;

			if (type == SOCK_STREAM) {
				int on = 1;
#ifdef SO_LINGER
				struct linger linger;
				memset(& linger, '\0', sizeof(linger));
#endif
				setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR,
					(char *) & on, sizeof(on));
#ifdef SO_LINGER
				linger.l_onoff = 1;
				/* linger for 30/100 second */
				linger.l_linger = 30;
				setsockopt(socket_fd, SOL_SOCKET, SO_LINGER,
					(char *) & linger, sizeof(linger));
#endif
			}
			if (bind(socket_fd, lres->ai_addr, lres->ai_addrlen) != 0)
				goto nextrres;

			if (! any_remote_host) { /* not ANY => create a client */
				if (connect(socket_fd, rres->ai_addr, rres->ai_addrlen) == 0)
					break;
			} else { /* remote host is ANY => create a server */
				if (type == SOCK_STREAM) {
					int clientsocket_fd = INVALID_HANDLE;

					struct sockaddr_storage remote_addr;
					socklen_t namelen = sizeof(remote_addr);

					if (listen(socket_fd, 1) >= 0
					    && (clientsocket_fd = accept(socket_fd,
						(struct sockaddr *) & remote_addr,
						& namelen)) >= 0) {
						close(socket_fd);
						socket_fd = clientsocket_fd;
						break;
					}
				} else if (type == SOCK_DGRAM) {
#ifdef MSG_PEEK
					char buf[10];
					struct sockaddr_storage remote_addr;
					socklen_t read_len;

					if (recvfrom(socket_fd, buf, 1, MSG_PEEK,
						(struct sockaddr *) & remote_addr,
							& read_len) >= 0
							&& read_len
							&& connect(socket_fd,
						(struct sockaddr *) & remote_addr,
								read_len) == 0)
							break;
#endif
				}
			}

nextrres:
			if (socket_fd != INVALID_HANDLE)
				close(socket_fd);
			socket_fd = INVALID_HANDLE;
			rres = rres->ai_next;
		}
		freeaddrinfo(rres0);
		if (socket_fd != INVALID_HANDLE)
			break;
		lres = lres->ai_next;
	}
	if (lres0)
		freeaddrinfo(lres0);

	return socket_fd;
}

/* devopen --- handle /dev/std{in,out,err}, /dev/fd/N, regular files */

/*
 * Strictly speaking, "name" is not a "const char *" because we temporarily
 * change the string.
 */

int
devopen(const char *name, const char *mode)
{
	int openfd;
	char *cp;
	char *ptr;
	int flag = 0;
	int len;
	int family;
	int protocol;
	char *hostname;
	char *hostnameslastcharp;
	char *localpname;
	char *localpnamelastcharp;

	flag = str2mode(mode);
	openfd = INVALID_HANDLE;

	/* /inet/protocol/localport/hostname/remoteport */
	len = 6;

	cp = (char *) name + len;
	/* which protocol? */
	if (strncmp(cp, "tcp/", 4) == 0)
		protocol = SOCK_STREAM;
	else if (strncmp(cp, "udp/", 4) == 0)
		protocol = SOCK_DGRAM;
	else {
		protocol = SOCK_STREAM;	/* shut up the compiler */
		fprintf(stderr, _("no (known) protocol supplied in special filename `%s'"), name);
		exit(1);
	}
	cp += 4;

	/* which localport? */
	localpname = cp;
	while (*cp != '/' && *cp != '\0')
		cp++;
	/*
	 * Require a port, let them explicitly put 0 if
	 * they don't care.
	 */
	if (*cp != '/' || cp == localpname) {
		fprintf(stderr, _("special file name `%s' is incomplete"), name);
		exit(1);
	}

	/*
	 * We change the special file name temporarily because we
	 * need a 0-terminated string here for conversion with atoi().
	 * By using atoi() the use of decimal numbers is enforced.
	 */
	*cp = '\0';
	localpnamelastcharp = cp;

	/* which hostname? */
	cp++;
	hostname = cp;
	while (*cp != '/' && *cp != '\0')
		cp++;
	if (*cp != '/' || cp == hostname) {
		*localpnamelastcharp = '/';
		fprintf(stderr, _("must supply a remote hostname to `/inet'"));
		exit(1);
	}
	*cp = '\0';
	hostnameslastcharp = cp;

	/* which remoteport? */
	cp++;
	/*
	 * The remote port ends the special file name.
	 * This means there already is a '\0' at the end of the string.
	 * Therefore no need to patch any string ending.
	 *
	 * Here too, require a port, let them explicitly put 0 if
	 * they don't care.
	 */
	if (*cp == '\0') {
		*localpnamelastcharp = '/';
		*hostnameslastcharp = '/';
		fprintf(stderr, _("must supply a remote port to `/inet'"));
		exit(1);
	}

	{
#define DEFAULT_RETRIES 20
	static unsigned long def_retries = DEFAULT_RETRIES;
	static bool first_time = true;
	unsigned long retries = 0;
	static long msleep = 1000;

	if (first_time) {
		char *cp, *end;
		unsigned long count = 0;
		char *ms2;

		first_time = false;
		if ((cp = getenv("GAWK_SOCK_RETRIES")) != NULL) {
			count = strtoul(cp, & end, 10);
			if (end != cp && count > 0)
				def_retries = count;
		}

		/*
		 * Env var is in milliseconds, paramter to usleep()
		 * is microseconds, make the conversion. Default is
		 * 1 millisecond.
		 */
		if ((ms2 = getenv("GAWK_MSEC_SLEEP")) != NULL) {
			msleep = strtol(ms2, & end, 10);
			if (end == ms2 || msleep < 0)
				msleep = 1000;
			else
				msleep *= 1000;
		}
	}
	retries = def_retries;

	do {
		openfd = socketopen(family, protocol, localpname, cp, hostname);
		retries--;
	} while (openfd == INVALID_HANDLE && retries > 0 && usleep(msleep) == 0);
	}

	*localpnamelastcharp = '/';
	*hostnameslastcharp = '/';

	return openfd;
}


/* str2mode --- convert a string mode to an integer mode */

static int
str2mode(const char *mode)
{
	int ret;
	const char *second = & mode[1];

	if (*second == 'b')
		second++;

	switch(mode[0]) {
	case 'r':
		ret = O_RDONLY;
		if (*second == '+' || *second == 'w')
			ret = O_RDWR;
		break;

	case 'w':
		ret = O_WRONLY|O_CREAT|O_TRUNC;
		if (*second == '+' || *second == 'r')
			ret = O_RDWR|O_CREAT|O_TRUNC;
		break;

	case 'a':
		ret = O_WRONLY|O_APPEND|O_CREAT;
		if (*second == '+')
			ret = O_RDWR|O_APPEND|O_CREAT;
		break;

	default:
		ret = 0;		/* lint */
		fprintf(stderr, "bad mode passed to str2mode\n");
		exit(1);
	}
	if (strchr(mode, 'b') != NULL)
		ret |= O_BINARY;
	return ret;
}

int main(int argc, char **argv)
{
	int ofd;
	int ifd;
	int n;
	int nread;
	int wrote;
	char inbuf[BUFSIZ];
	/* char outbuf[] = "GET www.yahoo.com/index.html HTTP/1.0\r\n\r\n"; */
	char outbuf[] = "GET index.html HTTP/1.0\r\n\r\n";
	char sockname[BUFSIZ];

	strcpy(sockname, "/inet/tcp/0/www.yahoo.com/80");

	ofd = devopen(sockname, "rw");
	if (ofd < 0) {
		perror("devopen");
		exit(1);
	}

	if ((ifd = dup(ofd)) < 0) {
		perror("dup");
		exit(1);
	}

	n = strlen(outbuf);
	if ((wrote = write(ofd, outbuf, n)) != n) {
		perror("write");
		exit(1);
	}

	while ((nread = read(ifd, inbuf, sizeof(inbuf))) > 0) {
		write(1, inbuf, nread);
	}

	close(ifd);
	close(ofd);

	return 0;
}
