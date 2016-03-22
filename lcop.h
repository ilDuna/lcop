/*
 *	LCOP
 *	Lidar COntrol Protocol
 */
#include <stdlib.h>

typedef enum {
	MISSION_MC1,
	MISSION_MC2,
	SETTINGS_DAQ,
	SETTINGS_CLEAR,
	SETTINGS_HV,
	SETTINGS_CHANNELS,
	SETTINGS_LOCK,
	SETTINGS_FPGA,
	SNAPSHOTS
} lcop_msg_type;

typedef struct lcop_msg_t {
	lcop_msg_type type;		/* lcop message type/command */
	uint32_t length;		/* length of the serialized msg */
	uint32_t paylength;		/* length of the payload */
	char *payload;			/* data to carry */
} lcop_msg;

typedef struct lcop_queue_item {
	char *msg;						/* serialized message (ready to send) */
	uint32_t length;				/* length of the serialized message */
	struct lcop_queue_item *next;	/* pointer to the next serial. item */
} lcop_queue_item;

lcop_msg *lcop_new_msg(lcop_msg_type type, char *payload, int length);

char *lcop_serialize(lcop_msg *msg);						/* lcop_msg --> string */
lcop_msg *lcop_deserialize(char *str, uint32_t length);		/* string --> lcop_msg */
int calculateTypeLength(int msgtype);

int lcop_queue_push(lcop_queue_item **head, lcop_msg *msg);
lcop_queue_item *lcop_queue_pop(lcop_queue_item **head);

void lcop_print(lcop_msg *msg);
void lcop_free(lcop_msg *msg);
void lcop_queue_free(lcop_queue_item **head);
void lcop_queue_item_free(lcop_queue_item *item);