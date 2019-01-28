/* 
 * tcpserver.c - A simple TCP echo server 
 * usage: tcpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFSIZE 1024
#define MAX_QUEUE 15

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

void * SendRecvData (void *arg)
{
    int n = 0; /* message byte size */
	int childfd = (int) arg;
    char buf[BUFSIZE]; 
	while (1)
    {
        memset(buf, 0x00, BUFSIZE);
        n = recv(childfd, buf, BUFSIZE, 0);
		if (n == 0)
        {
            printf ("Peer closed connection\n");
            break;
        }
        if (n < 0)
        { 
            error("ERROR reading from socket");
        }
    
        printf("server received %d bytes: %s \n", n, buf);
    
        n = send(childfd, buf, n, 0);
        if (n < 0)
        { 
            error("ERROR sending to socket");
        }
    }
    close(childfd);
    childfd = -1;
    return NULL;
}

int main(int argc, char **argv) {
  
    int parentfd; /* parent socket */
    int childfd; /* child socket */
    int portno; /* port to listen on */
    int clientlen; /* byte size of client's address */
    struct sockaddr_in serveraddr; /* server's addr */
    struct sockaddr_in clientaddr; /* client addr */
    struct hostent *hostp; /* client host info */
    char *hostaddrp; /* dotted decimal host addr string */
    int optval; /* flag value for setsockopt */
 
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) {
        perror ("Failed to set thread attribute as detached");
    }

    /* 
     * check command line arguments 
     */
    if (argc != 2) {
      fprintf(stderr, "usage: %s <port>\n", argv[0]);
      exit(1);
    }
    portno = atoi(argv[1]);

    /* 
     * socket: create the parent socket 
     */
    parentfd = socket(AF_INET, SOCK_STREAM, 0);
    if (parentfd < 0) 
      error("ERROR opening socket");

    /* setsockopt: Handy debugging trick that lets 
     * us rerun the server immediately after we kill it; 
     * otherwise we have to wait about 20 secs. 
     * Eliminates "ERROR on binding: Address already in use" error. 
     */
    optval = 1;
    setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, 
           (const void *)&optval , sizeof(int));

    /*
     * build the server's Internet address
     */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)portno);
    clientlen = sizeof(clientaddr);

    if (bind(parentfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) 
        error("ERROR on binding");

    if (listen(parentfd, MAX_QUEUE) < 0) /* allow 15 requests to queue up */ 
        error("ERROR on listen");
      
    while (1) {
        childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
        if (childfd < 0) 
            error("ERROR on accept");
        
        printf("server established connection with %s \n", inet_ntoa(clientaddr.sin_addr));
        /* function starts a new detached thread to send & recv data */
        if (pthread_create(&thread, &attr, SendRecvData, (void *)childfd)) {
            perror ("Failed to create thread");
        }

    }
    pthread_attr_destroy(&attr);
    close (parentfd);
    parentfd = -1;
    return 0;
}

