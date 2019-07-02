/*
 * UdpTools.h
 *
 *  Created on: Jul 2, 2019
 *      Author: thomas
 */
#include <string>

#ifndef UDPTOOLS_H_
#define UDPTOOLS_H_

class UdpTools {
public:
	UdpTools();
	int SendUDPDatagram(std::string targetAddress, std::string port, char buf[], int length);
	int ReceiveUDPDatagram(std::string listenAddress, std::string listenPort, void (* receiveCallback)(std::string datagram));
	virtual ~UdpTools();
};

#endif /* UDPTOOLS_H_ */
