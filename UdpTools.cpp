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

UdpTools::UdpTools() {
	// TODO Auto-generated constructor stub

}

UdpTools::~UdpTools() {
	// TODO Auto-generated destructor stub
}

int UdpTools::ReceiveUDPDatagram(std::string listenAddress, std::string listenPort, void (* receiveCallback)(std::string datagram))
{
	return 0;
}

int UdpTools::SendUDPDatagram(std::string targetAddress, std::string port, char buf[], int length)
{
	struct sockaddr_in6 saddr;
	struct ipv6_mreq mreq;
	//char buf[1400];
	//ssize_t len = 1;
	int sd, fd, on = 1, hops = 255, ifidx = 0;

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

	memset(&saddr, 0, sizeof(struct sockaddr_in6));
	saddr.sin6_family = AF_INET6;
	saddr.sin6_port = htons(atoi(port.c_str()));
	inet_pton(AF_INET6, targetAddress.c_str(), &saddr.sin6_addr);

	memcpy(&mreq.ipv6mr_multiaddr, &saddr.sin6_addr, sizeof(mreq.ipv6mr_multiaddr));
	mreq.ipv6mr_interface = ifidx;

	if (setsockopt(sd, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char *) &mreq, sizeof(mreq))) {
		perror("setsockopt");
		return 1;
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
	close(fd);

	return 0;
}
