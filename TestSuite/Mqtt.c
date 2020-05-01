#include "Common.h"
#include "Mqtt.h"

static int status = STATUS_CONNECTING;
static int mid_sent = 0;
static long long pub_ts = 0;
static char buf[MAX_SIZE] = {0};
static int *lat_array = NULL;
static int ix = 0;
static int no_of_pkts = 0;

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
    printf ("Entering %s:%d\n", __func__, __LINE__);
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
    printf ("Exiting %s:%d\n", __func__, __LINE__);
    return MOSQ_ERR_SUCCESS;
}

int client_opts_set(struct mosquitto *mosq, struct mosq_config *cfg)
{
    printf ("Entering %s:%d\n", __func__, __LINE__);
	if (cfg->port == MQTT_SEC_PORT)
    {
        if((cfg->cafile || cfg->capath)
	    		&& mosquitto_tls_set(mosq, cfg->cafile, cfg->capath, cfg->certfile, cfg->prvkeyfile, NULL))
        { 
	    	printf("Error: Problem setting TLS options.\n");
	    	mosquitto_lib_cleanup();
	    	return 1;
	    }
    }
	mosquitto_opts_set(mosq, MOSQ_OPT_PROTOCOL_VERSION, &(cfg->protocol_version));
    printf ("Exiting %s:%d\n", __func__, __LINE__);
	return MOSQ_ERR_SUCCESS;
}

void init_mqtt_client_params (mosq_config_t *cfg, char *hostname, int num_pkts, int pkt_size)
{
    printf ("Entering %s:%d\n", __func__, __LINE__);
	memset(cfg, 0, sizeof(mosq_config_t));
	cfg->port = MQTT_UNSEC_PORT;
	strcpy (cfg->id, CLI_ID);
	cfg->protocol_version = MQTT_PROTOCOL_V31;
    //a2zacfu9uv98fr-ats.iot.us-west-2.amazonaws.com
    strcpy (cfg->host, hostname);
    strcpy (cfg->prvkeyfile, PRV_KEY);
    strcpy (cfg->certfile, CERT_FILE); 
    strcpy (cfg->cafile, ROOTCA); 
    strcpy (cfg->capath, "tls_certs/"); 

    no_of_pkts = num_pkts;
    lat_array = (int *) calloc (no_of_pkts, sizeof (int));

	int i = 0;
    memset (buf, 0x00, MAX_SIZE);
    for (i = 0; i < pkt_size; i++)
    {	
	    buf[i] = 'a';	
    }
    printf ("Exiting %s:%d\n", __func__, __LINE__);
    return;
}

int mqtt_client_connect_and_start (struct mosquitto *mosq, struct mosq_config *cfg, char *hostname)
{
    printf ("Entering %s:%d\n", __func__, __LINE__);
    mosquitto_lib_init();
	mosq = mosquitto_new(cfg->id, true, NULL);
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

	if(client_opts_set(mosq, cfg)){
		return -1;
	}
    int rc = client_connect(mosq, cfg);
    if (rc) 
    {
        printf ("Connection failure : %d\n", rc);
        mosquitto_destroy(mosq);
        return -1;
    }           
    mosquitto_loop_forever (mosq, -1, 1);
    printf ("Exiting %s:%d\n", __func__, __LINE__);
}

void close_mqtt_client (struct mosquitto *mosq, int no_of_pkts, int pkt_size)
{
    printf ("Entering %s:%d\n", __func__, __LINE__);
    print_stats (lat_array, no_of_pkts, pkt_size);
    free_ptr (lat_array);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    printf ("Exiting %s:%d\n", __func__, __LINE__);
    return;
}
