#include "lcop.h"
#include "rocket.h"

#include <hiredis.h>
#include <async.h>
#include <adapters/libevent.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>

#define	DEFAULT_ROCK_PORT			125
#define REDIS_IPADDRESS				"127.0.0.1"
#define REDIS_PORT					6379

#define MISSIONID                   "rm125"
#define SENSORID                    "1"

#define STR_MISSION_MC1             MISSIONID ":mission:mc1"
#define STR_MISSION_MC2             MISSIONID ":mission:mc2"
#define STR_SETTINGS_DAQ            MISSIONID ":" SENSORID ":settings:daq"
#define STR_SETTINGS_CLEAR          MISSIONID ":" SENSORID ":settings:clear"
#define STR_SETTINGS_HV             MISSIONID ":" SENSORID ":settings:hv"
#define STR_SETTINGS_CHANNELS       MISSIONID ":" SENSORID ":settings:channels"
#define STR_SETTINGS_LOCK           MISSIONID ":" SENSORID ":settings:lock"
#define STR_SETTINGS_FPGA           MISSIONID ":" SENSORID ":settings:fpga"
#define STR_SNAPSHOTS               MISSIONID ":" SENSORID ":snapshots"
#define REDIS_SET_MISSION_MC1       "SET " STR_MISSION_MC1 " %b"
#define REDIS_SET_MISSION_MC2       "SET " STR_MISSION_MC2 " %b"
#define REDIS_SET_SNAPSHOTS         "SET " STR_SNAPSHOTS " %b"
#define REDIS_PUBLISH_SNAPSHOTS     "PUBLISH " STR_SNAPSHOTS " %b"
#define REDIS_MULTI_SUBSCRIBE       "SUBSCRIBE " STR_MISSION_MC1 " " STR_MISSION_MC2 " " STR_SETTINGS_DAQ " " STR_SETTINGS_CLEAR " " STR_SETTINGS_HV " " STR_SETTINGS_CHANNELS " " STR_SETTINGS_LOCK " " STR_SETTINGS_FPGA

/* used to pass arguments to threads and callbacks */
struct thread_arg {
    pthread_mutex_t *rocket_lock;
    pthread_mutex_t *lcop_lock;
    rocket_list_node **rocket_list;
    lcop_queue_item **lcop_queue;
    uint16_t cid;
    redisAsyncContext *redisc;
    redisContext *redissc;
};

