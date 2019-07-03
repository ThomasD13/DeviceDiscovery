/*	IPv6 multicast example - ipv6_multicast_send.c
	2012 - Bjorn Lindgren <nr@c64.org>
	https://github.com/bjornl/ipv6_multicast_example
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "UdpTools.h"

int sendMulticastDatagram(std::string targetAddress, std::string port, char buf[], int length);

int main(int argc, char *argv[])
{
	if (argc < 3) {
		printf("\nUsage: %s <address> <port>\n\nExample: %s ff02::5:6 12345\n\n", argv[0], argv[0]);
		return 1;
	}

	std::string message = "AreYouThere?";
	char buf[message.length()];
	strcpy(&buf[0], message.c_str());
	int length = sizeof(buf) / sizeof(char);

	UdpTools sendUdp;// = new UdpTools();
	sendUdp.SendUDPDatagram(true, argv[1], argv[2], buf, length);

}
