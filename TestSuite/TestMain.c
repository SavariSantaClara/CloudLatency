#include "Common.h"
#include "Socket.h"
#include "Mqtt.h"

int main (int argc, char *argv[])
{
    if (argc != 6)
    {
        printf ("Usage : %s <proto i.e u/t/m> <hostname> <port> <no_of_pkts> <pkt_size>\n", argv[0]);
        return 0;
    }

    char hostname[MAX_SIZE] = {0};
    strcpy (hostname, argv[2]);
    int port = atoi (argv[3]);
    int no_of_pkts = atoi (argv[4]);
    int pkt_size = atoi (argv[5]);
    
	if ((strcmp (argv[1], "m") == 0))
    {
        printf ("Running MQTT latency test for %d pkts of size %d bytes\n", no_of_pkts, pkt_size);
        struct mosquitto *mosq = NULL;
        struct mosq_config *cfg = NULL;
        cfg = (struct mosq_config *) calloc (1, sizeof (struct mosq_config));

        init_mqtt_client_params (cfg, hostname, no_of_pkts, pkt_size);
        mqtt_client_connect_and_start (mosq, cfg, hostname);
        close_mqtt_client (mosq, no_of_pkts, pkt_size);
        free_ptr (cfg);
    }
	else if ((strcmp (argv[1], "u") == 0))
    {
        printf ("Running UDP latency test for %d pkts of size %d bytes\n", no_of_pkts, pkt_size);
        struct sockaddr_in *serveraddr = NULL;
        serveraddr = (struct sockaddr_in *) calloc (1, sizeof(struct sockaddr_in));
        
        create_client_socket (hostname, port, SOCK_UDP, serveraddr);
        do_send_recive (serveraddr, pkt_size, no_of_pkts, SOCK_UDP);
        close_sock_client (no_of_pkts, pkt_size);
        free_ptr (serveraddr);
    }
	else if ((strcmp (argv[1], "t") == 0))
    {
        printf ("Running TCP latency test for %d pkts of size %d bytes\n", no_of_pkts, pkt_size);
        struct sockaddr_in *serveraddr = NULL;
        serveraddr = (struct sockaddr_in *) calloc (1, sizeof(struct sockaddr_in));

        create_client_socket (hostname, port, SOCK_TCP, serveraddr);
        do_send_recive (serveraddr, pkt_size, no_of_pkts, SOCK_TCP);
        close_sock_client (no_of_pkts, pkt_size);
        free_ptr (serveraddr);
    }
    else
    {
        printf ("Invalid mode argument, it should be u for UDP, t for TCP or m for MQTT");
    }
    return 0;
}
