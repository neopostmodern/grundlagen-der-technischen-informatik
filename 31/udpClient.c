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
#include <time.h>

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

int min(unsigned int a, unsigned int b) {
    if(a < b) return a;
    else return b;
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in their_addr; // connector's address information
    struct hostent *he;
    int serverPort;
    int a = 0;
    int b = 0;
    struct timespec tp1, tp2;

    printf("TCP client example\n\n");

    if (argc != 5) {
        fprintf(stderr,"Usage: tcpClient serverName serverPort int1 int2\n");
        exit(1);
    }

    serverPort = atoi(argv[2]);
    a = atoi(argv[3]);
    b = atoi(argv[4]);

    //Resolv hostname to IP Address
    if ((he=gethostbyname(argv[1])) == NULL) {  // get the host info
        herror("gethostbyname");
        exit(1);
    }

    /* ******************************************************************
    TO BE DONE: Create socket
    ******************************************************************* */

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Socket not created!");
        exit(1);
    }

    //setup transport address
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(serverPort);
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);

    /* ******************************************************************
    TO BE DONE:  Binding
    ******************************************************************* */

    unsigned char buffer[4];

    packData(buffer, a, b);

    /* ******************************************************************
    TO BE DONE:  Send data
    ******************************************************************* */
    clock_gettime(CLOCK_REALTIME, &tp1);
    int result = sendto(sockfd, buffer, sizeof(buffer), 0, (const struct sockaddr *)&their_addr, sizeof(their_addr));
    clock_gettime(CLOCK_REALTIME, &tp2);

    if(result < 0){
	printf("sending ERROR!!!!");
    }

    printf("tp1: %ld\n", (tp1.tv_nsec));
    printf("tp2: %ld\n", (tp2.tv_nsec));


    if(tp2.tv_nsec < tp1.tv_nsec){
	tp2.tv_nsec += 1000000000;
    	printf("\n\n\n time warp!!!!!\n\n\n");
    }
    printf("send time in ns: %ld\n", (tp2.tv_nsec - tp1.tv_nsec));


	recv(sockfd, buffer, sizeof(buffer), 0);
	unpackData(buffer, &a, &b);
	printf("udp received %d and %d. GCD is: %d\n ", a, b, gcd(a,b));


    /* ******************************************************************
    TO BE DONE:  Close socket
    ******************************************************************* */

	close(sockfd);
	 
    return 0;
}
