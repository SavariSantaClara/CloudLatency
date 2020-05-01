/* 
 * tcpclient.c - A simple TCP client
 * usage: tcpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h> 

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

long long current_timestamp()
{

    struct timespec tp;
    clock_gettime (CLOCK_REALTIME, &tp);
    long long milliseconds = tp.tv_sec*1000LL + tp.tv_nsec/1000000; // calculate milliseconds
    long long microseconds = tp.tv_sec*1000000LL + tp.tv_nsec/1000; // calculate milliseconds
    return microseconds;
}

void avg_array(int v[], int len)
{
    float sum = 0;
    for (int i = 0; i < len; i++)
    {
            sum += v[i];
    }
    printf("sum = %f\n",sum);
    printf("avg = %f\n", (sum/len));
}

void min_array(int v[], int len)
{
    int min = v[0];
    for (int i = 0; i < len; i++)
    {
        if (v[i] < min)
            min = v[i];
    }
    printf("min = %d\n",min);
}

void max_array(int v[], int len)
{
    int max = v[0];
    for(int i = 0; i < len; i++)
    {
        if (v[i] > max)
            max = v[i];
    }
    printf("max = %d\n",max);
}

int main(int argc, char **argv) {
    int sockfd, portno, n;
    struct sockaddr_in serveraddr;
    char *hostname;
    struct hostent *server = NULL;
    char *buf = NULL;
	long long send_time = 0;
	int no_of_pkts = 0;
	int pkt_size = 0;
	int *lat_array = NULL;

    /* check command line arguments */
    if (argc != 5) {
       fprintf(stderr,"usage: %s <hostname> <port> <number_of_pkts> <pkt size (1- 4096)>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);
    no_of_pkts = atoi(argv[3]);
    pkt_size = atoi(argv[4]);
    lat_array = (int *) calloc (no_of_pkts, (sizeof(int)));
    buf = (char *) calloc (1, pkt_size);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);
    
    /* connect: create a connection with the server */
    if (connect(sockfd, (void *)&serveraddr, sizeof(serveraddr)) < 0) 
        error("ERROR connecting");


	printf ("Number of packets = %d pkt size = %d\n", no_of_pkts, pkt_size);
	int i = 0, j = 0;
    for (i = 0; i < no_of_pkts; i++)
    {
        /* get message line from the user
        printf("Please enter msg: ");
        bzero(buf, pkt_size);
        fgets(buf, pkt_size, stdin); */
        memset (buf, 0x00, pkt_size);
        for (j = 0; j < pkt_size; j++)
        {
            buf[j] = 'a';
        }
        /* send the message line to the server */
	    send_time = current_timestamp();
        n = send(sockfd, buf, pkt_size, 0);
        if (n < 0)
        {
          error("ERROR writing to socket");
        }

        /* print the server's reply */
        memset (buf, 0x00, pkt_size);
        n = recv(sockfd, buf, pkt_size, 0);
        if (n < 0)
        { 
          error("ERROR reading from socket");
        }
        lat_array[i] = (current_timestamp() - send_time);
        //printf("Echo from server: %s ", buf);
	    printf ("Latency is %d microseconds\n", lat_array[i]);
        send_time = 0;
       
    }
    printf ("For %d packets of size %d\n", no_of_pkts, pkt_size);
    min_array (lat_array, no_of_pkts);
    max_array (lat_array, no_of_pkts);
    avg_array (lat_array, no_of_pkts);
    close(sockfd);
    if (lat_array != NULL)
    {
        free (lat_array);
        lat_array = NULL;
    }
    if (buf != NULL)
    {
        free (buf);
        buf = NULL;
    }
    return 0;
}
