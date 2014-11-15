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


int min(unsigned int a, unsigned int b) {
    if(a < b) return a;
    else return b;
}

int packData(unsigned char *buffer, unsigned int a, unsigned int b) {
    /* ******************************************************************
    TO BE DONE:  pack data
    ******************************************************************* */
    buffer[0] = a>>8;
    buffer[1] = a;
    buffer[2] = b>>8;
    buffer[3] = b; 	

    return 0;
}

void unpackData(unsigned char *buffer, unsigned int *a, unsigned int *b) {
    *a = (buffer[0]<<8) | buffer[1];
    *b = (buffer[2]<<8) | buffer[3];
}

int gcd(unsigned int a, unsigned int b) {
    int i;
    int gcd = 1;
    
    for(i = 2; i <= min(a,b); i++) {
        if((a%i == 0) && (b%i == 0)) {
            gcd = i;
        }
    }
    return gcd;
}



int main(int argc, char *argv[])
{
    int udpsockfd, status;

    struct sockaddr_in myaddr, their_addr;  // local address information
    socklen_t addr_size;

    unsigned char udpbuffer[MAX_BUFFER_LENGTH];
    unsigned int a, b;

    // usage check / notification
    if(argc != 2) {
        printf("usage Error: use: Server udpPortNumber\n\n");
        exit(1);
    }

    // ---------- UDP --------
    // setup udp address
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = INADDR_ANY;
    myaddr.sin_port = htons(atoi(argv[1])); // UDP Socket

    // init udp sock
    udpsockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(bind(udpsockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0){
		printf("udp binding error\n");
    }

	while(1) {
        addr_size = sizeof(their_addr);
        status = recvfrom(udpsockfd, udpbuffer, sizeof(udpbuffer), 0, (struct sockaddr *)&their_addr, &addr_size);

        if(status <= 0) {
            printf("Error: receiving");
        } else {
			unpackData(udpbuffer, &a, &b);
			printf("received: %d and %d\n", a, b);


			packData(udpbuffer, gcd(a,b), 0);
			printf("-> sending: %d\n", gcd(a,b));
			sendto(udpsockfd, udpbuffer, sizeof(udpbuffer), 0, (const struct sockaddr *)&their_addr, addr_size);
			break;
        }
    }


    return 0;
}
