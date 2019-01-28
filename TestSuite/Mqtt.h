#ifndef _LT_MQTT_H
#define _LT_MQTT_H

#include <mosquitto.h>
#include <stdbool.h>
#include <err.h>

#define STATUS_CONNECTING 0
#define STATUS_CONNACK_RECVD 1
#define MAX_ARR_SIZE 255
#define KEEPALIVE 60
#define QOS 0
#define RETAIN 0
#define TOPIC "/testing"
#define PRV_KEY "tls_certs/private.key"
#define CERT_FILE "tls_certs/cert.crt"
#define ROOTCA "tls_certs/rootca.pem"
#define CLI_ID "latency_testing"
#define MQTT_SEC_PORT 8883

typedef struct mosq_config {
    char id[MAX_ARR_SIZE];
    int protocol_version;
    char host[MAX_ARR_SIZE];
    char prvkeyfile[MAX_ARR_SIZE];
    char certfile[MAX_ARR_SIZE];
    char cafile[MAX_ARR_SIZE];
    char capath[MAX_ARR_SIZE];
    int port;
} mosq_config_t;

void my_connect_callback(struct mosquitto *mosq, void *obj, int result);
void my_log_callback(struct mosquitto *mosq, void *obj, int level, const char *str);
void my_disconnect_callback(struct mosquitto *mosq, void *obj, int rc);
void my_publish_callback(struct mosquitto *mosq, void *obj, int mid);
void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos);
void my_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);

int client_connect(struct mosquitto *mosq, struct mosq_config *cfg);
int client_opts_set(struct mosquitto *mosq, struct mosq_config *cfg);

void init_mqtt_client_params (mosq_config_t *cfg, char *hostname, int num_pkts, int pkt_size);
int mqtt_client_connect_and_start (struct mosquitto *mosq, struct mosq_config *cfg, char *hostname);
void close_mqtt_client (struct mosquitto *mosq, int no_of_pkts, int pkt_size);

#endif// _LT_MQTT_H
