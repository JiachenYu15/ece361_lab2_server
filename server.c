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

/*Existed sessions*/
struct session{
    int session_id;
    struct client *connected_client;
    struct session *next;
};

struct packet {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

struct parameters{
    int new_sock;
    struct client** clientHead;
    struct session** sessionHead;
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

struct client* client_search(struct client* head, char* username){
    while(head != NULL){
        if(!strcmp(head->username, username)){
            return head;
        }
        head = head->next;
    }
    return NULL;
}

struct session* session_search(struct session* head, int id){
    while(head != NULL){
        if(head->session_id == id){
            return head;
        }
        head = head->next;
    }
    return NULL;
}

struct client* client_generate(struct client** head, char* client_id) {
    if (client_search(*head, client_id) == NULL) {
        struct client* temp;
        temp = (struct client*) malloc(sizeof (struct client));
        strcpy(temp->username, client_id);
        if (*head == NULL) {
            *head = temp;
            (*head)->next = NULL;
        } else {
            temp->next = *head;
            *head = temp;
        }
        return *head;
    }
    else{
        return NULL;
    }
}

struct session* session_generate(struct session** head, int id) {
    if (session_search(*head, id) == NULL) {
        struct session* temp;
        temp = (struct session*) malloc(sizeof (struct session));
        temp->session_id = id;
        if (*head == NULL) {
            *head = temp;
            (*head)->next = NULL;
        } else {
            temp->next = *head;
            *head = temp;
        }
        return *head;
    }
    else{
        return NULL;
    }
}

void* client_handler(void* sock) {
    int sockfd = ((struct parameters*) sock)->new_sock;
    struct client** clienthp = ((struct parameters*) sock)->clientHead;
    //struct session** sessionhp = ((struct parameters*) sock)->sessionHead;
    int N, size;
    char* bpointer, rbuf[BUFLEN];
    while (1) { 
        bpointer = rbuf;
        size = BUFLEN;
        while ((N = read(sockfd, bpointer, size)) > 0) {
            bpointer += N;
            size -= N;
        }

        printf("%s\n", rbuf);
        // Decode the message received.
        struct packet temp ;//= parser(rbuf);
        if(!strcmp(rbuf, "add")){
            if(client_generate(clienthp, "first") != NULL)
                printf("add success\n");
            else
                printf("add fail\n");
        }
        
        else if(!strcmp(rbuf, "search")){
            if(client_search(*clienthp, "first") != NULL)
                printf("found\n");
            else
                printf("not found\n");
        }
        if (temp.type == LOGIN) {
            
        } 
        
        else if (temp.type == EXIT) {

        } 
        
        else if (temp.type == JOIN) {

        } 
        
        else if (temp.type == LEAVE_SESS) {

        }
        
        else if (temp.type == NEW_SESS) {
            
        }
        else if (temp.type == MESSAGE) {
            
        } 
        
        else if (temp.type == QUERY) {
            
        } 
        write(sockfd, rbuf, BUFLEN);
    }
    
    free(sock);
    return NULL;
}

int main(int argc, char **argv) {
    int n, bytes_to_read;
    int sd, new_sd, client_len, port;
    int *new_sock;
    struct sockaddr_in server, client;
    char *bp, buf[BUFLEN];
    char *rbp, rbuf[BUFLEN];
    struct parameters* thread_parameter;
    pthread_t t;

    /*User login Information*/
    struct client* user = user_initialization();

    /*Logged In User*/
    struct client* login = NULL;

    /*Existed Session*/
    struct session* sessions = NULL;

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

    /*queue up to 5 connect requests
     new_sd = accept(sd, (struct sockaddr *) &client, &client_len)*/
    listen(sd, 5);


    while (1) {
        client_len = sizeof (client);
        printf("before\n");
        if ((new_sd = accept(sd, NULL, NULL)) == -1) {
            fprintf(stderr, "Can't accept client\n");
            exit(1);
        }

        thread_parameter = malloc(sizeof(struct parameters));
        thread_parameter->new_sock = new_sd;
        thread_parameter->clientHead = &login;
        thread_parameter->sessionHead = &sessions;
        printf("after\n");
        if (pthread_create(&t, NULL, client_handler, (void*) thread_parameter)) {

            fprintf(stderr, "Error creating thread\n");
            return 1;

        }
        /*
                while(1){
                bp = buf;
                bytes_to_read = BUFLEN;
                while ((n = read(new_sd, bp, bytes_to_read)) > 0) {
                    bp += n;
                    bytes_to_read -= n;
                }
                printf("%s\n", buf);
                // do something with the control message
                //struct packet rPacket = parser(bp);
                /*Actions based on commands
        
                write(new_sd, buf, BUFLEN);
            }
         */
        //close(new_sd);
    }
    close(sd);
    return (0);
}