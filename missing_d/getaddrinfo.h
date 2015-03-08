#ifndef AI_ADDRCONFIG
#define AI_ADDRCONFIG 0
#endif /* AI_ADDRCONFIG */
#ifndef AI_PASSIVE
#define AI_PASSIVE 1
#endif /* AI_PASSIVE */

/* undef these, just in case, to avoid noisy warnings */
#undef addrinfo
#undef freeaddrinfo
#undef getaddrinfo

#define addrinfo xaddrinfo
#define freeaddrinfo xfreeaddrinfo
#define getaddrinfo xgetaddrinfo

struct addrinfo
{
	int ai_flags;
	int ai_socktype;
	int ai_family;
	int ai_protocol;
	socklen_t ai_addrlen;
	struct sockaddr * ai_addr;
	struct xaddrinfo * ai_next;
};

void freeaddrinfo(struct xaddrinfo * res);

int getaddrinfo(const char * hostname, const char * portname,
	struct xaddrinfo * hints, struct xaddrinfo ** res);

const char *gai_strerror(int errcode);
