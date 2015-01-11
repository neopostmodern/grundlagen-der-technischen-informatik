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

void unpackData(unsigned char *buffer, unsigned char *instr, unsigned int *t1_sec, unsigned int *t1_nsec, unsigned int *t2_sec, unsigned int *t2_nsec) {
    instr[0] = buffer[0];
    instr[1] = buffer[1];
    instr[2] = buffer[2];
    instr[3] = buffer[3];
    *t1_sec = (buffer[4]<<24) | (buffer[5]<<16) | (buffer[6]<<8) | buffer[7];
    *t1_nsec = (buffer[8]<<24) | (buffer[9]<<16) | (buffer[10]<<8) | buffer[11];
    *t2_sec = buffer[12]<<24 | buffer[13]<<16 | buffer[14]<<8 | buffer[15];
    *t2_nsec = buffer[16]<<24 | buffer[17]<<16 | buffer[18]<<8 | buffer[19];
}

// Claculates the delay in nanoseconds according to formula: Delay = (T4-T1)-(T3-T2)
void calcDelay(struct timespec *delay, struct timespec *t1, struct timespec *t2, struct timespec *t3, struct timespec *t4) {
    struct timespec help;
    if(t4->tv_nsec + t2->tv_nsec >= 1000000000) {
        delay->tv_nsec = t4->tv_nsec + t2->tv_nsec -  1000000000;
        delay->tv_sec = t4->tv_sec + t2->tv_sec + 1;
    } else {
        delay->tv_nsec = t4->tv_nsec + t2->tv_nsec;
        delay->tv_sec = t4->tv_sec + t2->tv_sec;
    }

    if(t1->tv_nsec + t3->tv_nsec >= 1000000000) {
        help.tv_nsec = t1->tv_nsec + t3->tv_nsec - 1000000000;
        help.tv_sec = t1->tv_sec + t3->tv_sec + 1;
    } else {
        help.tv_nsec = t1->tv_nsec + t3->tv_nsec;
        help.tv_sec = t1->tv_sec + t3->tv_sec;
    }
    
    if(delay->tv_sec > help.tv_sec || (delay->tv_sec == help.tv_sec && delay->tv_nsec >= help.tv_nsec)) {
        if(delay->tv_nsec - help.tv_nsec < 0) {
            delay->tv_nsec = delay->tv_nsec - help.tv_nsec + 1000000000;
            delay->tv_sec = delay->tv_sec - help.tv_sec - 1;
        } else {
            delay->tv_nsec -= help.tv_nsec;
            delay->tv_sec -= help.tv_sec;
        }
    } else {
        if(help.tv_nsec - delay->tv_nsec < 0) {
            help.tv_nsec = help.tv_nsec - delay->tv_nsec + 1000000000;
            help.tv_sec = help.tv_sec - delay->tv_sec - 1;
        } else {
            help.tv_nsec -= delay->tv_nsec;
            help.tv_sec -= delay->tv_sec;
        }
        help.tv_sec *= -1;
        delay->tv_sec = help.tv_sec;
        delay->tv_nsec = help.tv_nsec;
    }
}

void calcOffset(struct timespec *offset, struct timespec *t1, struct timespec *t2, struct timespec *t3, struct timespec *t4) {
	//test values	
	/*t1->tv_sec = 1;
	t1->tv_nsec = 50;
	t2->tv_sec = 2;
	t2->tv_nsec = 1000;
	
	t3->tv_sec = 1;
	t3->tv_nsec = 9000000;
	t4->tv_sec = 3;
	t4->tv_nsec = 0;*/


	// offset = 0.5 * ((t2-t1)+(t3-t4))
	int off;
	off = timespec2ms(t2) - timespec2ms(t1);
	off = off + timespec2ms(t3) - timespec2ms(t4);
	off = off * 0.5;

	ms2timespec(offset, off);
}



int timespec2ms(struct timespec *time){
	return time->tv_sec*1000 + time->tv_nsec/1000000;
}

void ms2timespec(struct timespec *time, int ms){
	time->tv_sec = ms/1000;
	time->tv_nsec = ms*1000000%1000000000;

	if(time->tv_nsec < 0 && time->tv_nsec != 0){
		time->tv_nsec = time->tv_nsec * -1;
	}
}


int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in their_addr; // connector's address information
    struct hostent *he;
    int serverPort;
    char instr[4];
    struct timespec t1, t2, t3, t4, delay, offset;
    unsigned int a,b,c,d;
    socklen_t addrlen;
    int status = 0; // receiving status


    printf("UDP client\n\n");

    if (argc != 3) {
        fprintf(stderr,"Usage: udpClient serverName serverPort\n");
        exit(1);
    }

    serverPort = atoi(argv[2]);

    //Resolv hostname to IP Address
    if ((he=gethostbyname(argv[1])) == NULL) {  // get the host info
        herror("gethostbyname");
        exit(1);
    }

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Socket not created!");
        exit(1);
    }

    //setup transport address
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(serverPort);
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);

    unsigned char buffer[20];

    clock_gettime(CLOCK_REALTIME, &t1);
    packData(buffer, "REQ", 0, 0, 0, 0);
	if(sendto(sockfd, buffer, sizeof(buffer), 0, (const struct sockaddr *)&their_addr, sizeof(their_addr)) < 0){
	    printf("sending ERROR!!!!");
	}

    while(status == 0) {
        status = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&their_addr, &addrlen);
    }
    clock_gettime(CLOCK_REALTIME, &t4);

    unpackData(buffer, instr, &a, &b, &c, &d);
    t2.tv_sec = (time_t)a;
    t2.tv_nsec = (long)b;
    t3.tv_sec = (time_t)c;
    t3.tv_nsec = (long)d;


    printf("Instruction: %s \n----------------------\n\n", instr);
    printf("Time1: %ld.%d \nTime2: %ld.%d \nTime3: %ld.%d \nTime4: %ld.%d\n\n", (long)t1.tv_sec, t1.tv_nsec, (long)t2.tv_sec, t2.tv_nsec, (long)t3.tv_sec, t3.tv_nsec, (long)t4.tv_sec, t4.tv_nsec);
    calcDelay(&delay, &t1, &t2, &t3, &t4);
    printf("Delay: %d.%09d\n", (long)delay.tv_sec, delay.tv_nsec);
	
	calcOffset(&offset, &t1, &t2, &t3, &t4);
	printf("Offset: %d.%09d\n", (long)offset.tv_sec, offset.tv_nsec);
	close(sockfd);
	 

	close(sockfd);

    return 0;
}
