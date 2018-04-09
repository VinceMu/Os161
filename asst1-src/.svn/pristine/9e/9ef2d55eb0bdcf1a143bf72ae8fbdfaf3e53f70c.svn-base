	/* This file will contain your solution. Modify it as you wish. */
	#include <types.h>
	#include "producerconsumer_driver.h"
	#include <synch.h>  
	

	/* Declare any variables you need here to keep track of and
	   synchronise your bounded. A sample declaration of a buffer is shown
	   below. You can change this if you choose another implementation. */
	   
	static struct pc_data myData[BUFFER_SIZE];


	//typedef int semaphore;
	static struct semaphore *useQueue, *emptyCount, *fullCount;
	
	int firstGlobalIndex;
	int lastGlobalIndex; 

	/* consumer_receive() is called by a consumer to request more data. It
	   should block on a sync primitive if no data is available in your
	   buffer. */

	struct pc_data consumer_receive(void)
	{
		struct pc_data thedata;
			 
		P(fullCount);
		P(useQueue);
			

		thedata = myData[firstGlobalIndex];
		firstGlobalIndex = (firstGlobalIndex + 1)%BUFFER_SIZE;	
		
		V(useQueue);
		V(emptyCount);
		return thedata;		
	}


	/* procucer_send() is called by a producer to store data in your
	   bounded buffer. */

	void producer_send(struct pc_data item)
	{
		P(emptyCount);
		P(useQueue);
		
		
		myData[lastGlobalIndex] = item;
		lastGlobalIndex = (lastGlobalIndex + 1)%BUFFER_SIZE;
		
		
		V(useQueue);
		V(fullCount);
	}




	/* Perform any initialisation (e.g. of global data) you need
	   here. Note: You can panic if any allocation fails during setup */

	void producerconsumer_startup(void)
	{
		fullCount = sem_create("fullCount", 0);
		useQueue = sem_create("useQueue", 1);
		emptyCount = sem_create("emptyCount", BUFFER_SIZE);

	}

	/* Perform any clean-up you need here */
	void producerconsumer_shutdown(void)
	{
		sem_destroy(useQueue);
		sem_destroy(fullCount);
		sem_destroy(emptyCount);

	}

