#ifndef __HANDLELIST_H__
#define __HANDLELIST_H__



#ifdef __cplusplus
extern "C" {
#endif 
#include <pthread.h>


typedef struct _HandleList_S{
	void *handle;
	pthread_mutex_t 	selfMutex;
	struct _HandleList_S	*next;//��һ��
	struct _HandleList_S	*pre;//��һ��
}HandleList_S;

typedef int (*HandleManageLoopDoFunc)(void *handle,int argc,void *arg[]);

int HandleManageInitHead(HandleList_S *head);
int HandleManageDelHandle(HandleList_S *head,void *handle);
int HandleManageAddHandle(HandleList_S *head,void *handle);
int HandleManageGetUsrCount(HandleList_S *head);
int HandleManagePostLoop(HandleList_S *head,HandleManageLoopDoFunc fun,int argc,void *arg[]);
void* HandleManageGetNextHandle(HandleList_S *head);

/*

��������: ͨ��ĳ����Ա������ֵ����ȡ����Handle��ͷλ��
����˵��
��һ��������listhead 
�ڶ���������Ҫ�Աȵĳ�Ա��������
������������Ҫ�Աȵĳ�Ա����ֵ
���ĸ������Ƿ���ֵ���
����������Ƿ���ֵ����
ʵ���߼�:
	1. ����List�б�
	2. ǿ��ת���������͵�handleΪstructmem ����
	3. ȡstructmem->���͵�ֵ��value ���жԱ�
	4. ����ɹ��ͽ� resultHanlde = ��ǰ�������͵�handle
*/
#define  HandleManageGetHandleByInt(head,member,value,resultHanlde,structmem) \
{\
	HandleList_S *tpHead = head;\
	resultHanlde = NULL;\
	if(tpHead != NULL){\
		HandleList_S *cur = NULL;\
		pthread_mutex_lock(&tpHead->selfMutex);\
		for(cur = tpHead->next;cur != NULL;cur = cur->next){\
			if(((structmem *)cur->handle)->member == value){\
				resultHanlde = cur->handle;\
				break;\
			}\
		}\
		pthread_mutex_unlock(&tpHead->selfMutex);\
	}\
}

#ifdef __cplusplus
}
#endif

#endif

