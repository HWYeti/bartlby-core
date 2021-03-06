#include "config.h"



#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/ipc.h>
#include <sys/resource.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/wait.h>	
#include <sys/wait.h>  
#include <time.h>
#include <unistd.h>
#include <unistd.h>
#include <limits.h>

#include <mysql/mysql.h>


/* DEBUGGING */

#define HAVE_DEBUG 1
#define _debug(...) if(HAVE_DEBUG==1) { _log(__VA_ARGS__); }




/* BIT MORE PORTABLE :)*/
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif


/* SIGCHLD fix for OSX*/
#ifndef SIGCLD
#define SIGCLD SIGCHLD
#endif


#ifdef HAVE_SSL 
	#include <openssl/dh.h>
	#include <openssl/ssl.h>
	#include <openssl/err.h>
	
#endif

#ifdef SSH_ADDON 
	#include <libssh/libssh.h>	
#endif


#ifdef SNMP_ADDON
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#endif


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif




#define SNMP_CHECK_TYPE_LOWER 1
#define SNMP_CHECK_TYPE_GREATER 2
#define SNMP_CHECK_TYPE_EQ 3
#define SNMP_CHECK_TYPE_NOT_EQ 4
#define SNMP_CHECK_TYPE_CONTAINS 5


#define PROGNAME "bartlby"
#define REL_NAME_INT "fusion"
#define REL_NAME REL_NAME_INT " - rev:" __GIT_VERSION
//#define VERSION  "1.4.0"
#define EXPECTCORE 1500001

#define MAX_CCACHE 1024
#define MAX_CCACHE_KEY 1024
#define MAX_CCACHE_VALUE 1024


#define RECOVERY_OUTSTANDING 1
#define RECOVERY_DONE 0


#define EXTENSION_OK 0
#define EXTENSION_NOK 1


#define EXTENSION_CALLBACK_ALL -1
#define EXTENSION_CALLBACK_PRE_CHECK 1
#define EXTENSION_CALLBACK_POST_CHECK 2
#define EXTENSION_CALLBACK_STATE_CHANGED 3
#define EXTENSION_CALLBACK_TRIGGER_PRE 4
#define EXTENSION_CALLBACK_SCHED_WAIT 5
#define EXTENSION_CALLBACK_UNKOWN_CHECK_TYPE 6
#define EXTENSION_CALLBACK_ROUND_TIME 7
#define EXTENSION_CALLBACK_CHECK_TIME 8
#define EXTENSION_CALLBACK_EVENT_PUSHED 9
#define EXTENSION_CALLBACK_REPLICATION_GO 10
#define EXTENSION_CALLBACK_TRIGGER_FIRED 11



#define PORTIER_CONN_TIMEOUT 30

#define EVENT_QUEUE_MAX 128
#define EVENT_STATUS_CHANGED 2
#define EVENT_TRIGGER_PUSHED 3



#define STATE_OK 0
#define STATE_WARNING 1
#define STATE_CRITICAL 2
#define STATE_UNKOWN 3
#define STATE_INFO 4
#define STATE_SIRENE 7
#define STATE_DOWNTIME 8

//Service ACK

#define ACK_NOT_NEEDED 0
#define ACK_NEEDED 1
#define ACK_OUTSTANDING 2

#define DT_SERVICE 1
#define DT_SERVER 2
#define DT_SERVERGROUP 3
#define DT_SERVICEGROUP 4


#define SVC_TYPE_ACTIVE 1
#define SVC_TYPE_PASSIVE 2
#define SVC_TYPE_GROUP 3
#define SVC_TYPE_LOCAL 4
#define SVC_TYPE_SNMP 5
#define SVC_TYPE_NRPE 6
#define SVC_TYPE_NRPE_SSL 7
#define SVC_TYPE_V2 8
#define SVC_TYPE_V2_NO_SSL 9
#define SVC_TYPE_SSH 10


#define SVC_THRESHOLD 10

#define PERF_TYPE_SVC_TIME 1
#define PERF_TYPE_ROUND_TIME 2 


#define PASSIVE_TIMEOUT "Passive Service has been timed out"
#define DNS_ERROR "DNS lookup error"
#define SOCKET_CREATE_ERROR "Socket create error"
#define ALARM_ERROR "Alarm setup error"
#define CONN_ERROR "Connection error"
#define RECV_ERROR "Recieve Error"
#define PROTOCOL_ERROR "Protocol Error"
#define TIMEOUT_ERROR "Recv() timedout"





#define GROUP_CRITICAL "Group check critical"
#define GROUP_WITHOUT_PARMS "Group check without parameters"
#define GROUP_OK "Group check OK"



#define LOAD_SYMBOL(x,y,z) 	x=dlsym(y, z); \
    	if((dlmsg=dlerror()) != NULL) { \
        	_log("-Error: %s", dlmsg); \
        	exit(1); \
    	}




struct shm_counter {
	long worker;
	long services;
	long downtimes;
	long servers;	
	long servergroups;
	long servicegroups;
};

