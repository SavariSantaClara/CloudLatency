#ifndef _LT_SOCK_H
#define _LT_SOCK_H

int create_client_socket (char *hostname, int port, SOCK_TYPE_T type, struct sockaddr_in *serveraddr);
void do_send_recive (struct sockaddr_in *serveraddr, int pkt_size, int no_of_pkts, SOCK_TYPE_T type);
void close_sock_client (int no_of_pkts, int pkt_size);

#endif // _LT_SOCK_H

