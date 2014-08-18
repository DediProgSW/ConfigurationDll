#ifndef	_SOCKET_PROTOCOL_H
#define	_SOCKET_PROTOCOL_H

//notify
#define CTRL_NOTIFY_BASE				20000
#define CTRL_NOTIFY_CONNLOST				CTRL_NOTIFY_BASE + 1
#define CTRL_NOTIFY_PROGRESS				CTRL_NOTIFY_BASE + 2
#define CTRL_NOTIFY_BATCH_RESULT			CTRL_NOTIFY_BASE + 3
#define CTRL_NOTIFY_PROGRESS_BATCH			CTRL_NOTIFY_BASE + 4
#define CTRL_NOTIFY_OPERATE_RESULT			CTRL_NOTIFY_BASE + 5
#define CTRL_NOTIFY_DEVICE_LOST             CTRL_NOTIFY_BASE + 6

//message type
#define CTRL_VERSION				0x0100
#define CTRL_COMMAND				0
#define CTRL_NOTIFY				1
#define CTRL_ACK				2

//
#define  MAX_MSG_SIZE				1024*512	//max msg length
#define  MAX_TYPE_SIZE				16	//type string max length
#define  MAX_MANUFACTURE_SIZE			30	//mft string max length
#define  MAX_PARTNUM_SIZE			50	//partnum string max length
#define  MAX_FILENAME_SIZE			64	//filename string max length

#pragma pack(push, 1)
typedef struct {
	/* Number of transfer in bytes,include cp_header */
	unsigned int msglen;
	/* signature = 'Dedi' */
	unsigned int signature;
	/* this version of structure */
	unsigned short version;
	/*  transaction serial num */
	unsigned short transactnum;
	/* message type:cmd,notify,etc */
	unsigned char msgtype;
	/* event */
	unsigned short event;
	//
	unsigned char reserved[8];
} cp_header;
#pragma pack(pop)

#endif