struct perf_statistic {
	long sum;
	long counter;	
};


struct snmpi {
	char community[512];
	int version;
	char objid[1024];
	long warn;
	long crit;
	long type;
	char textmatch[1024];
};
struct sprocess {
	int start_time;
	int pid;
		
};

struct sched_threads {
	int pid;
	struct service * svc;
	int start_time;
	int its_over;
} astt;



struct shm_header {
	long size_of_structs;
	long thrdcount;
	long svccount;
	long wrkcount;
	long srvcount;
	long srvgroupcount;
	long svcgroupcount;
	long current_running;
	char  version[50];
	int do_reload;
	int last_replication;
	int startup_time;
	long dtcount;
	int sirene_mode;
	struct perf_statistic pstat;
	int cur_event_index;
	
	
};

struct server {
	long server_id;
	char  client_ip[2048];
	char  server_name[2048];
	char server_icon[1024];
	int server_enabled;
	int client_port;
	int server_dead;
	int server_notify;
	long server_flap_seconds;
	int flap_count;
	int last_notify_send;
	
	struct service * dead_marker;
	int is_gone;
	
	struct servergroup * servergroups[200];
	long servergroup_counter;
	long servergroup_place[200];
	
	char server_ssh_keyfile[512];
	char server_ssh_passphrase[512];
	char server_ssh_username[512];
	
	char enabled_triggers[512];
	
} xxyz;



struct service {
	long service_id;
	long server_id;
	int last_state;
	int current_state;
	
	char  new_server_text[2048];
	char  service_name[2048];
	
	char  plugin[2048];
	char  plugin_arguments[2048];
	long check_interval;
	long check_interval_original;
	int last_check;
	struct timeval lcheck;
	
	/*Time stuff*/
	
	char service_exec_plan[2048];
	
	/*Notify things*/
	int notify_enabled;
	int last_notify_send;
	int last_state_change;
	long flap_count;
	
	int service_active;
	
	char  service_var[2048];
	int service_type;
	long service_passive_timeout;
	
	int notify_last_state;
	
	long service_check_timeout;
	
	
	
	int service_ack_enabled;
	int service_ack_current;
	
	
	long service_retain;
	long service_retain_current;
	
	
	
	struct perf_statistic pstat;
	struct perf_statistic delay_time;
	
	int do_force;
	
	struct snmpi snmp_info;
	
	int recovery_outstanding; //Flag to see if recover is waiting
	
	struct sprocess process;
	
	long flap_seconds;
	
	struct server * srv;
	long srv_place;
	
	int is_server_dead;
	
	/**/
	long renotify_interval; // interval to renotify
	long escalate_divisor; //

	int is_gone;
	
	
	struct servicegroup * servicegroups[200];
	long servicegroup_counter;
	long servicegroup_place[200];
	
	long fires_events;
	
	char enabled_triggers[512];
	
};

struct servicegroup {
	long servicegroup_id;
	char servicegroup_name[1024];
	int servicegroup_notify;
	int servicegroup_active;
	char servicegroup_members[1024];
	
	int servicegroup_dead;
	struct service * dead_marker;
	
	char enabled_triggers[512];
	
};

struct servergroup {
	long servergroup_id;
	char servergroup_name[1024];
	int servergroup_notify;
	int servergroup_active;
	char servergroup_members[1024];
	int servergroup_dead;
	struct service * dead_marker;
	char enabled_triggers[512];
};


struct service_sort {
	struct service * svc;	
};


struct worker {
	char name[2048];
	char  mail[2048];
	char  icq[2048];
	char  services[2048];
	
	long worker_id;
	int active;
	char notify_plan[2048];
	char password[2048];
	
	int escalation_count;
	int escalation_time;
	char notify_levels[20];
	char enabled_triggers[2048];
	char t[500];
	long escalation_limit;
	long escalation_minutes;
	int is_gone;
}sa;


struct downtime {
	long downtime_id;
	int downtime_type;
	int downtime_from;
	int downtime_to;
	char downtime_notice[2048];
	int service_id;
	
	int is_gone;
}sb;

struct btl_event {
	long evnt_id;
	char evnt_message[1024];
	int evnt_time;
		
}eb;
struct ext_notify {
	struct service * svc;	
	struct worker * wrk;
	char * trigger;
} ty;



typedef struct v2_packet_struct{

	u_int32_t crc32_value;
	int16_t   exit_code;
	int16_t   packet_type;
	char      output[2048];
	char      cmdline[2048];
	char      plugin[2048];
	char 	   perf_handler[1024];
	
	 
} agent_v2_packet;

typedef struct port_packet_struct{

	u_int32_t crc32_value;
	int16_t   exit_code;
	int16_t   packet_type;
	char      output[2048];
	char      cmdline[2048];
	char      plugin[2048];
	char 	   perf_handler[1024];
	int32_t	   service_id;
	
	 
} portier_packet;

