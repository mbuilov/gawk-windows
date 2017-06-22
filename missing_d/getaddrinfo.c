#ifndef HAVE_SOCKETS
#error getaddrinfo.c included by mistake! no socket support!
#else
#include <sys/types.h>
#include <sys/socket.h>
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include <errno.h>
#include <string.h>	/* strerror */

#include "getaddrinfo.h"

void
freeaddrinfo(struct addrinfo *res)
{
	if (res->ai_addr != NULL)
		free(res->ai_addr);
	free(res);
}

int
getaddrinfo(const char *hostname, const char *portname,
	struct addrinfo *hints, struct addrinfo **res)
{
	struct addrinfo *out;
	if (res == NULL)
		return EINVAL;

	out = (struct addrinfo *) calloc(1, sizeof(*out));
	if (out == NULL) {
		*res = NULL;
		return ENOMEM;
	}

	out->ai_addr = (struct sockaddr *) malloc(sizeof(struct sockaddr_in));
	if (out->ai_addr == NULL) {
		free(out);
		*res = NULL;
		return ENOMEM;
	}

	out->ai_socktype = SOCK_STREAM;
	if (hints != NULL) {
		if (hints->ai_socktype)
			out->ai_socktype = hints->ai_socktype;
		if (hints->ai_protocol)
			out->ai_protocol = hints->ai_protocol;
	}

	if (out->ai_protocol == 0) {
		switch (out->ai_socktype) {
		case SOCK_STREAM:
			out->ai_protocol = IPPROTO_TCP;
			break;
		case SOCK_DGRAM:
			out->ai_protocol = IPPROTO_UDP;
			break;
		case SOCK_RAW:
			out->ai_protocol = IPPROTO_RAW;
			break;
		}
	}

	out->ai_addrlen = sizeof(struct sockaddr_in);
	memset(out->ai_addr, '\0', sizeof(struct sockaddr_in));

	if (hostname != NULL) {
		struct hostent *he;
		he = gethostbyname(hostname);
		if (he != NULL && he->h_addr_list != NULL) {
			((struct sockaddr_in *)out->ai_addr)->sin_addr.s_addr
				= ((struct in_addr *)he->h_addr_list[0])->s_addr;
		} else {
			freeaddrinfo(out);
			return EADDRNOTAVAIL;
		}
	} else {
		if (!(out->ai_flags & AI_PASSIVE))
			((struct sockaddr_in *)out->ai_addr)->sin_addr.s_addr
							= htonl(INADDR_ANY);
	}
	((struct sockaddr_in *)out->ai_addr)->sin_family = AF_INET;
	out->ai_family = AF_INET;

	if (portname != NULL && *portname) {
		long portnum;
		char *end;
		portnum = strtol(portname, &end, 10);
		if (*end == '\0' && portnum > 0 && portnum < 65536) {
			((struct sockaddr_in *)out->ai_addr)->sin_port
							= htons(portnum);
		} else {
			struct servent *se;
			se = getservbyname(portname, NULL);
			if (se != NULL) {
				((struct sockaddr_in *)out->ai_addr)->sin_port
							= se->s_port;
			}
		}
	}

	*res = out;

	return 0;
}

const char *
gai_strerror(int errcode)
{
	return strerror(errcode);
}
#endif
