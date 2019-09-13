#include <pthread.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

static pthread_mutex_t semA;
static pthread_cond_t grpReady = PTHREAD_COND_INITIALIZER;
static int startGroup, startGroupCount, otherGroupCount, otherGroupSignalCount, groupWait, posWait;

static pthread_cond_t posCond[10];
static bool posTaken[10];

struct Request
{
	int id, group, position, arrival, duration;
};


void *access_database(void *req)
{
// First critical section
	pthread_mutex_lock(&semA);
	struct Request *request = (struct Request*) req;
	printf("User %d from Group %d arrives to the DBMS\n", request->id, request->group);
	if (request->group != startGroup && startGroupCount > 0)
	{
		printf("User %d is waiting due to its group\n", request->id);
		groupWait++;
		pthread_cond_wait(&grpReady, &semA);
	}
	pthread_mutex_unlock(&semA);
// Gives chance to access first crit section
	pthread_mutex_lock(&semA);
	// Access the position
	int pos = request->position-1;
	
	if (posTaken[pos]!=0)
	{
		printf("User %d is waiting: position %d of the database is being used by user %d\n",
			request->id, request->position, posTaken[pos]);
		posWait++;
		pthread_cond_wait(&posCond[pos], &semA);
	}
	posTaken[pos] = request->id;
	printf("User %d is accessing the position %d of the database for %d second(s)\n",
		request->id, request->position, request->duration);
	pthread_mutex_unlock(&semA); 

// Sleep the duration the process is accessing the database	
	sleep(request->duration);

// Next critical section
	pthread_mutex_lock(&semA);

	printf("User %d finished its execution\n", request->id);
	posTaken[pos]=0;
	if (posTaken[pos]==0)
	{
		pthread_cond_signal(&posCond[pos]);
	}
	
	if (request->group == startGroup)
		startGroupCount--;
	else
		otherGroupCount++;

	if(startGroupCount == 0)
	{
		if(otherGroupCount == 0)
		{
			printf("\nAll users from Group %d finished their execution\n",
				startGroup);
			printf("The users from Group %d start their execution\n\n",
				startGroup%2+1);
		}
		for (int i = 0; i < 3; i++)
			pthread_cond_signal(&grpReady);
	}

	pthread_mutex_unlock(&semA);
	return NULL;
}

int main()
{
	cin >> startGroup;

	int group, position, arrival, duration;
	int count = 0, groupOneCount=0, groupTwoCount=0;
	pthread_mutex_init(&semA, NULL);
	vector<Request> vect;
// initialize condition variables for position access
	for(int i = 0; i < 10; i++)
	{
		pthread_cond_init(&posCond[i], NULL);
	}
//  Store requests in container called vect
	while (cin >> group && cin >> position && cin >> arrival && cin >> duration)
	{
		Request req { ++count, group, position, arrival, duration };
		if (req.group == 1)
			groupOneCount++;
		else
			groupTwoCount++;
		
		if (req.group == startGroup)
			startGroupCount++;

		vect.push_back(req);
	}
	otherGroupSignalCount = count - startGroupCount;
	pthread_t tid[count];
	for(int i = 0; i < count; i++)
	{
		sleep(vect.at(i).arrival);
		if(pthread_create(&tid[i], NULL, access_database, &vect.at(i))) 
		{
			fprintf(stderr, "Error creating thread\n");
			return 1;
		}
	}
// Wait for the other threads to finish
	for (int i = 0; i < count; i++)
	{
        	pthread_join(tid[i], NULL);
	}
// print summary here
	printf("\nTotal Requests:\n");
	printf("\tGroup 1: %d\n", groupOneCount);
	printf("\tGroup 2: %d\n", groupTwoCount);
	printf("\nRequests that waited:\n");
	printf("\tDue to its group: %d\n", groupWait);
	printf("\tDue to a locked position: %d\n", posWait);
	return 0;
}
