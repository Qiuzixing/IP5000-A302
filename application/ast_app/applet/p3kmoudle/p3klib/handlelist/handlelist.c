#include <string.h>
#include <stdlib.h>
#include "handlelist.h"


int HandleManageInitHead(HandleList_S *head)
{
	if(head == NULL){
		return -1;
	}
	pthread_mutex_init(&head->selfMutex,NULL);
	pthread_mutex_lock(&head->selfMutex);
	head->handle = NULL;
	head->next = NULL;
	head->pre = NULL;
	pthread_mutex_unlock(&head->selfMutex);
	return 0;
}
int HandleManageAddHandle(HandleList_S *head,void *handle)
{
	if(head == NULL || handle == NULL){
		return -1;
	}
	HandleList_S *cur = NULL;
	HandleList_S *newNode = (HandleList_S *)malloc(sizeof(HandleList_S));
	pthread_mutex_lock(&head->selfMutex);
	for(cur = head;cur->next!= NULL;cur = cur->next);
	cur->next = newNode;
	newNode->pre = cur;
	newNode->next = NULL;
	newNode->handle = handle;
	pthread_mutex_init(&newNode->selfMutex,NULL);
	pthread_mutex_unlock(&head->selfMutex);
	return 0;
}

int HandleManageDelHandle(HandleList_S *head,void *handle)
{
	if(head == NULL || handle == NULL){
		return -1;
	}
	HandleList_S *cur = NULL;
	pthread_mutex_lock(&head->selfMutex);
	for(cur = head->next;cur != NULL;cur = cur->next){
		if(handle == cur->handle){
			cur->pre->next = cur->next;
			if(NULL != cur->next){
				cur->next->pre = cur->pre;
			}
			cur->next = NULL;
			cur->pre = NULL;
			free(cur);
			pthread_mutex_unlock(&head->selfMutex);
			return 0;
		}
	}
	pthread_mutex_unlock(&head->selfMutex);
	return -1;
}


void* HandleManageGetNextHandle(HandleList_S *head)
{
	if(NULL == head || NULL == head->next ){
		return NULL;
	}
	void *handle = NULL;
	HandleList_S *cur = NULL;
	pthread_mutex_lock(&head->selfMutex);
	if(NULL != head->next){
		cur = head->next;
		handle=cur->handle;
	}
	pthread_mutex_unlock(&head->selfMutex);
	return handle;
}

int HandleManageGetUsrCount(HandleList_S *head)
{
	if(head == NULL){
		return -1;
	}
	
	int count = 0;
	HandleList_S *cur = NULL;
	pthread_mutex_lock(&head->selfMutex);
	for(cur = head;cur->next!= NULL;cur = cur->next){
		count++;
	}
	pthread_mutex_unlock(&head->selfMutex);
	return count;
}

int HandleManagePostLoop(HandleList_S *head,HandleManageLoopDoFunc fun,int argc,void *arg[])
{
	if(head == NULL || fun == NULL){
		return -1;
	}
	HandleList_S *cur = NULL;
	pthread_mutex_lock(&head->selfMutex);
	for(cur = head->next;cur != NULL;cur = cur->next){
		fun(cur->handle,argc,arg);
	}
	pthread_mutex_unlock(&head->selfMutex);
	return 0;
}

