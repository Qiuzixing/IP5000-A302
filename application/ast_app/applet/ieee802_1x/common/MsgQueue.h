
#ifndef __MSG_QUEUE_H__
#define __MSG_QUEUE_H__

#define TEXT_SIZE		1
#define MSG_MAGIC			0x55
#define MSG_EXIT			0xAA

struct MsgBuf{
	long int mtype;
	char mtext[TEXT_SIZE];
};

class MessageQueue
{
public:
	int QueueInit();
	int QueueDeinit();
	int MsgSend(int i_s32Status);
	int MsgRecv(struct MsgBuf *Msg);
	
	int m_s32QueueId;

private:
	

};


#endif

