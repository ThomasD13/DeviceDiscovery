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
#include "UdpTools.h"

void ReceiveCallback(std::string datagram, sockaddr& sourceAddr)
{
	std::string toCompare = "AreYouThere?";
	if(datagram.compare(toCompare) != 0)
		return;

	//Send correct code, send our hostname back
	char hostname[1024];

	printf("Now a response with hostname would be sent back!\n");
	gethostname(&hostname[0], 1024);
	std::string hostnameAsString(hostname);

	UdpTools sendBack;
	std::string targetAddress = sendBack.DecodeSockAddr(sourceAddr);

	printf("%s to %s\n", &hostname[0], targetAddress.c_str());

	sendBack.SendUDPDatagram(
			false,
			targetAddress,
			"54321",
			hostnameAsString.c_str(),
			hostnameAsString.length());


}

int main(int argc, char ** argv)
{
	if(argv[1] == NULL || argv[2] == NULL)
		printf("Usage: listen address port. Example receive ff02::5:6 12345\n");

	std::string listenAddress = std::string(argv[1]);
	std::string port = std::string(argv[2]);

	UdpTools udpSender;
	receiveCallbackFcn fcn = ReceiveCallback;

	udpSender.ReceiveUDPDatagram(listenAddress, port, fcn);
}



