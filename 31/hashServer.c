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


// -------- HASHTABLE -----------
struct hashnode
{
	int init, key, value;
	struct hashnode *next;
};

int hashKey(int key)
{
	return key % 256;
}


// fix: first node in bucket always will be empty
int hashSet(struct hashnode *table, int key, int value)
{
	// jump to bucket
	int hash = hashKey(key);
	struct hashnode *node;
	node = table + hash;

	while(node != 0){
		// if list.root uninitiated		
		if(node->init == 0){
			node->init = 1;
			node->key = key;
			node->value = value;
			
			return 0;
		}
		// if keyExists -> replace value
		else if(node->key == key){
			return -1;
		}
		// if nextNode == 0 -> insert at end of linkedList
		else if(node->next == 0){
			struct hashnode *newnode;
			newnode = (struct hashnode *) malloc(sizeof(struct hashnode));
			newnode->init = 1;
			newnode->key = key;
			newnode->value = value;
			node->next = (struct hashnode *) newnode;
			
			return 0;
		}


		// check next
		node = (struct hashnode *)node->next;
	}
	
}


int hashGet(struct hashnode *table, int key)
{
	int debug = 0;

	// jump to bucket
	int hash = hashKey(key);
	struct hashnode *node;
	node = table + hash;

	if(debug == 1) { printf("GET traverse key: %d\n", key); }
	// traverse linked list until found or empty
	while(node != 0 && node->init != 0){
		if(debug == 1) { printf("<key:%d> <val:%d> @p: %p   -   ", node->key, node->value, (void *)node);}
		if(node->key == key){
			if(debug == 1) { printf("\n"); }
			return node->value;
		}
		node = (struct hashnode *)node->next;
	}
	
	if(debug == 1) { printf("\n");}
	return -1;
}

int hashDel(struct hashnode *table, int key)
{
	// jump to bucket
	int hash = hashKey(key);
	struct hashnode *node;
	node = table + hash;


	// traverse linked list until found or empty
	while(node != 0 && node->init != 0){

		if(node->key == key){
			if(node->next != 0){
				node->key = node->next->key;
				node->value = node->next->value;
				node->next = node->next->next;
			}else{
				node->init = 0;
				node->key = 0;
				node->value = 0;
				node->next = 0;
			}

			return 0;
		}


		node = (struct hashnode *)node->next;
	}

	return -1;
}


// ---------- UN/PACKING --------------
void unpackData(unsigned char *buffer, char *befehl, unsigned int *key, unsigned int *value) {
    *value = (buffer[6]<<8) | buffer[7];
    *key = (buffer[4]<<8) | buffer[5];
    befehl[3] = buffer[3];
    befehl[2] = buffer[2];
    befehl[1] = buffer[1];
    befehl[0] = buffer[0];
}

int packData(unsigned char *buffer, char *befehl, unsigned int key, unsigned int value) {
	buffer[0] = befehl[0];
    buffer[1] = befehl[1];
    buffer[2] = befehl[2];
    buffer[3] = '\0';
    buffer[4] = key >> 8;
    buffer[5] = key;
    buffer[6] = value >> 8;
    buffer[7] = value;
    return 0;
}


// ------------ MAIN --------------
int main(int argc, char *argv[])
{
    // usage check / notification
    if(argc != 2) {
        printf("usage Error: use: Server udpPortNumber\n\n");
        exit(1);
    }

	int sockfd, status;
    struct sockaddr_in myaddr;  // local address information
    struct sockaddr_storage their_addr;
    socklen_t addr_size;

    unsigned char buffer[MAX_BUFFER_LENGTH];
    unsigned int key, value;
    char befehl[4];

	// alloc hashtable
	struct hashnode *table;
	table = (struct hashnode *) malloc(sizeof(struct hashnode) * 256);


    // ---------- UDP --------
    // setup UDP address
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = INADDR_ANY;
    myaddr.sin_port = htons(atoi(argv[1])); // UDP 

    // init udp sock
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0){
		printf("udp binding error\n");
    }
    

	// ------- CORE LOOP -----------
    while(1) {
        addr_size = sizeof(their_addr);
        status = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&their_addr, &addr_size);

        if(status <= 0) {
            printf("Error: receiving");
        } else {
            unpackData(buffer, befehl, &key, &value);
            printf("%s %d %d \n", befehl, key, value);

			// -- process set cmd
            if(strcmp(befehl, "set") == 0) {
				if(hashSet(table, key, value) == -1){
					strcpy(befehl, "err");
					key = value = 0;
				}else{
					strcpy(befehl, "ok!");
				}
				

			}
			// -- process get cmd
            else if(strcmp(befehl, "get") == 0) {
				int val = hashGet(table, key);

				if(val == -1){
					strcpy(befehl, "nof");
					key = value = 0;
				}else{
					strcpy(befehl, "val");
					value = val;
				}
			}
			// -- process del cmd
            else if(strcmp(befehl, "del") == 0) {
				if(hashDel(table, key) == -1){
					strcpy(befehl, "err");
					key = value = 0;
				}else{
					strcpy(befehl, "ok!");
				}
			}
            else {
				strcpy(befehl, "err");
				key = value = 0;
			}

			// -- return result
            packData(buffer, befehl, key, value);
            printf("-> %s %d %d \n\n", befehl, key, value);
            if(sendto(sockfd, buffer, sizeof(buffer), 0, (const struct sockaddr *)&their_addr, addr_size) < 0){
	            printf("sending ERROR!!!!");
	        }
        }
    }
    

    close(sockfd); 


    return 0;
}
