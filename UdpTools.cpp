/*
 * UdpTools.cpp
 *
 *  Created on: Jul 2, 2019
 *      Author: thomas
 */

#include "UdpTools.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>

UdpTools::UdpTools() {

	stopReceiveThread = false;

}

UdpTools::~UdpTools() {
	// TODO Auto-generated destructor stub
}

int UdpTools::ReceiveUDPDatagram(std::string listenAddress, std::string listenPort, receiveCallbackFcn receiveCallback)
{
	struct sockaddr_in6 saddr, maddr;
	struct ipv6_mreq mreq;
	char buf[1400];
	ssize_t len;
	int sd, on = 1, hops = 255, ifidx = 0;
	fd_set fds;

	stopReceiveThread = false;

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
			printf("Read %zd bytes from sd: %s\n", len, buf);


			if (!len) {
				break;
			} else if (len < 0) {
				perror("read");
				return 1;
			} else {
				//len = write(fd, buf, len);
				/* printf("wrote %zd bytes to fd\n", len); */
				std::string code = std::string(&buf[0], len);

				receiveCallback(code, p_sockadr);
			}
		}

	close(sd);
	//close(fd);

	return 0;
}

void UdpTools::StopReceiveUDPDatagram()
{
	stopReceiveThread = true;
}

int UdpTools::SendUDPDatagram(bool useMulticast, std::string targetAddress, std::string port, const char buf[], int length)
{
	struct sockaddr_in6 saddr;
	struct ipv6_mreq mreq;
	//char buf[1400];
	//ssize_t len = 1;
	int sd, on = 1, hops = 255, ifidx = 0;

	sd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if (sd < 0) {
		perror("socket");
		return 1;
	}

	if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
		perror("setsockopt");
		return 1;
	}

	if(useMulticast)
	{
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
	}


	memset(&saddr, 0, sizeof(struct sockaddr_in6));
	saddr.sin6_family = AF_INET6;
	saddr.sin6_port = htons(atoi(port.c_str()));
	inet_pton(AF_INET6, targetAddress.c_str(), &saddr.sin6_addr);

	if(useMulticast)
	{
		memcpy(&mreq.ipv6mr_multiaddr, &saddr.sin6_addr, sizeof(mreq.ipv6mr_multiaddr));
		mreq.ipv6mr_interface = ifidx;

		if (setsockopt(sd, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char *) &mreq, sizeof(mreq))) {
			perror("setsockopt");
			return 1;
		}
	}


	/*
	fd = open("/dev/stdin", O_RDONLY, NULL);
	if (fd < 0) {
		perror("open");
		return 1;
	}
	*/


	//	while (len) {
	//		len = read(fd, buf, 1400);
	//		/* printf("read %zd bytes from fd\n", len); */
	//		if (!len) {
	//			break;
	//		} else if (len < 0) {
	//			perror("read");
	//			return 1;
	//		} else {
	//
	//			/* printf("sent %zd bytes to sd\n", len); */
	//			usleep(10000); /* rate limit, 10000 = 135 kilobyte/s */
	//		}
	//	}

	sendto(sd, buf, length, 0, (const struct sockaddr *) &saddr, sizeof(saddr));

	close(sd);
	//close(fd);

	return 0;
}
