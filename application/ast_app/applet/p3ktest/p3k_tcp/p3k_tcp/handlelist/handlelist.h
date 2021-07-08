#ifndef __HANDLELIST_H__
#define __HANDLELIST_H__



#ifdef __cplusplus
extern "C" {
#endif 
#include <pthread.h>


typedef struct _HandleList_S{
	void *handle;
	pthread_mutex_t 	selfMutex;
	struct _HandleList_S	*next;//下一个
	struct _HandleList_S	*pre;//上一个
}HandleList_S;

typedef int (*HandleManageLoopDoFunc)(void *handle,int argc,void *arg[]);

int HandleManageInitHead(HandleList_S *head);
int HandleManageDelHandle(HandleList_S *head,void *handle);
int HandleManageAddHandle(HandleList_S *head,void *handle);
int HandleManageGetUsrCount(HandleList_S *head);
int HandleManagePostLoop(HandleList_S *head,HandleManageLoopDoFunc fun,int argc,void *arg[]);
void* HandleManageGetNextHandle(HandleList_S *head);

/*

函数作用: 通过某个成员变量的值来获取整个Handle的头位置
参数说明
第一个参数是listhead 
第二个参数是要对比的成员变量名字
第三个参数是要对比的成员变量值
第四个参数是返回值句柄
第五个参数是返回值类型
实现逻辑:
	1. 遍历List列表
	2. 强制转换任意类型的handle为structmem 类型
	3. 取structmem->类型的值与value 进行对比
	4. 如果成功就将 resultHanlde = 当前任意类型的handle
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