#define AGENT_V2_SENT_PACKET 1
#define AGENT_V2_RETURN_PACKET 2

#define PORTIER_SVCLIST_PACKET 1
#define PORTIER_RESULT_PACKET 2
#define PORTIER_REQUEST_PACKET 3


char * getConfigValue_ex(const char * key, const char * fname, int cache);
char * getConfigValue(char *, char *);
int clear_serviceMap(struct service **);
int clear_workerMap(struct worker ** m);

int schedule_loop(char *, void *, void *);
void sched_write_back_all(char * cfgfile, void * shm_addr, void * SOHandle);

void sched_reschedule(struct service * svc);
int service_is_in_time(const char * time_plan);
void bartlby_check_service(struct service * svc, void *, void *, char *);
void bartlby_check_active(struct service * svc, char * cfgfile);
void bartlby_check_local(struct service * svc, char * cfgfile);
void bartlby_check_group(struct service * svc, void * shm_addr);


void bartlby_check_snmp(struct service * svc, char * cfgfile);
void bartlby_check_ssh(struct service * svc, char * cfgfile);
void bartlby_check_nrpe(struct service * svc, char * cfgfile, int use_ssl);

void str_mysql_safe(char * str);
void service_mysql_safe(struct service * svc);


//Replication
int replication_go(char *, void *, void *);

//SHM

int GetDowntimeMap(struct downtime * svcs, char * config);
struct service * bartlby_SHM_ServiceMap(void *);
struct sched_threads * bartlby_SHM_ThreadMap(void * shm_addr);
struct downtime * bartlby_SHM_DowntimeMap(void * shm_addr);
struct shm_header * bartlby_SHM_GetHDR(void *);
struct worker * bartlby_SHM_WorkerMap(void * shm_addr);
struct server * bartlby_SHM_ServerMap(void * shm_addr);
struct servicegroup * bartlby_SHM_ServiceGroupMap(void * shm_addr);
struct servergroup * bartlby_SHM_ServerGroupMap(void * shm_addr);

void bartlby_SHM_link_services_servers(void * shm_addr, char * cfgfile, void * SOHandle);

void bartlby_perf_track(struct service * svc,char * return_buffer, int return_bytes, char * cfgfile);
int bartlby_core_perf_track(struct shm_header * hdr, struct service * svc, int type, int time);
long bartlby_milli_timediff(struct timeval end, struct timeval start);

void bartlby_trigger(struct service * svc, char * cfgfile, void * shm_addr, int do_check, int standby_workers_only);
//Global :-)
int _log(const char * str,  ...);

void bartlby_decode(char * msg, int length);
void bartlby_encode(char * msg, int length);
char * bartlby_beauty_state(int status);

void bartlby_end_clean(char *cfgfile);
void bartlby_get_daemon(char * cfgfile);
void set_cfg(char * cfg);

void str_replace(char *str, const char *from, const char *to, int maxlen);
void bartlby_replace_svc_in_str(char * str, struct service * svc, int max);

void bartlby_action_handle_reply(struct service * svc, char * rmessage, char * cfgfile);
int bartlby_action_handle_reply_line(struct service * svc, char * line, char *cfgfile);
void bartlby_check_sirene(char * configfile, void * bartlby_address);
int bartlby_is_in_downtime(void * bartlby_address, struct service * svc);
ssize_t recvall(int _socket, char* buffer, int max_len,int flags);


//EVNT's
void bartlby_event_init(void * bartlby_address);
struct btl_event * bartlby_SHM_EventMap(void * shm_addr);
int bartlby_push_event(int event_id, const char * str,  ...);

void bartlby_pre_init(char * cfgfile);

void cfg_init_cache(void);
char * cfg_add_to_cache(const char * k, char * v);
void cfg_fill_with_file(char * f);
void cfg_update_cache(char * k, char * v);
void bartlby_ext_init(void * shm_addr, void * data_loader, char * cfg);
void bartlby_ext_shutdown(int sched_exit_code);
int bartlby_ext_register_callback(int type, void * fcn);
int bartlby_callback(int type, void *data);

void bartlby_fin_service(struct service * svc, void * SOHandle, void * shm_addr,char * cfgfile);

void bartlby_check_grep_perf_line(char * l, struct service * svc, char * cfgfile);

extern char config_file[255];

int bartlby_tcp_recvall(int s, char *buf, int *len, int timeout);
int bartlby_tcp_sendall(int s, char *buf, int *len);

void bartlby_log_usage(void);

void nrpe_display_license(void);
void bartlby_check_v2(struct service * svc, char * cfgfile, int use_ssl);


int sched_servergroup_notify(struct server * srv);
int sched_servergroup_active(struct server * srv);
int sched_servicegroup_notify(struct service * svc);
int sched_servicegroup_active(struct service * svc);

int bartlby_agent_tcp_my_connect(char *host_name,int port,int *sd,char *proto, struct service * svc);
int bartlby_agent_tcp_connect(char *host_name,int port,int *sd, struct service * svc);
