#include "lcop.h"

#include <stdio.h>
#include <string.h>

lcop_msg *lcop_new_msg(lcop_msg_type type, char *payload, int length) {
	lcop_msg *msg = malloc(sizeof(lcop_msg));
	msg->type = type;
	msg->length = 0;
	msg->paylength = length;
	msg->payload = malloc(msg->paylength);
	msg->payload = memcpy(msg->payload, payload, length);
	msg->length += calculateTypeLength(msg->type);
	msg->length += 1; /* NEWLINE */
	msg->length += msg->paylength;
	return msg;
}

char *lcop_serialize(lcop_msg *msg) {
	char *serialized = malloc(msg->length);
	int cursor = 0;
	switch (msg->type) {
		case MISSION_MC1:
			memcpy(serialized, "MISSION_MC1 ", 12);
			cursor += 12;
			break;
		case MISSION_MC2:
			memcpy(serialized, "MISSION_MC2 ", 12);
			cursor += 12;
			break;
		case SETTINGS_DAQ:
			memcpy(serialized, "SETTINGS_DAQ ", 13);
			cursor += 13;
			break;
		case SETTINGS_CLEAR:
			memcpy(serialized, "SETTINGS_CLEAR ", 15);
			cursor += 15;
			break;
		case SETTINGS_HV:
			memcpy(serialized, "SETTINGS_HV ", 12);
			cursor += 12;
			break;
		case SETTINGS_CHANNELS:
			memcpy(serialized, "SETTINGS_CHANNELS ", 18);
			cursor += 18;
			break;
		case SETTINGS_LOCK:
			memcpy(serialized, "SETTINGS_LOCK ", 14);
			cursor += 14;
			break;
		case SETTINGS_FPGA:
			memcpy(serialized, "SETTINGS_FPGA ", 14);
			cursor += 14;
			break;
		case SNAPSHOTS:
			memcpy(serialized, "SNAPSHOTS ", 10);
			cursor += 10;
			break;
		default:
			break;
	}
	serialized[cursor] = '\n';
	cursor++;
	memcpy(serialized+cursor, msg->payload, msg->paylength);
	return serialized;
}

lcop_msg *lcop_deserialize(char *str, uint32_t length) {
	lcop_msg *msg = malloc(sizeof(lcop_msg));
	msg->length = length;
	/* strtok is not thread-safe so we use a temp variable
	and the function strtok_r to guarantee thread safety */
	char **temptok = malloc(sizeof(char **));
	char *pos = strchr(str, '\n');
	int nlpos = pos - str;	/* new line position */
	char *header = malloc(nlpos);
	memcpy(header, str, nlpos);
	int headerlen = nlpos;

	/* extract message type */
	char *type = strtok_r(header, " ", temptok);
	if (strcmp(type, "MISSION_MC1") == 0)
		msg->type = MISSION_MC1;
	else if (strcmp(type, "MISSION_MC2") == 0)
		msg->type = MISSION_MC2;
	else if (strcmp(type, "SETTINGS_DAQ") == 0)
		msg->type = SETTINGS_DAQ;
	else if (strcmp(type, "SETTINGS_CLEAR") == 0)
		msg->type = SETTINGS_CLEAR;
	else if (strcmp(type, "SETTINGS_HV") == 0)
		msg->type = SETTINGS_HV;
	else if (strcmp(type, "SETTINGS_CHANNELS") == 0)
		msg->type = SETTINGS_CHANNELS;
	else if (strcmp(type, "SETTINGS_LOCK") == 0)
		msg->type = SETTINGS_LOCK;
	else if (strcmp(type, "SETTINGS_FPGA") == 0)
		msg->type = SETTINGS_FPGA;
	else if (strcmp(type, "SNAPSHOTS") == 0)
		msg->type = SNAPSHOTS;

	/* extract payload */
	int paylength = length - headerlen - 1;
	msg->payload = malloc(paylength);
	msg->paylength = paylength;
	memcpy(msg->payload, str+nlpos+1, paylength);

	free(temptok);
	return msg;
}

/* calculate the length of the serialized message */
int calculateTypeLength(int msgtype) {
	switch (msgtype) {
		case MISSION_MC1:
			return 12; /* MISSION_MC1_ */
		case MISSION_MC2:
			return 12; /* MISSION_MC2_ */
		case SETTINGS_DAQ:
			return 13; /* SETTINGS_DAQ_ */
		case SETTINGS_CLEAR:
			return 15; /* SETTINGS_CLEAR_ */
		case SETTINGS_HV:
			return 12; /* SETTINGS_HV_ */
		case SETTINGS_CHANNELS:
			return 18; /* SETTINGS_CHANNELS_ */
		case SETTINGS_LOCK:
			return 14; /* SETTINGS_LOCK_ */
		case SETTINGS_FPGA:
			return 14; /* SETTINGS_FPGA_ */
		case SNAPSHOTS:
			return 10; /* SNAPSHOTS_ */
		default:
			break;
	}
	return 0;
}

int lcop_queue_push(lcop_queue_item **head, lcop_msg *msg) {
	lcop_queue_item *item = malloc(sizeof(lcop_queue_item));
	item->msg = lcop_serialize(msg);
	item->length = msg->length;
	item->next = 0;
	if (*head == 0) {
		*head = item;
		return 0;
	}
	else {
		lcop_queue_item *x = *head;
		while (x->next != 0) {
			x = x->next;
		}
		x->next = item;
		return 0;
	}
	return -1;
}

lcop_queue_item *lcop_queue_pop(lcop_queue_item **head) {
	lcop_queue_item *ret = *head;
	*head = ret->next;
	return ret;
}

void lcop_print(lcop_msg *msg) {
	printf("+----\n");
	printf("| type:\t\t\t%d\n", msg->type);
	printf("| length:\t\t%d bytes\n", msg->length);
	printf("| payload length:\t%d bytes\n", msg->paylength);
	printf("| payload: %s--END--\n", msg->payload);
	printf("+----\n");
}

void lcop_free(lcop_msg *msg) {
	free(msg->payload);
	free(msg);
}

void lcop_queue_free(lcop_queue_item **head) {
	if (*head == 0)
		return;
	lcop_queue_item *x = *head;
	while (x->next != 0) {
		lcop_queue_item *nextdel = x->next;
		lcop_queue_item_free(x);
		x = nextdel;
	}
	lcop_queue_item_free(x);
}

void lcop_queue_item_free(lcop_queue_item *item) {
	free(item->msg);
	free(item);
}