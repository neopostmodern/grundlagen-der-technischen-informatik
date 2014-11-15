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

int min(unsigned int a, unsigned int b) {
    if(a < b) return a;

    else return b;
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
    int sockfd;
    struct sockaddr_in their_addr; // connector's address information
    struct hostent *he;
    int serverPort;
    int a = 0;
    int b = 0;

    if (argc != 5) {
        fprintf(stderr,"Usage: udpClient serverName serverPort int1 int2\n");
        exit(1);
    }

	//process args
    serverPort = atoi(argv[2]);
    a = atoi(argv[3]);
    b = atoi(argv[4]);

    //resolve hostname to IP Address
    if ((he=gethostbyname(argv[1])) == NULL) {  // get the host info
        herror("gethostbyname");
        exit(1);
    }
    
	//create socket
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Socket not created!");
        exit(1);
    }

    //setup address
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(serverPort);
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);


	//setup buffer
    unsigned char buffer[4];
    packData(buffer, a, b);


    //send data
    printf("rpc.gcd(%d,%d);\n", a, b);
    int result = sendto(sockfd, buffer, sizeof(buffer), 0, (const struct sockaddr *)&their_addr, sizeof(their_addr));
	if(result < 0){
		printf("sending ERROR!!!!");
    }


	//wait for rpc response
	recv(sockfd, buffer, sizeof(buffer), 0);
	unpackData(buffer, &a, &b);
	printf("rpc.gcd(a,b) = %d\n", a);


	close(sockfd);
	 
    return 0;
}
