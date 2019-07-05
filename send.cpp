/*	IPv6 multicast example - ipv6_multicast_send.c
	2012 - Bjorn Lindgren <nr@c64.org>
	https://github.com/bjornl/ipv6_multicast_example
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "UdpTools.h"

int main(int argc, char *argv[])
{
	if (argc < 3) {
		printf("\nUsage: %s <address> <port>\n\nExample: %s ff02::5:6 12345\n\n", argv[0], argv[0]);
		return 1;
	}

	if(argc != 3 && argc != 4)
	{
		printf("Usage: send address port. Example send ff02::5:6 12345\n");
		printf("Usage (need be root): send iface-name address port. Example sudo send eth0 ff02::5:6 12345\n");
		return -1;
	}

	std::string sendAddress = std::string(argv[1]);
	std::string port = std::string(argv[2]);
	std::string ifaceName;
	UdpTools* p_udpSender;

	if(argc == 3)
	{
		if(argv[1] == NULL || argv[2] == NULL)
		{
			printf("Usage: listen address port. Example receive ff02::5:6 12345\n");
			return -1;
		}
		sendAddress = std::string(argv[1]);
		port = std::string(argv[2]);
		p_udpSender = new UdpTools();

	}

	if(argc == 4)
	{
		if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL)
		{
			printf("Usage: listen iface-name address port. Example receive eth0 ff02::5:6 12345\n");
			return -1;
		}
		ifaceName = std::string(argv[1]);
		sendAddress = std::string(argv[2]);
		port = std::string(argv[3]);
		p_udpSender = new UdpTools(ifaceName);
	}

	std::string message = "AreYouThere?";
	p_udpSender->SendUDPDatagram(true, sendAddress, port, message.c_str(), message.length());

}
