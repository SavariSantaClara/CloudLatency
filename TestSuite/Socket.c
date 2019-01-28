#include "Common.h"

static int sockfd = -1;
static int *lat_array = NULL;

int create_client_socket (char *hostname, int port, SOCK_TYPE_T type, struct sockaddr_in *serveraddr)
{
    printf ("Entering %s:%d\n", __func__, __LINE__);
    struct hostent *server = NULL;
    
    if (type == SOCK_UDP)
    { 
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    }
    else if (SOCK_TCP)
    {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
    }
    else 
    {
        printf ("Invalid socket type\n");
        return -1;
    }

    if (sockfd < 0)
    { 
        perror("ERROR opening socket");
        return -1;
    }

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        printf("ERROR, no such host as %s\n", hostname);
        return -1;
    }
    /* build the server's Internet address */
    memset (serveraddr, 0x00, sizeof(struct sockaddr_in));
    serveraddr->sin_family = AF_INET;
    serveraddr->sin_port = htons(port);
    memcpy (server->h_addr, &serveraddr->sin_addr.s_addr, server->h_length);
   
    if (type == SOCK_TCP)
    { 
        if (connect(sockfd, (void *)serveraddr, sizeof(struct sockaddr_in)) < 0)
        { 
            perror("ERROR connecting");
        }
    }
    printf ("Exiting %s:%d\n", __func__, __LINE__);
    return sockfd;
}

void do_send_recive (struct sockaddr_in *serveraddr, int pkt_size, int no_of_pkts, SOCK_TYPE_T type)
{
    printf ("Entering %s:%d\n", __func__, __LINE__);
    char buf[MAX_SIZE] = {0};
	int i = 0, j = 0, n = -1;
    long long send_time = 0;
    int serverlen = 0;

    lat_array = (int *) calloc (no_of_pkts, (sizeof (int)));

    for (i = 0; i < no_of_pkts; i++)
    {
        memset (buf, 0x00, MAX_SIZE);
        for (j = 0; j < pkt_size; j++)
        {
            buf[j] = 'a';
        }
        /* send the message to the server */
	    send_time = current_timestamp();
        if (type == SOCK_TCP)
        {
            n = send(sockfd, buf, pkt_size, 0);
        }
        else if (type == SOCK_UDP)
        {
            printf ("Trying to UDP send\n");
            serverlen = sizeof(struct sockaddr_in);
            n = sendto(sockfd, buf, strlen(buf), 0, (void *)serveraddr, serverlen);
        }
        if (n < 0)
        { 
            perror("ERROR in sending");
            return;
        }
        
        memset (buf, 0x00, MAX_SIZE);
        if (type == SOCK_TCP)
        {
            n = recv(sockfd, buf, pkt_size, 0);
        }
        else if (type == SOCK_UDP)
        {
            n = recvfrom(sockfd, buf, strlen(buf), 0, (void *)serveraddr, &serverlen);
        }
        if (n < 0)
        { 
            perror("ERROR in recving");
        }
        lat_array[i] = (current_timestamp() - send_time);
	    printf ("Latency is %d microseconds\n", lat_array[i]);
        send_time = 0;
    }
    printf ("Exiting %s:%d\n", __func__, __LINE__);
}

void close_sock_client (int no_of_pkts, int pkt_size)
{
    printf ("Entering %s:%d\n", __func__, __LINE__);
    print_stats (lat_array, no_of_pkts, pkt_size);
    free_ptr (lat_array);
    close_socket (sockfd);
    printf ("Exiting %s:%d\n", __func__, __LINE__);
}
    
