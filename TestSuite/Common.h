#ifndef _LT_COMMON_H
#define _LT_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h> 

#define MAX_SIZE 4096

typedef enum 
{
    SOCK_UDP = 100,
    SOCK_TCP = 101,
} SOCK_TYPE_T;

void error(char *msg);
long long current_timestamp();
void free_ptr (void *ptr);
void close_socket (int sockfd);

void min_array(int v[], int len);
void max_array(int v[], int len);
void avg_array(int v[], int len);
void print_stats (int *lat_array, int no_of_pkts, int pkt_size);

#endif
