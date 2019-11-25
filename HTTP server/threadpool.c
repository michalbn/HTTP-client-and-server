#include "threadpool.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

threadpool* create_threadpool(int num_threads_in_pool)
{
	if(num_threads_in_pool>MAXT_IN_POOL || num_threads_in_pool<1)//  1<=numer of thread<=200
	{
		printf("Usage: threadpool <pool-size> <max-number-of-jobs>\n");
		return NULL;
	}
	threadpool* threadpool_p;
	threadpool_p=(threadpool*)malloc(sizeof(threadpool));//create thread pool
	if(threadpool_p==NULL)
	{
		perror("error: <sys_call>\n");
		return NULL;	
	}
	threadpool_p->shutdown=0;
	threadpool_p->dont_accept=0;
	threadpool_p->num_threads=num_threads_in_pool;
	threadpool_p->qsize=0;
	threadpool_p->threads=(pthread_t*)malloc(sizeof(pthread_t)*(threadpool_p->num_threads));//array or threads
	if(threadpool_p->threads==NULL)
	{
		perror("error: <sys_call>\n");
		free(threadpool_p);
		return NULL;
	}
	threadpool_p->qhead=NULL;
	threadpool_p->qtail=NULL;
	
	pthread_mutex_init(&(threadpool_p->qlock),NULL);
	pthread_cond_init(&(threadpool_p->q_not_empty),NULL);
	pthread_cond_init(&(threadpool_p->q_empty),NULL);

	int i,rc;
	for(i=0;i<(threadpool_p->num_threads);i++)
		{
			rc=pthread_create(&threadpool_p->threads[i],NULL,do_work,threadpool_p );//create thread and send to function
			if(rc!=0)
			{
				perror("error: <sys_call>\n");
				free(threadpool_p->threads);
				free(threadpool_p);
				return NULL;
			}
		}
	return threadpool_p;

}
void dispatch(threadpool* from_me, dispatch_fn dispatch_to_here, void *arg)
{
	if(from_me->dont_accept==1)
	{
		return;
	}
	work_t* tempJob=(work_t*)malloc(sizeof(work_t));
	if(tempJob==NULL)
	{
		perror("error: <sys_call>\n");
		return;	
	}
	pthread_mutex_lock(&(from_me->qlock));//enter job to queue
	tempJob->routine=dispatch_to_here;//enter function
	tempJob->arg=arg;//enter value to the function

	if(from_me->qsize==0)
	{
		from_me->qhead=tempJob;
		from_me->qtail=tempJob;
	}
	else
	{
		from_me->qtail->next=tempJob;
		from_me->qtail=tempJob;
		
		
	}
	from_me->qtail->next=NULL;
	from_me->qsize++;
	pthread_mutex_unlock(&(from_me->qlock));
	pthread_cond_signal(&(from_me->q_empty));//wake up thread after sleep- when queue was empty
}

void* do_work(void* p)
{
	if(p==NULL)
	{
		return NULL;
	}
	threadpool* threadpool_p;
	threadpool_p=(threadpool*)p;
	while(1)
	{
		pthread_mutex_lock(&(threadpool_p->qlock));//lock thread, thread do work
		if(threadpool_p->shutdown==1)
		{
			pthread_mutex_unlock(&(threadpool_p->qlock));//unlock to destroy
			return NULL;	
		}


		while(threadpool_p->qsize==0)
		{	
			if(threadpool_p->shutdown==1)//destruction after wake up
			{
				pthread_mutex_unlock(&(threadpool_p->qlock));
				return NULL;
			}
			pthread_cond_wait(&(threadpool_p->q_empty),&(threadpool_p->qlock)); //queue empty, thread go to "sleep"
		}
		if(threadpool_p->shutdown==1)//destruction after wake up
		{
			pthread_mutex_unlock(&(threadpool_p->qlock));
			return NULL;
		}
		work_t* tempJob=threadpool_p->qhead;//take first job in the queue
		threadpool_p->qhead=threadpool_p->qhead->next;
		(threadpool_p->qsize)-=1;//update queue
		if(tempJob==NULL)
		{
			pthread_mutex_unlock(&(threadpool_p->qlock));
			continue;
		}
		if((threadpool_p->qsize==0) && (threadpool_p->dont_accept==1))
		{
			pthread_mutex_unlock(&(threadpool_p->qlock));
			pthread_cond_signal(&(threadpool_p->q_not_empty));//send signal to destruction process
		}
		pthread_mutex_unlock(&(threadpool_p->qlock));
		tempJob->routine(tempJob->arg);
		free(tempJob);
		tempJob=NULL;
	}
}

void destroy_threadpool(threadpool* destroyme)
{
	if(destroyme==NULL)
	{
		printf("Usage: threadpool <pool-size> <max-number-of-jobs>\n");
		return;
	}
	pthread_mutex_lock(&(destroyme->qlock));//lock threads from use queue
	destroyme->dont_accept=1;//warn threads before destroy
	if(destroyme->qsize!=0)
	{
		pthread_cond_wait(&(destroyme->q_not_empty),&(destroyme->qlock));
	}
	destroyme->shutdown=1;
	pthread_cond_broadcast(&(destroyme->q_empty));
	pthread_mutex_unlock(&(destroyme->qlock));
	int i;
	for(i=0;i<destroyme->num_threads;i++)
	{ 
		pthread_join(destroyme->threads[i],NULL);//wait until the threads finish theirs job
	}
	//free
	pthread_mutex_destroy(&(destroyme->qlock)); 
	pthread_cond_destroy(&(destroyme->q_not_empty));
	pthread_cond_destroy(&(destroyme->q_empty));
	free(destroyme->threads);
	free(destroyme);
	destroyme=NULL;
}