void subscribeCallback(redisAsyncContext *c, void *r, void *arg) {
    pthread_mutex_t *lcop_lock = ((struct thread_arg *)arg)->lcop_lock;
    lcop_queue_item **lcop_queue = ((struct thread_arg *)arg)->lcop_queue;
    redisReply *reply = r;
    if (reply == NULL) return;
    if (reply->type == REDIS_REPLY_ARRAY) {
    	int i;
        if (strcmp(reply->element[0]->str, "subscribe") == 0)
            return; /* ignore successful subscribtion */
        else {
            if (strcmp(reply->element[1]->str, STR_MISSION_MC1) == 0) {
                lcop_msg *msg = lcop_new_msg(MISSION_MC1, 
                    reply->element[2]->str, strlen(reply->element[2]->str));
                pthread_mutex_lock(lcop_lock);
                lcop_queue_push(lcop_queue, msg);
                pthread_mutex_unlock(lcop_lock);
                lcop_free(msg);
            }
            else if (strcmp(reply->element[1]->str, STR_MISSION_MC2) == 0) {
                lcop_msg *msg = lcop_new_msg(MISSION_MC2, 
                    reply->element[2]->str, strlen(reply->element[2]->str));
                pthread_mutex_lock(lcop_lock);
                lcop_queue_push(lcop_queue, msg);
                pthread_mutex_unlock(lcop_lock);
                lcop_free(msg);
            }
            else if (strcmp(reply->element[1]->str, STR_SETTINGS_DAQ) == 0) {
                lcop_msg *msg = lcop_new_msg(SETTINGS_DAQ, 
                    reply->element[2]->str, strlen(reply->element[2]->str));
                pthread_mutex_lock(lcop_lock);
                lcop_queue_push(lcop_queue, msg);
                pthread_mutex_unlock(lcop_lock);
                lcop_free(msg);
            }
            else if (strcmp(reply->element[1]->str, STR_SETTINGS_CLEAR) == 0) {
                lcop_msg *msg = lcop_new_msg(SETTINGS_CLEAR, 
                    reply->element[2]->str, strlen(reply->element[2]->str));
                pthread_mutex_lock(lcop_lock);
                lcop_queue_push(lcop_queue, msg);
                pthread_mutex_unlock(lcop_lock);
                lcop_free(msg);
            }
            else if (strcmp(reply->element[1]->str, STR_SETTINGS_HV) == 0) {
                lcop_msg *msg = lcop_new_msg(SETTINGS_HV, 
                    reply->element[2]->str, strlen(reply->element[2]->str));
                pthread_mutex_lock(lcop_lock);
                lcop_queue_push(lcop_queue, msg);
                pthread_mutex_unlock(lcop_lock);
                lcop_free(msg);
            }
            else if (strcmp(reply->element[1]->str, STR_SETTINGS_CHANNELS) == 0) {
                lcop_msg *msg = lcop_new_msg(SETTINGS_CHANNELS, 
                    reply->element[2]->str, strlen(reply->element[2]->str));
                pthread_mutex_lock(lcop_lock);
                lcop_queue_push(lcop_queue, msg);
                pthread_mutex_unlock(lcop_lock);
                lcop_free(msg);
            }
            else if (strcmp(reply->element[1]->str, STR_SETTINGS_LOCK) == 0) {
                lcop_msg *msg = lcop_new_msg(SETTINGS_LOCK, 
                    reply->element[2]->str, strlen(reply->element[2]->str));
                pthread_mutex_lock(lcop_lock);
                lcop_queue_push(lcop_queue, msg);
                pthread_mutex_unlock(lcop_lock);
                lcop_free(msg);
            }
            else if (strcmp(reply->element[1]->str, STR_SETTINGS_FPGA) == 0) {
                lcop_msg *msg = lcop_new_msg(SETTINGS_FPGA, 
                    reply->element[2]->str, strlen(reply->element[2]->str));
                pthread_mutex_lock(lcop_lock);
                lcop_queue_push(lcop_queue, msg);
                pthread_mutex_unlock(lcop_lock);
                lcop_free(msg);
            }
        }
    }

}

void connectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("[redis]\t\tError on connection: %s\n", c->errstr);
        return;
    }
    printf("[redis]\t\tConnected\n");
}

void disconnectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("[redis]\t\tError on disconnection: %s\n", c->errstr);
        return;
    }
    printf("[redis]\t\tDisconnected\n");
}

void *send_thread(void *arg) {
	printf("[lcop]\t\tSend thread started.\n");
	pthread_mutex_t *rocket_lock = ((struct thread_arg *)arg)->rocket_lock;
    pthread_mutex_t *lcop_lock = ((struct thread_arg *)arg)->lcop_lock;
    rocket_list_node **rocket_list = ((struct thread_arg *)arg)->rocket_list;
    lcop_queue_item **lcop_queue = ((struct thread_arg *)arg)->lcop_queue;
    uint16_t cid = ((struct thread_arg *)arg)->cid;

    while (1) {
    	lcop_queue_item *item = 0;
    	pthread_mutex_lock(lcop_lock);
    	if (*lcop_queue != 0)
    		item = lcop_queue_pop(lcop_queue);
    	pthread_mutex_unlock(lcop_lock);
    	if (item == 0) {
    		sleep(1);
    	}
    	else {
    		int s = rocket_send(rocket_list, cid, item->msg, item->length, rocket_lock);
            lcop_queue_item_free(item);
    	}
    }
}

