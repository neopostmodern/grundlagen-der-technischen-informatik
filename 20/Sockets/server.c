/*
############################################################################
#                                                                          #
# Copyright TU-Berlin, 2011-2014                                           #
# Die Weitergabe, Veröffentlichung etc. auch in Teilen ist nicht gestattet #
#                                                                          #
############################################################################
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_BUFFER_LENGTH 100



void unpackData(unsigned char *buffer, unsigned int *a, unsigned int *b) {
    *a = (buffer[0]<<8) | buffer[1];
    *b = (buffer[2]<<8) | buffer[3];
}

int gcd(unsigned int a, unsigned int b) {
    int i;
    int gcd = 1;
    for(i = 2; i < (b/2); i++) {
        if((a%i == 0) && (b%i == 0)) {
            gcd = i;
        }
    }
    return gcd;
}


int main(int argc, char *argv[])
{
    int sockfd, newsockfd, udpsockfd, status;
    int result;
    fd_set readfds;		// socket read list

    struct sockaddr_in myaddr, udpaddr;  // local address information
    struct sockaddr_storage their_addr;
    socklen_t addr_size;

    unsigned char buffer[MAX_BUFFER_LENGTH], udpbuffer[MAX_BUFFER_LENGTH];
    unsigned int a, b;

    // usage check / notification
    if(argc != 3) {
        printf("usage Error: use: Server tcpPortNumber udpPortNumber\n\n");
        exit(1);
    }

    // ---------- TCP --------
    // setup tcp address
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = INADDR_ANY;
    myaddr.sin_port = htons(atoi(argv[1])); // TCP 

    // init tcp sock
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0){
		printf("tcp binding error\n");
    }
    listen(sockfd, 5);


    // ---------- UDP --------
    // setup udp address
    udpaddr.sin_family = AF_INET;
    udpaddr.sin_addr.s_addr = INADDR_ANY;
    udpaddr.sin_port = htons(atoi(argv[2])); // UDP Socket

    // init udp sock
    udpsockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(bind(udpsockfd, (struct sockaddr *)&udpaddr, sizeof(udpaddr)) < 0){
		printf("tcp binding error\n");
    }


    // recive loop
    while(1){
	// wait for input
	    result = 0;
	    while(result == 0){
			// empty readfds list
			FD_ZERO(&readfds);
			// add all sockets to readfds list
			FD_SET(sockfd, &readfds);
			FD_SET(udpsockfd, &readfds);
			
			// wait for interaction
			result = select(udpsockfd + 1, &readfds, NULL, NULL, NULL);
	    }


	// process input
	    if(result != 0){
			if(FD_ISSET(sockfd, &readfds)){
				printf("\n-> incoming @tcp\n");

				// accept incoming connection
					addr_size = sizeof(their_addr);
				newsockfd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
			
				// process message
				int msg;
				msg = recv(newsockfd, buffer, sizeof(buffer), 0);
				printf("tcp msg length %i\n", msg);
				unpackData(buffer, &a, &b);
				printf("tcp received %d and %d. GCD is: %d\n ", a, b, gcd(a,b));
			}	

			if(FD_ISSET(udpsockfd, &readfds)){
				printf("\n-> incoming @udp\n");

				// process message
				int msg;			
				msg = recv(udpsockfd, udpbuffer, sizeof(udpbuffer), 0);
				printf("udp msg length %i\n", msg);
				unpackData(udpbuffer, &a, &b);
				printf("udp received %d and %d. GCD is: %d\n ", a, b, gcd(a,b));
			}

	    }
    }

    return 0;
}
