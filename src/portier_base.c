
#include <bartlby.h>


/*
json_object *my_string, *my_int, *my_object, *my_array, *my_object1, *my_object2;

        my_object = json_object_new_object();
        json_object_object_add(my_object, "key1", json_object_new_int(12));
        json_object_object_add(my_object, "key2", json_object_new_int(13));
        json_object_object_add(my_object, "key3", json_object_new_string("asdf"));

        printf("obj: %s\n", json_object_to_json_string(my_object));




        json_object_put(my_object);


        my_object1 = json_tokener_parse(argv[1]);
        if(json_object_object_get_ex(my_object1, "key1", &my_object2)) {
                printf("KEY1: %s \n", json_object_get_string(my_object2));
        }


        json_object_put(my_object1);
        json_object_put(my_object2);



      

        
                json_object_object_foreach(my_object, key, val)
        {
                printf("\t%s: %s\n", key, json_object_to_json_string(val));
        }
       

       
*/

int bartlby_portier_connect(char *host_name,int port);
int bartlby_portier_send_trigger(char * passive_host, int passive_port, int passive_cmd, int to_standbys,char * trigger_name, char * execline, struct service * svc, int node_id, char * portier_passwd);
int bartlby_portier_send_svc_status(char * passive_host, int passive_port, char * passwd, struct service * svc, char * cfgfile);



static sig_atomic_t portier_connection_timed_out=0;
#define CONN_TIMEOUT 15



static void bartlby_portier_conn_timeout(int signo) {
	portier_connection_timed_out=1;
}

//Caller has to free
char * bartlby_portier_fetch_reply(int sock) {
		char buffer[2048];
		portier_connection_timed_out=0;
		alarm(5);
		if(read(sock, buffer, 2047) < 0) {
			_log(LH_TRIGGER, B_LOG_CRIT, "Portier: fetching reply failed\n");
			return NULL;
		}
		if(portier_connection_timed_out == 1) {
			return NULL;
		} else {
			return strdup(buffer);
		}
		alarm(0);
}

void bartlby_portier_disconnect(int sock) {
	close(sock);
}

int bartlby_portier_send_cmd(json_object * obj, char * host, int port) {
	int sock;

	sock = bartlby_portier_connect(host, port);
	const char * json_package = json_object_to_json_string(obj);


	portier_connection_timed_out=0;
	alarm(5);
	if(write(sock, json_package, strlen(json_package)) < 0) {
			return -1;
	}
	alarm(0);
	if(portier_connection_timed_out == 1) {
		_log(LH_TRIGGER, B_LOG_DEBUG, "PORTIER: sending of json package timed out");
		return -1;
	}
	portier_connection_timed_out=0;
	return 1;
}

int bartlby_portier_connect(char *host_name,int port){
	int result;


	struct addrinfo hints, *res, *ressave;
	char ipvservice[20];
	int sockfd;
	
	sprintf(ipvservice, "%d",port);
	
	 memset(&hints, 0, sizeof(struct addrinfo));

   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
	
	 result = getaddrinfo(host_name, ipvservice, &hints, &res);
	 if(result < 0) {
	 		return -7;
	}
	ressave = res;
	 
	sockfd-1;
	while (res) {
        sockfd = socket(res->ai_family,
                        res->ai_socktype,
                        res->ai_protocol);

        if (!(sockfd < 0)) {
            if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
                break;

            close(sockfd);
            sockfd=-1;
        }
    res=res->ai_next;
  }
  freeaddrinfo(ressave);
 
	return sockfd;
	
	
}




