#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>

// /login a 1 128.100.13.245 3000

#define SERVER_TCP_PORT 3000 /* well-known port */
#define BUFLEN 1000  /* buffer length */
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
#define GEN_ACK 14
#define GEN_NACK 15
#define EXIT_ACK 16

typedef struct node {
    int session_id;
    struct node* next;
} node_t;

/*clients login information*/
struct client {
    char username[MAX_NAME];
    char password[14];
    int socket_descriptor;
    //int session_id;
    node_t* session_hp;
    struct client *next;
};

/*Existed sessions*/
struct session {
    int session_id;
    struct client *connected_client;
    struct session *next;
};

struct lab3message {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

struct parameters {
    int new_sock;
    struct client* userdb;
    struct client** clientHead;
    struct session** sessionHead;
};

struct client* user_initialization(void) {
    struct client* head;
    struct client *A = malloc(sizeof (struct client));
    struct client *B = malloc(sizeof (struct client));
    struct client *C = malloc(sizeof (struct client));
    struct client *D = malloc(sizeof (struct client));

    strcpy(A->username, "a");
    strcpy(B->username, "b");
    strcpy(C->username, "c");
    strcpy(D->username, "d");

    strcpy(A->password, "1");
    strcpy(B->password, "2");
    strcpy(C->password, "3");
    strcpy(D->password, "4");

    head = A;
    A->next = B;
    B->next = C;
    C->next = D;
    D->next = NULL;

    return head;
}

struct lab3message parser(char recMsg[]) {
    struct lab3message tempPack;
    //printf("PARSER INPUT: %s \n", recMsg);
    tempPack.type = atoi(strtok(recMsg, ":"));
    tempPack.size = atoi(strtok(NULL, ":"));
    strcpy(tempPack.source, strtok(NULL, ":"));
    strcpy(tempPack.data, strtok(NULL, "")); //consider changing to memcpy

