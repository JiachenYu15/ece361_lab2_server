#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>

#define SERVER_TCP_PORT 3000 /* well-known port */
#define BUFLEN 256  /* buffer length */
#define MAX_NAME 20
#define MAX_DATA 200

#define LOGIN 1
#define LO_ACK 2
#define LO_NACK 3
#define EXIT 4
#define JOIN 5
#define JN_ACK 6
#define JN_NACK 7
#define LEAVE_SESS 8
#define NEW_SESS 9
#define NS_ACK 10
#define MESSAGE 11
#define QUERY 12
#define QU_ACK 13

/*Simple change*/
/*clients login information*/
struct client {
    char username[MAX_NAME];
    char password[14];
    struct client *next;
};

struct packet {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

struct client* user_initialization(void) {
    struct client* head;
    struct client A, B, C, D;
    strcpy(A.username, "a");
    strcpy(B.username, "b");
    strcpy(C.username, "c");
    strcpy(D.username, "d");

    strcpy(A.password, "1");
    strcpy(B.password, "2");
    strcpy(C.password, "3");
    strcpy(D.password, "4");

    head = &A;
    A.next = &B;
    B.next = &C;
    C.next = &D;
    D.next = NULL;

    return head;
}

struct packet parser(char recMsg[BUFLEN]) {
    struct packet tempPack;
    
    tempPack.type = atoi(strtok(recMsg, ":"));
    tempPack.size = atoi(strtok(NULL, ":")); 
    strcpy(tempPack.source, strtok(NULL, ":"));
    strcpy(tempPack.data, strtok(NULL, "")); //consider changing to memcpy
    
    return tempPack;
};

int main(int argc, char **argv) {
    int n, bytes_to_read;
    int sd, new_sd, client_len, port;
    struct sockaddr_in server, client;
    char *bp, buf[BUFLEN];

    /*User database*/


    port = SERVER_TCP_PORT;

    /*Create a stream socket*/
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Can't create a socket\n");
        exit(1);
    }

    /*Bind an address to the socket*/
    bzero((char *) &server, sizeof (struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if ((bind(sd, (struct sockaddr *) &server, sizeof (server))) == -1) {
        fprintf(stderr, "Can't bind name to socket\n");
        exit(1);
    }

    /*queue up to 5 connect requests*/
    listen(sd, 5);


    while (1) {
        client_len = sizeof (client);
        if ((new_sd = accept(sd, (struct sockaddr *) &client, &client_len)) == -1) {
            fprintf(stderr, "Can't accept client\n");
            exit(1);
        }

        bp = buf;
        bytes_to_read = BUFLEN;
        while ((n = read(new_sd, bp, bytes_to_read)) > 0) {
            bp += n;
            bytes_to_read -= n;
        }
        
        // do something with the control message
        struct packet rPacket = parser(bp);
        /*Actions based on commands*/
        
        write(new_sd, buf, BUFLEN);
        close(new_sd);
    }
    close(sd);
    return (0);
}