////OLD STUFF
int bartlby_portier_send_svc_status(char * passive_host, int passive_port, char * passwd, struct service * svc, char * cfgfile) {
	int res;
	char verstr[2048];
	char cmdstr[2048];
	char result[2048];
	int rc;
	
	int client_socket;
	int client_connect_retval=-1;
	struct sigaction act1, oact1;
	
	
	portier_connection_timed_out=0;
	act1.sa_handler = bartlby_portier_conn_timeout;
	sigemptyset(&act1.sa_mask);
	act1.sa_flags=0;
	#ifdef SA_INTERRUPT
	act1.sa_flags |= SA_INTERRUPT;
	#endif
	
	if(sigaction(SIGALRM, &act1, &oact1) < 0) {
		
		return -3; //timeout handler
	
		
	}
	//_log("UPSTREAM START");
	alarm(5);
	client_socket = bartlby_portier_connect(passive_host, passive_port);
	alarm(0);
	if(portier_connection_timed_out == 1 || client_socket == -1) {
		return -4; //connect
	} 
	portier_connection_timed_out=0;

	res=client_socket;
	if(res > 0) {
		
		portier_connection_timed_out=0;
		alarm(5);
		if(read(res, verstr, 1024) < 0) {
			_log(LH_TRIGGER, B_LOG_CRIT, "UPSTREAM FAILED1!\n");
			return -1;
		}
		if(verstr[0] != '+') {
			_log(LH_TRIGGER, B_LOG_CRIT,"UPSTREAM: Server said a bad result: '%s'\n", verstr);
			close(res);
			return -1;
		}
		alarm(0);
		/*
		svc->last_check,
		svc->new_server_text,
		svc->current_state,
		svc->last_notify_send,
		svc->last_state_change,
		svc->service_ack_current,
		svc->service_retain_current,
		svc->handled,
		*/	
		sprintf(cmdstr, "8|%d|%d|%d|%d|%d|%d|%d|%d|%s|\n", svc->service_id, svc->current_state, svc->last_notify_send,  svc->last_state_change, svc->service_ack_current, svc->service_retain_current, svc->handled, svc->last_check, svc->new_server_text);
		_debug("SENDING SVC OBJ: '%s'", cmdstr);
		portier_connection_timed_out=0;
		alarm(5);
		if(write(res, cmdstr, 1024) < 0) {
			return -1;
		}
		alarm(0);
		portier_connection_timed_out=0;
		alarm(5);
		if((rc=read(res, result, 1024)) < 0) {
			_log(LH_TRIGGER, B_LOG_DEBUG,"Portier CMD Failed");
			return -1;
		}
		alarm(0);
		result[rc-1]='\0'; //cheap trim *fg*
		close(res);			
		if(result[0] != '+') {
			_log(LH_TRIGGER, B_LOG_DEBUG,"PORTIER CMD FAILED4 - '%s'\n", result);
			return -1;
		}  else {
			//_log("UPSTREAM DONE: %s\n", result);
			return 0;
		}
		
	} else {
		_log(LH_TRIGGER, B_LOG_DEBUG,"PORTIER CMD FAILED FAILED5");
		return -1;
	}	
	return 0;
}

int bartlby_portier_send_trigger(char * passive_host, int passive_port, int passive_cmd, int to_standbys,char * trigger_name, char * execline, struct service * svc, int node_id, char * portier_passwd) {
	int res;
	char verstr[2048];
	char cmdstr[2048];
	char result[2048];
	int rc;
	
	int client_socket;
	int client_connect_retval=-1;
	struct sigaction act1, oact1;
	
	
	portier_connection_timed_out=0;
	

	
	
	
	act1.sa_handler = bartlby_portier_conn_timeout;
	sigemptyset(&act1.sa_mask);
	act1.sa_flags=0;
	#ifdef SA_INTERRUPT
	act1.sa_flags |= SA_INTERRUPT;
	#endif
	
	if(sigaction(SIGALRM, &act1, &oact1) < 0) {
		
		return -3; //timeout handler
	
		
	}
	//_log("UPSTREAM START");
	alarm(5);
	client_socket = bartlby_portier_connect(passive_host, passive_port);
	alarm(0);
	if(portier_connection_timed_out == 1 || client_socket == -1) {
		return -4; //connect
	} 
	portier_connection_timed_out=0;

	res=client_socket;
	if(res > 0) {
		
		portier_connection_timed_out=0;
		alarm(5);
		if(read(res, verstr, 1024) < 0) {
			_log(LH_TRIGGER, B_LOG_CRIT, "UPSTREAM FAILED1!\n");
			return -1;
		}
		if(verstr[0] != '+') {
			_log(LH_TRIGGER, B_LOG_CRIT,"UPSTREAM: Server said a bad result: '%s'\n", verstr);
			close(res);
			return -1;
		}
		alarm(0);
		/*
		svc->service_id
		svc->server_id
		svc->notify_last_state
		svc->current_state
		svc->recovery_outstanding
		*/	
		if(svc != NULL) {
			sprintf(cmdstr, "%d|%d|%s|%s|%d|%d|%d|%d|%d|%d|%s|\n", passive_cmd, to_standbys, execline, trigger_name, svc->service_id, svc->server_id, svc->notify_last_state, svc->current_state, svc->recovery_outstanding, node_id, portier_passwd);
		} else {
			sprintf(cmdstr, "%d|%d|%s|%s|%d|%d|%d|%d|%d|%d|%s|\n", passive_cmd, to_standbys, execline, trigger_name, 0, 0, 0, 0, 0, node_id, portier_passwd);
		}
		//_log("UPSTREAM: sending '%s'", cmdstr);
		portier_connection_timed_out=0;
		alarm(5);
		if(write(res, cmdstr, 1024) < 0) {
			//_log("UPSTREAM: FAILED2");
			return -1;
		}
		alarm(0);
		portier_connection_timed_out=0;
		alarm(5);
		if((rc=read(res, result, 1024)) < 0) {
			_log(LH_TRIGGER, B_LOG_DEBUG,"UPSTREAM: FAILED3");
			return -1;
		}
		alarm(0);
		result[rc-1]='\0'; //cheap trim *fg*
		close(res);			
		if(result[0] != '+') {
			_log(LH_TRIGGER, B_LOG_DEBUG,"UPSTREAM: FAILED4 - '%s'\n", result);
			return -1;
		}  else {
			//_log("UPSTREAM DONE: %s\n", result);
			return 0;
		}
		
	} else {
		_log(LH_TRIGGER, B_LOG_DEBUG,"UPSTREAM: FAILED5");
		return -1;
	}	
	return 0;
}