    return tempPack;
};

char* packetToStr(struct lab3message packet) {
    char dummy[BUFLEN];
    sprintf(dummy, "%u:%u:%s:%s", packet.type, packet.size, packet.source, packet.data);
    char* output = malloc(strlen(dummy) * sizeof (char));
    strcpy(output, dummy);
    return output;
}

bool insertSessionToClient(node_t** head, int session_id) {
    node_t* curr = *head;
    node_t* newNode = malloc(sizeof (node_t));
    newNode->session_id = session_id;
    newNode->next = NULL;

    // if this is the first session
    if (curr == NULL) {
        *head = newNode;
        return true;
    } else {
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = newNode;
        return true;
    }
}

bool deleteSessionFromClient(node_t** head, int session_id) {
    node_t* curr = *head;
    node_t* temp = NULL;
    node_t* prev = NULL;
    
    while(curr != NULL){
        if(curr->session_id == session_id){
            if(prev == NULL){
                *head = curr->next;
                free(curr);
                return true;
            }
            else{
                prev->next = curr->next;
                free(curr);
                return true;
            }
        }
        prev = curr;
        curr = curr->next;
    }
    
    return false;
/*

    if (curr->session_id == session_id) { // this session is the head
        next = (*head)->next;
        *head = next;
        free(curr);
        return true;
    }

    // if it's not the head, we look at the rest of the list
    while (curr->next != NULL && (curr->next)->session_id != session_id)
        curr = curr->next;
    if ((curr->next)->session_id != session_id) // the key was not found
        return false;

    temp = curr->next; // the one to delete
    curr->next = temp->next; // set the previous pointer
    free(temp);
    return true;
*/
}

node_t* searchSessionOfClient(node_t** head, int session_id) {
    node_t* curr = *head;
    
    while(curr != NULL){
        if(curr->session_id == session_id)
            return curr;
        else{
            curr = curr->next;
        }
    }
    return NULL;

/*
    if (curr->session_id == session_id) { // this session is the head
        return curr;
    }

    // if it's not the head, we look at the rest of the list
    while (curr->next != NULL && (curr->next)->session_id != session_id)
        curr = curr->next;
    if ((curr->next)->session_id != session_id) // the key was not found
        return NULL;

    return curr->next;
*/
}

struct client* client_search(struct client* head, char* username) {
    while (head != NULL) {
        if (!strcmp(head->username, username)) {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

struct session* session_search(struct session* head, int id) {
    while (head != NULL) {
        if (head->session_id == id) {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

int client_delete(struct client** head, char* client_id) {
    struct client* current, *prev;
    prev = NULL;
    for (current = *head; current != NULL; prev = current, current = current->next) {
        if (!strcmp(current->username, client_id)) {
            if (prev == NULL)
                *head = current->next;
            else
                prev->next = current->next;
            free(current);
            return 1;
        }
    }
    return 0;
}

int session_delete(struct session** head, int id) {
    struct session* current, *prev;
    prev = NULL;
    for (current = *head; current != NULL; prev = current, current = current->next) {
        if (current->session_id == id) {
            if (prev == NULL)
                *head = current->next;
            else
                prev->next = current->next;
            free(current);
            return 1;
        }
    }
    return 0;
}

struct client* client_generate(struct client** head, char* client_id, int socket_descriptor) {
    if (client_search(*head, client_id) == NULL) {
        struct client* temp;
        temp = (struct client*) malloc(sizeof (struct client));
        strcpy(temp->username, client_id);
        temp->session_hp = NULL;
        temp->socket_descriptor = socket_descriptor;
        temp->next = NULL;
        if (*head == NULL) {
            *head = temp;
            (*head)->next = NULL;
        } else {
            temp->next = *head;
            *head = temp;
        }
        return *head;
    } else {
        return NULL;
    }
}

struct session* session_generate(struct session** head, int id) {
    if (session_search(*head, id) == NULL) {
        struct session* temp;
        temp = (struct session*) malloc(sizeof (struct session));
        temp->session_id = id;
        temp->connected_client = NULL;
        temp->next = NULL;
        if (*head == NULL) {
            *head = temp;
            (*head)->next = NULL;
        } else {
            temp->next = *head;
            *head = temp;
        }
        return *head;
    } else {
        return NULL;
    }
}

void* client_handler(void* sock) {
    int sockfd = ((struct parameters*) sock)->new_sock;
    struct client* user_db = ((struct parameters*) sock)->userdb;
    struct client** clienthp = ((struct parameters*) sock)->clientHead;
    struct session** sessionhp = ((struct parameters*) sock)->sessionHead;
    int N, size;
    char* bpointer, rbuf[BUFLEN];

    printf("Connection successful.\n");
    while (1) {
        bpointer = rbuf;
        size = BUFLEN;
        while ((N = read(sockfd, bpointer, size)) > 0) {
            bpointer += N;
            size -= N;
        }

        //printf("%s\n", rbuf);

        // Decode the message received.
        struct lab3message temp = parser(rbuf);
        if (temp.type == LOGIN) {
            if (client_search(user_db, temp.source) == NULL) {
                struct lab3message outpacket;
                outpacket.type = LO_NACK;
                strcpy(outpacket.data, "User does not exist.");
                outpacket.size = strlen(outpacket.data);
                strcpy(outpacket.source, temp.source);
                char *dummy = packetToStr(outpacket);
                write(sockfd, dummy, BUFLEN);
                free(dummy); /*Send back lo_nack and state the reason*/
                continue;
            }
            if (client_search(*clienthp, temp.source) != NULL) {
                struct lab3message outpacket;
                outpacket.type = LO_NACK;
                strcpy(outpacket.data, "Already logged in.");
                outpacket.size = strlen(outpacket.data);
                strcpy(outpacket.source, temp.source);
                char *dummy = packetToStr(outpacket);
                write(sockfd, dummy, BUFLEN);
                free(dummy);
            } else {
                struct client* tp = client_search(user_db, temp.source);
                if (!strcmp(tp->password, temp.data)) {
                    /*password correct*/
                    if (client_generate(clienthp, temp.source, sockfd) != NULL) {
                        struct lab3message outpacket;
                        outpacket.type = LO_ACK;
                        outpacket.size = 0;
                        strcpy(outpacket.data, "Login.");
                        strcpy(outpacket.source, temp.source);
                        char *dummy = packetToStr(outpacket);
                        write(sockfd, dummy, BUFLEN);
                        printf("%s login successful. \n", temp.source);
                        free(dummy);
                    }
                } else {
                    struct lab3message outpacket;
                    outpacket.type = LO_NACK;
                    strcpy(outpacket.data, "Incorrect Password.");
                    outpacket.size = strlen(outpacket.data);
                    strcpy(outpacket.source, temp.source);
                    char *dummy = packetToStr(outpacket);
                    write(sockfd, dummy, BUFLEN);
                    free(dummy);
                }
            }
        } 
        else if (temp.type == EXIT) { //close this socket 
            struct client* cp = client_search(*clienthp, temp.source);
            if (cp != NULL) { //logged in
                node_t* curr = cp->session_hp;
                while (curr != NULL) { // delete this client from every session it's in
                    struct session* sp = session_search(*sessionhp, curr->session_id);
                    client_delete(&(sp->connected_client), temp.source);

                    //if this is the last person in the session, delete the session
                    if (sp->connected_client == NULL) {
                        session_delete(sessionhp, curr->session_id);
                    }
                    int toBeDeleted = curr->session_id;
                    deleteSessionFromClient(&(cp->session_hp), toBeDeleted); // free memory of this session from client
                    curr = curr->next;
                }
                client_delete(clienthp, temp.source); //log out
            }

            struct lab3message outpacket;
            outpacket.type = EXIT_ACK;
            strcpy(outpacket.data, "Exit");
            outpacket.size = strlen(outpacket.data);
            strcpy(outpacket.source, temp.source);
            char *dummy = packetToStr(outpacket);
            write(sockfd, dummy, BUFLEN);
            free(dummy);

            printf("Exit successful. \n");
            close(sockfd);
            break;
        } 
        else if (temp.type == JOIN) {
            struct client* cp = client_search(*clienthp, temp.source);
            struct lab3message outpacket;
            int joinThisSess = atoi(temp.data);

            if (cp == NULL) {
                outpacket.type = JN_NACK;
                sprintf(outpacket.data, "%d:Not logged in.", atoi(temp.data));
                outpacket.size = strlen(outpacket.data);
                strcpy(outpacket.source, temp.source);
                char *dummy = packetToStr(outpacket);
                write(sockfd, dummy, BUFLEN);
                free(dummy); /*Not logged in */
                continue;
            }

            if (searchSessionOfClient(&(cp->session_hp), joinThisSess) != NULL) {
                outpacket.type = JN_NACK;
                sprintf(outpacket.data, "%d:Already in this session.", joinThisSess);
                outpacket.size = strlen(outpacket.data);
                strcpy(outpacket.source, temp.source);
                char *dummy = packetToStr(outpacket);
                write(sockfd, dummy, BUFLEN);
                free(dummy);
                continue;
            }

            struct session* sessiontemp = session_search(*sessionhp, joinThisSess);
            if (sessiontemp != NULL) {
                /*Join the session*/
                client_generate(&(sessiontemp->connected_client), temp.source, sockfd);
                insertSessionToClient(&(cp->session_hp), joinThisSess);

                /*Response*/
                outpacket.type = JN_ACK;
                sprintf(outpacket.data, "Joined session %d", joinThisSess);
                outpacket.size = strlen(outpacket.data);
                strcpy(outpacket.source, temp.source);
                char *dummy = packetToStr(outpacket);
                write(sockfd, dummy, BUFLEN);
                printf("%d join successful. \n", joinThisSess);
                free(dummy);
            } else {
                outpacket.type = JN_NACK;
                sprintf(outpacket.data, "%d:Session does not exist.", atoi(temp.data));
                outpacket.size = strlen(outpacket.data);
                strcpy(outpacket.source, temp.source);
                char *dummy = packetToStr(outpacket);
                write(sockfd, dummy, BUFLEN);
                free(dummy);
            }
        } 
        else if (temp.type == LEAVE_SESS) {
            struct client* cp = client_search(*clienthp, temp.source);
            int leaveThisSession = atoi(temp.data);

            if (cp != NULL) { //logged in
                if (searchSessionOfClient(&(cp->session_hp), leaveThisSession) != NULL) { //in this session
                    deleteSessionFromClient(&(cp->session_hp), leaveThisSession); // leave from this session
                    struct session* sp = session_search(*sessionhp, leaveThisSession);
                    client_delete(&(sp->connected_client), temp.source);
                    if (sp->connected_client == NULL) { //if this is the last person in the session, delete the session
                        session_delete(sessionhp, leaveThisSession);
                    }

                    struct lab3message outpacket;
                    outpacket.type = GEN_ACK;
                    sprintf(outpacket.data, "Leave session %d", leaveThisSession);
                    outpacket.size = strlen(outpacket.data);
                    strcpy(outpacket.source, temp.source);
                    char *dummy = packetToStr(outpacket);
                    write(sockfd, dummy, BUFLEN);
                    free(dummy);

                    printf("Leave session %d successful. \n", leaveThisSession);
                    continue;
                } else { // not in this session so send error message
                    struct lab3message outpacket;
                    outpacket.type = GEN_NACK;
                    sprintf(outpacket.data, "Not in session %d.", leaveThisSession);
                    outpacket.size = strlen(outpacket.data);
                    strcpy(outpacket.source, temp.source);
                    char *dummy = packetToStr(outpacket);
                    write(sockfd, dummy, BUFLEN);
                    free(dummy);
                }
            } else {
                struct lab3message outpacket;
                outpacket.type = GEN_NACK;
                strcpy(outpacket.data, "Not logged in.");
                outpacket.size = strlen(outpacket.data);
                strcpy(outpacket.source, temp.source);
                char *dummy = packetToStr(outpacket);
                write(sockfd, dummy, BUFLEN);
                free(dummy);
            }
        } 
        else if (temp.type == NEW_SESS) {
            struct lab3message outpacket;
            int sessionToCreate = atoi(temp.data);
            struct session* sp = session_search(*sessionhp, sessionToCreate);
            struct client* cp = client_search(*clienthp, temp.source);

            if (cp == NULL) {
                outpacket.type = GEN_NACK;
                strcpy(outpacket.data, "Not logged in.");
                outpacket.size = strlen(outpacket.data);
                strcpy(outpacket.source, temp.source);
                char *dummy = packetToStr(outpacket);
                write(sockfd, dummy, BUFLEN);
                free(dummy);
                continue;
            }

            if (sp != NULL) { // error message
                outpacket.type = GEN_NACK;
                strcpy(outpacket.data, "Session already exists.");
                outpacket.size = strlen(outpacket.data);
                strcpy(outpacket.source, temp.source);
                char *dummy = packetToStr(outpacket);
                write(sockfd, dummy, BUFLEN);
                free(dummy);
                continue;
            } else { // create and join
                struct session* sessiontemp = session_generate(sessionhp, sessionToCreate);
                client_generate(&(sessiontemp->connected_client), temp.source, sockfd);
                insertSessionToClient(&(cp->session_hp), sessionToCreate);

                /*Response*/
                outpacket.type = NS_ACK;
                sprintf(outpacket.data, "Created and joined session %d", sessionToCreate);
                outpacket.size = strlen(outpacket.data);
                strcpy(outpacket.source, temp.source);
                char *dummy = packetToStr(outpacket);
                write(sockfd, dummy, BUFLEN);
                printf("%d create and join successful. \n", sessiontemp->session_id);
                free(dummy);
            }
        } 
        else if (temp.type == MESSAGE) {
            struct lab3message outpacket;
            struct client* cp = client_search(*clienthp, temp.source);

            if (cp == NULL) {
                outpacket.type = GEN_NACK;
                strcpy(outpacket.data, "Not logged in.");
                outpacket.size = strlen(outpacket.data);
                strcpy(outpacket.source, temp.source);
                char *dummy = packetToStr(outpacket);
                write(sockfd, dummy, BUFLEN);
                free(dummy);
                continue;
            }

            int sessionToShare = atoi(strtok(temp.data, ":"));
            char messageToShare[BUFLEN]; strcpy(messageToShare, strtok(NULL, ""));

            if (searchSessionOfClient(&(cp->session_hp), sessionToShare) == NULL) {
                outpacket.type = GEN_NACK;
                strcpy(outpacket.data, "Not in this session.");
                outpacket.size = strlen(outpacket.data);
                strcpy(outpacket.source, temp.source);
                char *dummy = packetToStr(outpacket);
                write(sockfd, dummy, BUFLEN);
                free(dummy);
                continue;
            }

            struct session* sp = session_search(*sessionhp, sessionToShare);
            struct client* curr = sp->connected_client;
            while (curr != NULL) {
                if (strcmp(curr->username, cp->username)) { // skip itself
                    outpacket.type = MESSAGE;
                    strcpy(outpacket.data, messageToShare);
                    outpacket.size = strlen(outpacket.data);
                    strcpy(outpacket.source, temp.source);
                    char *dummy = packetToStr(outpacket);
                    write(curr->socket_descriptor, dummy, BUFLEN);
                    free(dummy);
                } else {
                    outpacket.type = GEN_ACK;
                    strcpy(outpacket.data, "Message sent.");
                    outpacket.size = strlen(outpacket.data);
                    strcpy(outpacket.source, temp.source);
                    char *dummy = packetToStr(outpacket);
                    write(curr->socket_descriptor, dummy, BUFLEN);
                    free(dummy);
                }
                curr = curr->next;
            }

        } 
        else if (temp.type == QUERY) {
            struct lab3message outpacket;
            struct client* cp = client_search(*clienthp, temp.source);

            if (cp == NULL) {
                outpacket.type = GEN_NACK;
                strcpy(outpacket.data, "Not logged in.");
                outpacket.size = strlen(outpacket.data);
                strcpy(outpacket.source, temp.source);
                char *dummy = packetToStr(outpacket);
                write(sockfd, dummy, BUFLEN);
                free(dummy);
                continue;
            }

            // for each client, determine the sessions they're in
            struct client* curr = *clienthp;
            curr = *clienthp;
            strcpy(outpacket.data, "QUERY: \n");

            while (curr != NULL) {
                outpacket.type = QU_ACK;
                char add1[BUFLEN], add2[BUFLEN];
                node_t* currSess = curr->session_hp;

                if (currSess == NULL)
                    sprintf(add1, "Client %s is not part of any sessions.\n", curr->username);
                else
                    sprintf(add1, "Client %s is in session(s): ", curr->username);
                strcat(outpacket.data, add1);

                while (currSess != NULL) {
                    sprintf(add2, "%d ", currSess->session_id);
                    strcat(outpacket.data, add2);
                    currSess = currSess->next;
                }

                strcat(outpacket.data, "\n");
                curr = curr->next;
            }

            outpacket.size = strlen(outpacket.data);
            strcpy(outpacket.source, temp.source);
            char *dummy = packetToStr(outpacket);
            write(sockfd, dummy, BUFLEN);
            //            printf("sending from query server: %s \n", dummy);
            free(dummy);
        }
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
    pthread_t t[100];
    int pcount = 0;
    /*User login Information*/
    struct client *user_all = user_initialization();

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
        printf("Ready! \n");
        if ((new_sd = accept(sd, NULL, NULL)) == -1) {
            fprintf(stderr, "Can't accept client\n");
            exit(1);
        }

        thread_parameter = malloc(sizeof (struct parameters));
        thread_parameter->new_sock = new_sd;
        thread_parameter->userdb = user_all;
        thread_parameter->clientHead = &login;
        thread_parameter->sessionHead = &sessions;

        if (pthread_create(t + pcount, NULL, client_handler, (void*) thread_parameter)) {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }
        pcount++;
    }

    close(sd);
    return (0);
}