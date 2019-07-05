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
#include <net/if.h>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdexcept>

UdpTools::UdpTools() {

	stopReceiveThread = false;
	specifiedInterface = "";

}

UdpTools::UdpTools(std::string iface)
{
	stopReceiveThread = false;
	specifiedInterface = iface;
	std::list<std::string> interfacesOnSystem = getInterfaces();
	std::list<std::string>::iterator it;

	bool ifacefound = false;
	for(it = interfacesOnSystem.begin(); it != interfacesOnSystem.end(); it++)
	{
		if(it->compare(iface) == 0)
		{
			ifacefound = true;
			break;
		}
	}

	if(!ifacefound)
	{
		printf("Given interface name %s was not found!\n", iface.c_str());
		printf("Detected interfaces: ");
		for(it = interfacesOnSystem.begin(); it != interfacesOnSystem.end(); it++)
		{
			printf("\t\t %s\n", it->c_str());
		}
		throw std::invalid_argument("Could not detect iface name");
	}

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

	if(!this->specifiedInterface.empty())
	{
		bool success = bindSocketToInterface(sd, this->specifiedInterface);
		if(!success)
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
				usleep(1000 * 100);	//Wait 100 ms, lower cpu consumption
			}

			len = recvfrom(sd, buf, 1400, 0, &p_sockadr, &p_socklength);
			buf[len] = '\0';
			std::string source = this->DecodeSockAddr(p_sockadr);
			printf("Read %zd bytes from %s: %s\n", len, source.c_str(), buf);


			if (!len) {
				break;
			} else if (len < 0) {
				perror("read");
				return 1;
			} else {
				std::string code = std::string(&buf[0], len);
				receiveCallback(code, p_sockadr);
			}
		}

	close(sd);
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

	if(!this->specifiedInterface.empty())
	{
		bool success = bindSocketToInterface(sd, this->specifiedInterface);
		if(!success)
			return 1;
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

	sendto(sd, buf, length, 0, (const struct sockaddr *) &saddr, sizeof(saddr));

	close(sd);
	return 0;
}


std::string UdpTools::DecodeSockAddr(sockaddr& sockAddr)
{
	char *s = NULL;
	std::string defaultCase = "CouldNotDecodeIP";

	switch(sockAddr.sa_family) {
		case AF_INET: {
			struct sockaddr_in *addr_in = (struct sockaddr_in *)&sockAddr;
			s = (char*)malloc(INET_ADDRSTRLEN);
			inet_ntop(AF_INET, &(addr_in->sin_addr), s, INET_ADDRSTRLEN);
			break;
		}
		case AF_INET6: {
			struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)&sockAddr;
			s = (char*)malloc(INET6_ADDRSTRLEN);
			inet_ntop(AF_INET6, &(addr_in6->sin6_addr), s, INET6_ADDRSTRLEN);
			break;
		}
		default:
			s = (char*)defaultCase.c_str();
	}

	std::string ipAddress = std::string(s);
	free(s);
	return ipAddress;
}

std::list<std::string> UdpTools::getInterfaces()
{
	std::list<std::string> detectedInterfaces;
    struct if_nameindex *if_nidxs, *intf;

    if_nidxs = if_nameindex();
    if ( if_nidxs != NULL )
    {
        for (intf = if_nidxs; intf->if_index != 0 || intf->if_name != NULL; intf++)
        {
            //printf("%s\n", intf->if_name);
            detectedInterfaces.push_back(std::string(intf->if_name));
        }

        if_freenameindex(if_nidxs);
    }
    return detectedInterfaces;
}

bool UdpTools::bindSocketToInterface(int sd, std::string interfaceName)
{
	/*Bind socket to a specific interface if set*/
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), interfaceName.c_str());

	if ((setsockopt(sd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr))) < 0)
	{
		perror("Server-setsockopt() error for SO_BINDTODEVICE");
		return false;
	}
	else
	{
		return true;
	}


}
