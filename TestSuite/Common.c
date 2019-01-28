#include "Common.h"

long long current_timestamp()
{
    printf ("Entering %s:%d\n", __func__, __LINE__);
    struct timespec tp;
    clock_gettime (CLOCK_REALTIME, &tp);
    long long milliseconds = tp.tv_sec*1000LL + tp.tv_nsec/1000000; // calculate milliseconds
    long long microseconds = tp.tv_sec*1000000LL + tp.tv_nsec/1000; // calculate microseconds
    printf ("Exiting %s:%d\n", __func__, __LINE__);
    return microseconds;
}

void avg_array(int v[], int len)
{
    printf ("Entering %s:%d\n", __func__, __LINE__);
    float sum = 0;
    for (int i = 0; i < len; i++)
    {
            sum += v[i];
    }
    printf("sum = %f\n",sum);
    printf("avg = %f\n", (sum/len));
    printf ("Exiting %s:%d\n", __func__, __LINE__);
    return;
}

void min_array(int v[], int len)
{
    printf ("Entering %s:%d\n", __func__, __LINE__);
    int min = v[0];
    for (int i = 0; i < len; i++)
    {
        if (v[i] < min)
            min = v[i];
    }
    printf("min = %d\n",min);
    printf ("Exiting %s:%d\n", __func__, __LINE__);
    return;
}

void max_array(int v[], int len)
{
    printf ("Entering %s:%d\n", __func__, __LINE__);
    int max = v[0];
    for(int i = 0; i < len; i++)
    {
        if (v[i] > max)
            max = v[i];
    }
    printf("max = %d\n",max);
    printf ("Exiting %s:%d\n", __func__, __LINE__);
    return;
}

void free_ptr (void *ptr)
{
    printf ("Entering %s:%d\n", __func__, __LINE__);
    if (ptr != NULL)
    {
        free (ptr);
        ptr = NULL;
    }
    printf ("Exiting %s:%d\n", __func__, __LINE__);
    return;
}

void close_socket (int sockfd)
{
    printf ("Entering %s:%d\n", __func__, __LINE__);
    shutdown (sockfd, 2);
    close (sockfd);
    sockfd = -1;
    printf ("Exiting %s:%d\n", __func__, __LINE__);
}

void print_stats (int *lat_array, int no_of_pkts, int pkt_size)
{
    printf ("Entering %s:%d\n", __func__, __LINE__);
    printf ("For %d packets of size %d the latencies are: \n", no_of_pkts, pkt_size);
    max_array (lat_array, no_of_pkts);
    min_array (lat_array, no_of_pkts);
    avg_array (lat_array, no_of_pkts);
    printf ("Exiting %s:%d\n", __func__, __LINE__);
    return;
}
