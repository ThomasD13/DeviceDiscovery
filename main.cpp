#define _GNU_SOURCE   // needed for some IPv6 datatypes to be visible

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// multicast address to send to
const char *maddr = "ff03::1:2:3";
//const char *maddr = "6::1:2:3";

// uncomment the line for the source address you want to use
//const char *srcaddr = "fe80::19f:d834:623a:214d";
const char *srcaddr = "2002::1:2:3";

int main()
{
    int sock;
    struct sockaddr_in6 dstaddr;

    struct iovec iovec[1];
    struct msghdr msg;
    struct cmsghdr* cmsg;
    char msg_control[1024];
    char udp_packet[] = "this is a test";
    int cmsg_space;
    struct in6_pktinfo *pktinfo;

    dstaddr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, maddr, &dstaddr.sin6_addr);
    dstaddr.sin6_port = htons(5555);
    dstaddr.sin6_flowinfo = 0;
    dstaddr.sin6_scope_id = 0;

    if ((sock=socket(AF_INET6, SOCK_DGRAM, 0)) == -1) {
        perror("socket failed");
        exit(1);
    }

    // set up the msghdr structure with the destination address, 
    // buffer to send, and control info buffer
    iovec[0].iov_base = udp_packet;
    iovec[0].iov_len = strlen(udp_packet);
    msg.msg_name = &dstaddr;
    msg.msg_namelen = sizeof(dstaddr);
    msg.msg_iov = iovec;
    msg.msg_iovlen = sizeof(iovec) / sizeof(*iovec);
    msg.msg_control = msg_control;
    msg.msg_controllen = sizeof(msg_control);
    msg.msg_flags = 0;

    // add IPV6_PKTINFO control message to specify source address
    cmsg_space = 0;
    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = IPPROTO_IPV6;
    cmsg->cmsg_type = IPV6_PKTINFO;
    cmsg->cmsg_len = CMSG_LEN(sizeof(*pktinfo));
    pktinfo = (struct in6_pktinfo*) CMSG_DATA(cmsg);
    pktinfo->ipi6_ifindex = 0;
    inet_pton(AF_INET6, srcaddr, &pktinfo->ipi6_addr);
    cmsg_space += CMSG_SPACE(sizeof(*pktinfo));
    msg.msg_controllen = cmsg_space;

    // send packet
    if (sendmsg(sock, &msg, 0) == -1) {
        perror("send failed");
    }

    return 0;
}
