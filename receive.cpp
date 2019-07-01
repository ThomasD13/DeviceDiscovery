/*	IPv6 multicast example - ipv6_multicast_recv.c
	2012 - Bjorn Lindgren <nr@c64.org>
	https://github.com/bjornl/ipv6_multicast_example
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>

int
main(int argc, char *argv[])
{
	struct sockaddr_in6 saddr, maddr;
	struct ipv6_mreq mreq;
	char buf[1400];
	ssize_t len;
	int sd, fd, rc, on = 1, flag = 0, hops = 255, ifidx = 0;
	struct timeval tv;
	fd_set fds;

	if (argc < 3) {
		printf("\nUsage: %s <address> <port>\n\nExample: %s ff02::5:6 12345\n\n", argv[0], argv[0]);
		return 1;
	}

	sd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if (sd < 0) {
		perror("socket");
		return 1;
	}

	if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
		perror("setsockopt");
		return 1;
	}

	if (setsockopt(sd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifidx, sizeof(ifidx))) {
		perror("setsockopt");
		return 1;
	}

	if (setsockopt(sd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hops, sizeof(hops))) {
		perror("setsockopt");
		return 1;
	}

	if (setsockopt(sd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &on, sizeof(on))) {
		perror("setsockopt");
		return 1;
	}

	memset(&saddr, 0, sizeof(saddr));
	saddr.sin6_family = AF_INET6;
	saddr.sin6_port = htons(atoi(argv[2]));
	saddr.sin6_addr = in6addr_any;

	if (bind(sd, (struct sockaddr *) &saddr, sizeof(saddr))) {
		perror("bind");
		return 1;
	}

	memset(&maddr, 0, sizeof(maddr));
	inet_pton(AF_INET6, argv[1], &maddr.sin6_addr);

	memcpy(&mreq.ipv6mr_multiaddr, &maddr.sin6_addr, sizeof(mreq.ipv6mr_multiaddr));
	mreq.ipv6mr_interface = ifidx;

	if (setsockopt(sd, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char *) &mreq, sizeof(mreq))) {
		perror("setsockopt");
		return 1;
	}

	FD_ZERO(&fds);
	FD_SET(sd, &fds);
	tv.tv_sec  = 10;
	tv.tv_usec = 0;

	fd = open("/dev/stdout", O_WRONLY, NULL);
	if (fd < 0) {
		perror("open");
		return 1;
	}

	while (1) {
		std::string code;
		sockaddr* p_sockadr;
		socklen_t p_socklength = sizeof(struct sockaddr_in6);

		/*
		if (flag) {
			rc = select(sd + 1, &fds, NULL, NULL, &tv);
			if (!rc) {
				break;
			}
			tv.tv_sec  = 10;
			tv.tv_usec = 0;
		} */
		//len = read(sd, buf, 1400);
		len = recvfrom(sd, buf, 1400, 0, p_sockadr, &p_socklength);
		buf[len] = '\0';
		printf("Read %zd bytes from sd\n", len);

		if (!len) {
			break;
		} else if (len < 0) {
			perror("read");
			return 1;
		} else {
			len = write(fd, buf, len);
			/* printf("wrote %zd bytes to fd\n", len); */
			//flag++;
			code = std::string(&buf[0], len);
			if(code.compare("AreYouThere?\n") == 0)
			{
				char hostname[1024];
				char sourceName[INET6_ADDRSTRLEN];

				printf("Now a response with hostname would be sent back!\n");
				gethostname(&hostname[0], 1024);

				char *s = NULL;
				switch(p_sockadr->sa_family) {
				    case AF_INET: {
				        struct sockaddr_in *addr_in = (struct sockaddr_in *)p_sockadr;
				        s = (char*)malloc(INET_ADDRSTRLEN);
				        inet_ntop(AF_INET, &(addr_in->sin_addr), s, INET_ADDRSTRLEN);
				        break;
				    }
				    case AF_INET6: {
				        struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)p_sockadr;
				        s = (char*)malloc(INET6_ADDRSTRLEN);
				        inet_ntop(AF_INET6, &(addr_in6->sin6_addr), s, INET6_ADDRSTRLEN);
				        break;
				    }
				    default:
				        break;
				}
				printf("%s to %s\n", &hostname[0], s);
				free(s);
			}
		}
	}

	close(sd);
	close(fd);

	return 0;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
