#include "lcop.h"
#include "rocket.h"

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define	DEFAULT_ROCK_PORT			125
#define DEFAULT_ROCK_IP				"127.0.0.1"


/* used to pass arguments to threads */
struct thread_arg {
    pthread_mutex_t *rocket_lock;
    pthread_mutex_t *lcop_lock;
    rocket_list_node **rocket_list;
    lcop_queue_item **lcop_queue;
    uint16_t cid;
};

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

    while (1) {
    	char *recvbuffer;
		int r = rocket_recv(rocket_list, cid, &recvbuffer, rocket_lock);
		if (r < 0)
			continue;
		lcop_msg *msg = lcop_deserialize(recvbuffer, r);

        if (msg->type == MISSION_MC1) {
            printf("[lcop]\t\tReceived a MISSION_MC1 pkt with payload: %s.\n", msg->payload);
        }
        else if (msg->type == MISSION_MC2) {
            printf("[lcop]\t\tReceived a MISSION_MC2 pkt with payload: %s.\n", msg->payload);
        }
        else if (msg->type == SETTINGS_DAQ) {
            printf("[lcop]\t\tReceived a SETTINGS_DAQ pkt with payload: %s.\n", msg->payload);
        }
        else if (msg->type == SETTINGS_CLEAR) {
            printf("[lcop]\t\tReceived a SETTINGS_CLEAR pkt with payload: %s.\n", msg->payload);
        }
        else if (msg->type == SETTINGS_HV) {
            printf("[lcop]\t\tReceived a SETTINGS_HV pkt with payload: %s.\n", msg->payload);
        }
        else if (msg->type == SETTINGS_CHANNELS) {
            printf("[lcop]\t\tReceived a SETTINGS_CHANNELS pkt with payload: %s.\n", msg->payload);
        }
        else if (msg->type == SETTINGS_LOCK) {
            printf("[lcop]\t\tReceived a SETTINGS_LOCK pkt with payload: %s.\n", msg->payload);
        }
        else if (msg->type == SETTINGS_FPGA) {
            printf("[lcop]\t\tReceived a SETTINGS_FPGA pkt with payload: %s.\n", msg->payload);
        }

		free(recvbuffer);
		lcop_free(msg);
    }
}

void *manager_thread(void *arg) {
    printf("[lcop]\t\tManager thread started.\n");
    pthread_mutex_t *rocket_lock = ((struct thread_arg *)arg)->rocket_lock;
    pthread_mutex_t *lcop_lock = ((struct thread_arg *)arg)->lcop_lock;
    rocket_list_node **rocket_list = ((struct thread_arg *)arg)->rocket_list;
    lcop_queue_item **lcop_queue = ((struct thread_arg *)arg)->lcop_queue;
    uint16_t cid = ((struct thread_arg *)arg)->cid;

    pthread_mutex_lock(rocket_lock);
    rocket_t *rocket = rocket_list_find(*rocket_list, cid);
    pthread_mutex_unlock(rocket_lock);

    while (1) {
        // do some things, schedule others...
        sleep(5);

        /* create the lcop snapshot message with a random value */
        int length = 0;
        char snapshotJSON[10240];
        length = sprintf(snapshotJSON, "[{\"ch\":0,\"eq\":0,\"fc\":8,\"cc\":166,\"es\":1,\"msg\":\"FAIL: AD7 DAL BTP\",\"ec\":40,\"ms\":\"ERR=2106\"}, \
{\"ch\":1,\"eq\":1,\"vq\":0.0,\"gq\":2500,\"rq\":300,\"es\":1,\"vs\":15.0,\"ss\":271,\"rs\":274,\"bs\":271}, \
{\"ch\":2,\"eq\":1,\"vq\":0.0,\"gq\":2500,\"rq\":300,\"es\":1,\"vs\":14.7,\"ss\":284,\"rs\":278,\"bs\":284}, \
{\"ch\":3,\"eq\":1,\"vq\":0.0,\"gq\":2500,\"rq\":300,\"es\":1,\"vs\":14.7,\"ss\":263,\"rs\":255,\"bs\":263}, \
{\"ch\":4,\"eq\":1,\"vq\":0.0,\"gq\":2500,\"rq\":300,\"es\":1,\"vs\":14.9,\"ss\":387,\"rs\":375,\"bs\":386}\
,\
{\"gt\":0,\"dq\":%d,\"ds\":139.032,\"ec\":0,\"ms\":\"Ok, ERR=0\"}, \
{\"gt\":1,\"dq\":2.336,\"wq\":356.667,\"ds\":2.336,\"ws\":356.667}, \
{\"gt\":2,\"dq\":1.168,\"wq\":0.292,\"ds\":1.168,\"ws\":0.292}, \
{\"gt\":3,\"dq\":0.584,\"wq\":1.460,\"ds\":0.584,\"ws\":1.460}, \
{\"gt\":4,\"dq\":0.292,\"wq\":356.667,\"ds\":0.292,\"ws\":356.667}, \
{\"gt\":5,\"dq\":1.372,\"wq\":0.146,\"ds\":1.372,\"ws\":0.146}, \
{\"gt\":6,\"dq\":0.033,\"wq\":10.000,\"ds\":0.033,\"ws\":10.000}, \
{\"gt\":7,\"dq\":140.000,\"wq\":10.000,\"ds\":140.000,\"ws\":10.000}\
,\
{\"ch\":0,\"msg\":\"Ok\",\"ec\":0,\"ms\":\"ERR=0\"}, \
{\"ch\":1,\"eq\":0,\"nq\":1000.0,\"xq\":1200.0,\"tq\":100.0,\"sq\":1.00,\"hq\":400.00}, \
{\"ch\":2,\"eq\":0,\"nq\":1000.0,\"xq\":1200.0,\"tq\":100.0,\"sq\":1.00,\"hq\":400.00}, \
{\"ch\":3,\"eq\":0,\"nq\":1000.0,\"xq\":1200.0,\"tq\":100.0,\"sq\":1.00,\"hq\":400.00}, \
{\"ch\":4,\"eq\":0,\"nq\":1000.0,\"xq\":1200.0,\"tq\":100.0,\"sq\":1.00,\"hq\":400.00}\
,\
{\"sts\":1,\"f\":\"/thisFolder/ThisFile.dat\",\"s\":1000}\
]", rand()%140);

        lcop_msg *msg = lcop_new_msg(SNAPSHOTS, snapshotJSON, length);

        pthread_mutex_lock(lcop_lock);
        lcop_queue_push(lcop_queue, msg);
        pthread_mutex_unlock(lcop_lock);
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
    printf("[lcop]\t\tStarting lcop client...\n");

    /* connect the rocket and receive the cid */
    uint16_t rocket_cid = rocket_client(&list, DEFAULT_ROCK_IP, DEFAULT_ROCK_PORT, rocket_lock);

    /* launch send & recv threads + manager thread */
    pthread_t tid_send, tid_recv, tid_manager;
    struct thread_arg *arg = malloc(sizeof(struct thread_arg));
    arg->rocket_lock = rocket_lock;
    arg->lcop_lock = lcop_lock;
    arg->rocket_list = &list;
    arg->lcop_queue = &queue;
    arg->cid = rocket_cid;
    pthread_create(&tid_send, NULL, send_thread, arg);
    pthread_create(&tid_recv, NULL, recv_thread, arg);
    pthread_create(&tid_manager, NULL, manager_thread, arg);

    //pthread_join(tid_send, NULL);
    //pthread_join(tid_recv, NULL);
    pthread_join(tid_manager, NULL);    //just for debug
	return 0;
}