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



// Marchalling
int packData(unsigned char *buffer, unsigned char *instr, unsigned int t1_sec, unsigned int t1_nsec, unsigned int t2_sec, unsigned int t2_nsec) {
	buffer[0] = instr[0];
    buffer[1] = instr[1];
    buffer[2] = instr[2];
    buffer[3] = '\0';
    buffer[4] = t1_sec>>24; 
    buffer[5] = t1_sec>>16;
    buffer[6] = t1_sec>>8;
    buffer[7] = t1_sec;
    buffer[8] = t1_nsec>>24; 
    buffer[9] = t1_nsec>>16;
    buffer[10] = t1_nsec>>8;
    buffer[11] = t1_nsec;
    buffer[12] = t2_sec>>24; 
    buffer[13] = t2_sec>>16;
    buffer[14] = t2_sec>>8;
    buffer[15] = t2_sec;
    buffer[16] = t2_nsec>>24; 
    buffer[17] = t2_nsec>>16;
    buffer[18] = t2_nsec>>8;
    buffer[19] = t2_nsec;
    return 0;
}

void unpackData(unsigned char *buffer, unsigned char *instr) {
    instr[0] = buffer[0];
    instr[1] = buffer[1];
    instr[2] = buffer[2];
    instr[3] = buffer[3];
}


int main(int argc, char *argv[])
{
    int udpsockfd, status;

    struct sockaddr_in myaddr, their_addr;  // local address information
    socklen_t addr_size;
    struct timespec t2, t3;
    char instr[3];

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
            clock_gettime(CLOCK_REALTIME, &t2);
			unpackData(udpbuffer, instr);
			printf("received: %s \n", instr);

            clock_gettime(CLOCK_REALTIME, &t3);
            printf("%d", sizeof(time_t));
            printf("RES %lld.%ld\n", (long long)t2.tv_sec, t2.tv_nsec);
            printf("RES %d.%d\n\n", (unsigned int)t2.tv_sec, (unsigned int)t2.tv_nsec);
            printf("RES %lld.%ld\n", (long long)t3.tv_sec, t3.tv_nsec);
            printf("RES %d.%d\n\n", (unsigned int)t3.tv_sec, (unsigned int)t3.tv_nsec);
			packData(udpbuffer, "RES", (unsigned int)t2.tv_sec, (unsigned int)t2.tv_nsec, (unsigned int)t3.tv_sec, (unsigned int)t3.tv_nsec);
			sendto(udpsockfd, udpbuffer, sizeof(udpbuffer), 0, (const struct sockaddr *)&their_addr, addr_size);
			break;
        }
    }


    return 0;
}
