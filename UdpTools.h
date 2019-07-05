/*
 * UdpTools.h
 *
 *  Created on: Jul 2, 2019
 *      Author: thomas
 */
#include <string>
#include <sys/socket.h>

#ifndef UDPTOOLS_H_
#define UDPTOOLS_H_

typedef void (*receiveCallbackFcn)(std::string datagram, sockaddr& sourceAddr);

class UdpTools {
public:
	UdpTools();
	int SendUDPDatagram(bool useMulticast, std::string targetAddress, std::string port, const char buf[], int length);


	int ReceiveUDPDatagram(std::string listenAddress, std::string listenPort, receiveCallbackFcn receiveCallback);

	void StopReceiveUDPDatagram();

	std::string DecodeSockAddr(sockaddr& sockAddr);

	virtual ~UdpTools();

private:
	bool stopReceiveThread;
};

#endif /* UDPTOOLS_H_ */
