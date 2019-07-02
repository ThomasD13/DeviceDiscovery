/*	IPv6 multicast example - ipv6_multicast_recv.c
	2012 - Bjorn Lindgren <nr@c64.org>
	https://github.com/bjornl/ipv6_multicast_example
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>

bool static stopReceiveThread = false;

int receiveThread(std::string listenAddress, std::string listenPort);

int main(int argc, char ** argv)
{
	receiveThread(argv[1], argv[2]);
}

int receiveThread(std::string listenAddress, std::string listenPort)
{
	struct sockaddr_in6 saddr, maddr;
	struct ipv6_mreq mreq;
	char buf[1400];
	ssize_t len;
	int sd, fd, rc, on = 1, hops = 255, ifidx = 0;
	bool checkIfAbort = false;
	struct timeval tv;
	fd_set fds;

	if (listenAddress.empty() || listenPort.empty()) {
		printf("\nUsage: %s <address> <port>\n\nExample: %s ff02::5:6 12345\n\n", listenAddress, listenPort);
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
	saddr.sin6_port = htons(atoi(listenPort.c_str()));
	saddr.sin6_addr = in6addr_any;

	if (bind(sd, (struct sockaddr *) &saddr, sizeof(saddr))) {
		perror("bind");
		return 1;
	}

	memset(&maddr, 0, sizeof(maddr));
	inet_pton(AF_INET6, listenAddress.c_str(), &maddr.sin6_addr);

	memcpy(&mreq.ipv6mr_multiaddr, &maddr.sin6_addr, sizeof(mreq.ipv6mr_multiaddr));
	mreq.ipv6mr_interface = ifidx;

	if (setsockopt(sd, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char *) &mreq, sizeof(mreq))) {
		perror("setsockopt");
		return 1;
	}

	FD_ZERO(&fds);
	FD_SET(sd, &fds);


	fd = open("/dev/stdout", O_WRONLY, NULL);
	if (fd < 0) {
		perror("open");
		return 1;
	}

	while (!stopReceiveThread) {
		sockaddr p_sockadr;
		socklen_t p_socklength = sizeof(struct sockaddr_in6);

		//Wait until stream is ready to read or
		//user signaled to stop reading from socket
		unsigned long int bytesToRead = 0;
		while(bytesToRead < 1)
		{
			ioctl(sd, FIONREAD, &bytesToRead);
			if(stopReceiveThread)
			{
				break;
			}
		}

		len = recvfrom(sd, buf, 1400, 0, &p_sockadr, &p_socklength);
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
			std::string code = std::string(&buf[0], len);
			std::string foobar = "AreYouThere?";
			if(code.compare(foobar) == 0)
			{
				char hostname[1024];
				char sourceName[INET6_ADDRSTRLEN];

				printf("Now a response with hostname would be sent back!\n");
				gethostname(&hostname[0], 1024);

				char *s = NULL;


				switch(p_sockadr.sa_family) {
				    case AF_INET: {
				        struct sockaddr_in *addr_in = (struct sockaddr_in *)&p_sockadr;
				        s = (char*)malloc(INET_ADDRSTRLEN);
				        inet_ntop(AF_INET, &(addr_in->sin_addr), s, INET_ADDRSTRLEN);
				        break;
				    }
				    case AF_INET6: {
				        struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)&p_sockadr;
				        s = (char*)malloc(INET6_ADDRSTRLEN);
				        inet_ntop(AF_INET6, &(addr_in6->sin6_addr), s, INET6_ADDRSTRLEN);
				        break;
				    }
				    default:
				        break;
				}

				//TODO: Send UDP datagram back :)
				printf("%s to %s\n", &hostname[0], s);
				free(s);
			}
		}
	}

	close(sd);
	close(fd);

	return 0;
}
