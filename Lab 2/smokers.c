// Noah krill
// OS Lab2
//This project is free use except for cheating purposes

#include <stdio.h> 
#include <pthread.h> 
#include <semaphore.h> 
#include <unistd.h> 
#include <time.h> 
#include <stdlib.h>
#include <stdbool.h> 
#include <errno.h>

sem_t agentSem;
sem_t smokerSems[3];
sem_t pusherSems[3];
sem_t mutex;
char* smokerTypes[3] = { "matches and tobacco", "matches and paper", "tobacco and paper"};
bool currentItems[3] = { false, false, false };

// Keeps track of which 
void* smoker(void* arg)
{
	int smokerId = *(int*) arg;
	int typeId = smokerId % 3;
	
	// iterates through smokers and outputs their status
	for (int i = 0; i < 3; ++i)
	{
		printf("Smoker %d is waiting for %s\n", smokerId, smokerTypes[typeId]);
		sem_wait(&smokerSems[typeId]);
		printf("Smoker %d is making a cigarette and smoking\n", smokerId);
		sem_post(&agentSem);
	}
	
	return NULL;
}

// A pusher that signals what materials are needed
void* pusher(void* arg)
{
	int pusherId = *(int*) arg;
	
	// iterates through all threads 
	for (int i = 0; i < 12; ++i)
	{
		sem_wait(&pusherSems[pusherId]);
		sem_wait(&mutex);
		if (currentItems[(pusherId + 1) % 3])
		{
			currentItems[(pusherId + 1) % 3] = false;
			sem_post(&smokerSems[(pusherId + 2) % 3]);
		}
		else if (currentItems[(pusherId + 2) % 3])
		{
			currentItems[(pusherId + 2) % 3] = false;
			sem_post(&smokerSems[(pusherId + 1) % 3]);
		}
		else
		{
			currentItems[pusherId] = true;
		}
		sem_post(&mutex);
	}
	
	return NULL;
}

// simulates giving out different items
void* agent(void* arg)
{
	int agentId = *(int*) arg;
	for (int i = 0; i < 6; ++i)
	{
		// waits randomly between 0 and 200 milliseconds
		usleep(rand() % 200000);
		sem_wait(&agentSem);
		sem_post(&pusherSems[agentId]);
		sem_post(&pusherSems[(agentId + 1) % 3]);
		printf("Agent %d is giving out %s\n", agentId, smokerTypes[(agentId + 2) % 3]);
	}
	return NULL;
}

int main()
{
	int smokersIds[6], pusherIds[3], agentIds[3];
	pthread_t smokerThreads[6], pusherThreads[3], agentThreads[3];
	
	// seeds random
	srand(time(NULL));
	
	// initializes agent semaphore and and mutex
	sem_init(&agentSem, 0, 1);
	sem_init(&mutex, 0, 1);

	// initializes smokers semaphores
	for (int i = 0; i < 3; ++i)
	{
		sem_init(&smokerSems[i], 0, 0);
		sem_init(&pusherSems[i], 0, 0);
	}
	
	// creates threads for different smokers
	for (int i = 0; i < 6; ++i)
	{
		smokersIds[i] = i;
		if (pthread_create(&smokerThreads[i], NULL, smoker, &smokersIds[i]) == EAGAIN)
		{
			perror("Failed to create thread");
			return 0;
		}
	}
	
	// creates threads for pushers
	for (int i = 0; i < 3; ++i)
	{
		pusherIds[i] = i;
		if (pthread_create(&pusherThreads[i], NULL, pusher, &pusherIds[i]) == EAGAIN)
		{
			perror("Failed to create thread");
			return 0;
		}
	}
	
	// creates threads for agents
	for (int i = 0; i < 3; ++i)
	{
		agentIds[i] =i;
		if (pthread_create(&agentThreads[i], NULL, agent, &agentIds[i]) == EAGAIN)
		{
			perror("Failed to create thread");
		}
	}
	
	// joins all threads at the end of operation
	for (int i = 0; i < 6; ++i)
	{
		pthread_join(smokerThreads[i], NULL);
	}
	
	return 0;
}