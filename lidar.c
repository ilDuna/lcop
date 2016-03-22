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
            
        }
        else if (msg->type == SETTINGS_CLEAR) {
            
        }
        else if (msg->type == SETTINGS_HV) {
            
        }
        else if (msg->type == SETTINGS_CHANNELS) {
            
        }
        else if (msg->type == SETTINGS_LOCK) {
            
        }
        else if (msg->type == SETTINGS_FPGA) {
            
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

        char *test = "snapshots di prova";
        lcop_msg *msg = lcop_new_msg(SNAPSHOTS, test, 18);
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