void *recv_thread(void *arg) {
	printf("[lcop]\t\tRecv thread started.\n");
	pthread_mutex_t *rocket_lock = ((struct thread_arg *)arg)->rocket_lock;
    pthread_mutex_t *lcop_lock = ((struct thread_arg *)arg)->lcop_lock;
    rocket_list_node **rocket_list = ((struct thread_arg *)arg)->rocket_list;
    lcop_queue_item **lcop_queue = ((struct thread_arg *)arg)->lcop_queue;
    uint16_t cid = ((struct thread_arg *)arg)->cid;
    redisAsyncContext *c = ((struct thread_arg *)arg)->redisc;
    redisContext *sc = ((struct thread_arg *)arg)->redissc;

    while (1) {
    	char *recvbuffer;
		int r = rocket_recv(rocket_list, cid, &recvbuffer, rocket_lock);
		if (r < 0)
			continue;
		lcop_msg *msg = lcop_deserialize(recvbuffer, r);

        if (msg->type == MISSION_MC1) {
            redisAsyncCommand(c, NULL, NULL, 
                REDIS_SET_MISSION_MC1, msg->payload, msg->paylength);
        }
        else if (msg->type == MISSION_MC2) {
            redisAsyncCommand(c, NULL, NULL, 
                REDIS_SET_MISSION_MC2, msg->payload, msg->paylength);
        }
        else if (msg->type == SNAPSHOTS) {
            /*
            int ret = redisAsyncCommand(c, NULL, NULL, 
                "SET rm125:1:snapshots %b", msg->payload, msg->paylength);
            int ret2 = redisAsyncCommand(c, NULL, NULL, 
                "PUBLISH rm125:1:snapshots %b", msg->payload, msg->paylength);
                */
            redisCommand(sc, REDIS_PUBLISH_SNAPSHOTS, msg->payload, msg->paylength);
            redisCommand(sc, REDIS_SET_SNAPSHOTS, msg->payload, msg->paylength);
        }

		free(recvbuffer);
		lcop_free(msg);
    }
}

int main(int argc, char **argv) {
    pthread_mutex_t *rocket_lock = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_t *lcop_lock = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(rocket_lock, NULL);
    pthread_mutex_init(lcop_lock, NULL);
    rocket_list_node *list = 0;
    lcop_queue_item *queue = 0;
    printf("[lcop]\t\tStarting lcop server...\n");

    /* connect to Redis (either sync & async) */
    signal(SIGPIPE, SIG_IGN);
    struct event_base *base = event_base_new();
    redisContext *sc = redisConnect(REDIS_IPADDRESS, REDIS_PORT);
    redisAsyncContext *c = redisAsyncConnect(REDIS_IPADDRESS, REDIS_PORT);
    if (c->err) {
        printf("[redis]\t\tConnect error: %s\n", c->errstr);
        return -1;
    }
    redisLibeventAttach(c,base);
    redisAsyncSetConnectCallback(c,connectCallback);
    redisAsyncSetDisconnectCallback(c,disconnectCallback);
    //redisAsyncCommand(c, NULL, NULL, "SET key %b", argv[argc-1], strlen(argv[argc-1]));
    //redisAsyncCommand(c, getCallback, (char*)"end-1", "GET key");
    
    /* prepare the rocket and launch control server */
    uint16_t rocket_cid = rocket_server(&list, DEFAULT_ROCK_PORT, rocket_lock);
    rocket_ctrl_server(&list, rocket_lock);

    /* launch send & recv threads */
    pthread_t tid_send, tid_recv;
    struct thread_arg *arg = malloc(sizeof(struct thread_arg));
    arg->rocket_lock = rocket_lock;
    arg->lcop_lock = lcop_lock;
    arg->rocket_list = &list;
    arg->lcop_queue = &queue;
    arg->cid = rocket_cid;
    arg->redisc = c;
    arg->redissc = sc;
    pthread_create(&tid_send, NULL, send_thread, arg);
    pthread_create(&tid_recv, NULL, recv_thread, arg);

    /* subscribe to redis selected channels */
    redisAsyncCommand(c, subscribeCallback, (struct thread_arg *)arg, REDIS_MULTI_SUBSCRIBE);

    /* start the libevent loop in order to receive async calls */
    event_base_dispatch(base);
    //pthread_join(tid_send, NULL);
    //pthread_join(tid_recv, NULL);
    //redisAsyncDisconnect(c);
	return 0;
}