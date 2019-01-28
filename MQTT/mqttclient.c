#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <stdbool.h>
#include <err.h>
#include <unistd.h>
#include <time.h>

#define STATUS_CONNECTING 0
#define STATUS_CONNACK_RECVD 1
#define MAX_ARR_SIZE 255
#define KEEPALIVE 60
#define QOS 0
#define RETAIN 0
#define TOPIC "/testing"
#define PRV_KEY "private.key"
#define CERT_FILE "cert.crt"
#define ROOTCA "rootca.pem"
#define CLI_ID "latency_testing"
#define MQTT_SEC_PORT 8883
#define MAX_SIZE 1024 

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

static struct mosquitto *mosq = NULL;
static int status = STATUS_CONNECTING;
static int mid_sent = 0;
static long long pub_ts = 0;
static char buf[MAX_SIZE] = {0};
static int *lat_array = NULL;
static int ix = 0;
static int no_of_pkts = 0;

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

void my_connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	int rc = MOSQ_ERR_SUCCESS;
    status = STATUS_CONNACK_RECVD;
    printf ("Connect CB result : %d rc : %d\n", result, rc);

	if(!result){
		if(rc){
			switch(rc){
					case MOSQ_ERR_INVAL:
						fprintf(stderr, "Error: Invalid input. Does your topic contain '+' or '#'?\n");
						break;
					case MOSQ_ERR_NOMEM:
						fprintf(stderr, "Error: Out of memory when trying to publish message.\n");
						break;
					case MOSQ_ERR_NO_CONN:
						fprintf(stderr, "Error: Client not connected when trying to publish.\n");
						break;
					case MOSQ_ERR_PROTOCOL:
						fprintf(stderr, "Error: Protocol error when communicating with broker.\n");
						break;
					case MOSQ_ERR_PAYLOAD_SIZE:
						fprintf(stderr, "Error: Message payload is too large.\n");
						break;
		    }
            printf ("Calling disconnect\n");
			mosquitto_disconnect(mosq);
		}
	}else{
		if(result){
			printf("%s\n", mosquitto_connack_string(result));
		}
	}
    mosquitto_subscribe(mosq, NULL, TOPIC, QOS); 
}

void my_log_callback(struct mosquitto *mosq, void *obj, int level, const char *str)
{
	printf("Log CB %s\n", str);
}

void my_disconnect_callback(struct mosquitto *mosq, void *obj, int rc)
{
    printf ("Disconnect CB with error code : %d\n", rc);
}

void my_publish_callback(struct mosquitto *mosq, void *obj, int mid)
{
    //mid is the unique MQTT message ID
    printf ("Publish success CB msgid : %d\n", mid);
}

void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
    int i;

    printf("Subscribe success CB (mid: %d): %d", mid, granted_qos[0]);
    for(i = 1; i < qos_count; i++){
        printf(", %d", granted_qos[i]);
    }
    printf("\n");
    pub_ts = current_timestamp ();
    mosquitto_publish(mosq, &mid_sent, TOPIC, strlen(buf), buf, QOS, RETAIN);
}

void my_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
    printf ("Message %d received on subscribed topic\n", (ix + 1));
    if(message->payloadlen)
    {
        long long cur_time = current_timestamp ();
        lat_array[ix] = cur_time - pub_ts;
        pub_ts = current_timestamp ();   	 
        mosquitto_publish(mosq, &mid_sent, TOPIC, strlen(buf), buf, QOS, RETAIN);
        ix ++;
        if (ix == no_of_pkts) 
        {
            printf ("Finished publishing %d pkts, exiting application\n", no_of_pkts);
			mosquitto_disconnect(mosq);
        }
    }
    else
    {
        printf("%s (null)\n", message->topic);
    }
}

int client_connect(struct mosquitto *mosq, struct mosq_config *cfg)
{
    int rc;
    rc = mosquitto_connect (mosq, cfg->host, cfg->port, KEEPALIVE);
    if(rc > 0)
    {
        if(rc == MOSQ_ERR_ERRNO)
        {
            printf("Connect error: %s\n", err);
        }
        else
        {
            printf("Unable to connect (%s).\n", mosquitto_strerror(rc));
        }
        mosquitto_lib_cleanup();
        return rc;
    }
    return MOSQ_ERR_SUCCESS;
}

int client_opts_set(struct mosquitto *mosq, struct mosq_config *cfg)
{
	if((cfg->cafile || cfg->capath)
			&& mosquitto_tls_set(mosq, cfg->cafile, cfg->capath, cfg->certfile, cfg->prvkeyfile, NULL))
    { 
		printf("Error: Problem setting TLS options.\n");
		mosquitto_lib_cleanup();
		return 1;
	}
	mosquitto_opts_set(mosq, MOSQ_OPT_PROTOCOL_VERSION, &(cfg->protocol_version));
	return MOSQ_ERR_SUCCESS;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf ("Usage : %s <endpoint> <payload_size> <no.of packets>\n", argv[0]);
		return -1;
    }

    printf ("Staring MQTT application\n");
    mosq_config_t cfg;
	memset(&cfg, 0, sizeof(mosq_config_t));
	cfg.port = MQTT_SEC_PORT;
	strcpy (cfg.id, CLI_ID);
	cfg.protocol_version = MQTT_PROTOCOL_V31;
    //a2zacfu9uv98fr-ats.iot.us-west-2.amazonaws.com
    strcpy (cfg.host, argv[1]);
    strcpy (cfg.prvkeyfile, PRV_KEY);
    strcpy (cfg.certfile, CERT_FILE); 
    strcpy (cfg.cafile, ROOTCA); 
    strcpy (cfg.capath, "."); 

    no_of_pkts = atoi (argv[3]);
    lat_array = (int *) calloc (no_of_pkts, sizeof (int));

    int buf_size = atoi (argv[2]);
	int i = 0;
    memset (buf, 0x00, MAX_SIZE);
    for (i = 0; i < buf_size; i++)
    {	
	    buf[i] = 'a';	
    }

    mosquitto_lib_init();
	mosq = mosquitto_new(cfg.id, true, NULL);
	if(!mosq)
    {
		mosquitto_lib_cleanup();
		return -1;
	}
    mosquitto_log_callback_set(mosq, my_log_callback);
    mosquitto_connect_callback_set(mosq, my_connect_callback);
	mosquitto_publish_callback_set(mosq, my_publish_callback);
    mosquitto_message_callback_set(mosq, my_message_callback);
    mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);
	mosquitto_disconnect_callback_set(mosq, my_disconnect_callback);

	if(client_opts_set(mosq, &cfg)){
		return -1;
	}
    int rc = client_connect(mosq, &cfg);
    if (rc) 
    {
        printf ("Connection failure : %d\n", rc);
        mosquitto_destroy(mosq);
        return -1;
    }           
    
    mosquitto_loop_forever (mosq, -1, 1);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    min_array (lat_array, no_of_pkts);
    max_array (lat_array, no_of_pkts);
    avg_array (lat_array, no_of_pkts);
    if (lat_array != NULL)
    {
        free (lat_array);
        lat_array = NULL;
    }
    printf ("Exiting Application\n");
    return 0;
}
