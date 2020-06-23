#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <semaphore.h>

#define R 4
#define P 4

pthread_mutex_t mutex;
sem_t s[4]; //for system resource matrix, initialized to the initial_available_resources

int p[P] = {0,1,2,3};
int initResourceVector[R] = {4,3,5,4};
int available [R];
int allocation[P][R] = {0};
int maximum[P][R] = {
    { 3, 3, 2, 2},
    { 2, 1, 3, 3},
    { 1, 0, 1, 2},
    { 0, 0, 3, 1}
};
int need [P][R];
int orderMat [P];
int orderno=0,i=0,j=0;

int test_safety()
{
	int finish[P] = {0};
	int work[R],k;
	for(i = 0; i < R; i++)
	{
		work[i] = available[i];
	}
	for(i = 0; i < P; i++)
	{
		if (finish[i] == 0)
		{
			for(j = 0; j < R; j++)
			{
				if(need[i][j] <= work[j])
				{
					if(j == R - 1)
					{
						finish[i] = 1;
						for (k = 0; k < R; ++k)
						{
							work[k] += allocation[i][k];
						}
						i = -1;
						break;
					}
					else continue;
				}
				else break;
			}
		}
		else continue;
	}
	for(i = 0; i < P; i++)
	{
		if (finish[i] == 0) return -1;
		else continue;
	}
	return 0;
}

int check_need(int Pi,int requestVector[])
{
	for (i = 0; i < R; ++i)
	{
		if (requestVector[i] <= need[Pi][i]) continue;
		else return -1;
	}
	return 0;
}

int check_allocation(int requestVector[])
{
	for (i = 0; i < R; ++i)
	{
		if (requestVector[i] <= available[i])
			continue;
		else return -1;
	}
	return 0;
}

void printneed()
{
	for (i = 0; i < P; ++i)
	{
		printf("[ ");
		for (j = 0; j < R; ++j)
		{
			printf("%d ", need[i][j]);
		}
		printf("]\n");
	}
	return;
}

void printallocation()
{
	for (i = 0; i < P; ++i)
	{
		printf("[ ");
		for (j = 0; j < R; ++j)
		{
			printf("%d ", allocation[i][j]);
		}
		printf("]\n");
	}
	return;
}

void printAvailable()
{
	for (i = 0; i < R; ++i)
	{
		printf("%d ",available[i]);
	}
	printf("\n");
	return;
}

void printVector(int vec[])
{
	for (i = 0; i < R; ++i)
	{
		printf("%d ",vec[i]);
	}
	printf("\n");
	return;
}

int requestResource(int Pi,int requestVector[])
{
	if (check_need(Pi,requestVector) == -1)
	{
		printf("Requested resources are more than that needed.\n");
		return -1;
	}
	printf("If allocated...\n");

	if(check_allocation(requestVector) == -1)
	{
		printf("No enough resources for this process.\n");
		return -1;
	}
	for (i = 0; i < R; ++i)
	{
		need[Pi][i] -= requestVector[i];
		allocation[Pi][i] += requestVector[i];
		available[i] -= requestVector[i];
	}

	if (test_safety() == 0)
	{
		printf("SAFE\nAvailable resources \n");
		printAvailable();
		printf("Allocated matrix \n");
		printallocation();
		printf("Need matrix \n");
		printneed();
    for(i=0;i<R;i++){
      for(j=0;j<requestVector[i];j++)
        sem_wait(&s[i]);
    }
		return 0;
	}
	else
	{
		printf("It is NOT SAFE. Rolling back.\n");
		for (i = 0; i < R; ++i)
		{
			need[Pi][i] += requestVector[i];
			allocation[Pi][i] -= requestVector[i];
			available[i] += requestVector[i];
		}
		return -1;
	}
}

int releaseResource(int Pi,int releaseVector[],int o)
{
  orderMat[o]=Pi;
  int c=0;
  for(i = 0; i < R; i++)
	{
    if(releaseVector[i]==0){
      c++;
    }
  }
  for(i = 0; i < R; i++)
	{
    for(j=0;j<releaseVector[i];j++)
      sem_post(&s[i]);
		available[i] += releaseVector[i];
    allocation[Pi][i]=0;
	}
	printf("Process %d COMPLETED.\nAvailable resources \n",Pi);
	printAvailable();
	printf("Allocated matrix \n");
	printallocation();
	printf("Need matrix \n");
	printneed();
  printf("Completed Processes:");
  for(i=0;i<=o;i++){
    printf(" %d",orderMat[i]);
  }
  printf("\n");
	return 0;
}

void *customer(void* num)
{
	int Pi = *(int*)num;
  int check=0;
	while(1)
	{
		sleep(1);
		int requestVector[R];
		pthread_mutex_lock(&mutex);
    if(orderno!=P){
		for(i = 0; i < R; i++)
		{
			if(need[Pi][i] != 0)
			{
				requestVector[i] = rand() % (need[Pi][i]+1);
			}
			else
			{
				requestVector[i] = 0;
			}
		}
		printf("\nProcess %d is requesting resources: ",Pi);
		printVector(requestVector);
		requestResource(Pi,requestVector);
		pthread_mutex_unlock(&mutex);
    }
    if(orderno==P){
      pthread_mutex_unlock(&mutex);
      break;
    }
		sleep(1);
    int cr=0;check=0;
		int releaseVector[R];
		pthread_mutex_lock(&mutex);
		for(i = 0; i < R; i++)
		{
      if(need[Pi][i]==0){
        cr++;
      }
		}
    for(j=0;j<orderno;j++){
      if(orderMat[j]==Pi){
        check=1;
        break;
      }
    }
    if(orderno==P){
      pthread_mutex_unlock(&mutex);
      break;
    }
    if(cr==R && check==0){
		    printf("\nProcess %d is releasing resources: ",Pi);
        for(i = 0; i < R; i++)
    		{
          releaseVector[i]=maximum[Pi][i];
        }
		    printVector(releaseVector);
		    releaseResource(Pi,releaseVector,orderno);
        orderno++;
    }
    cr=0;
		pthread_mutex_unlock(&mutex);
	}
}

int main()
{
  sem_init(&s[0],0,4);
  sem_init(&s[1],0,3);
  sem_init(&s[2],0,5);
  sem_init(&s[3],0,4);

	for(i = 0; i < R; i++)
	{
		available[i] = initResourceVector[i];
	}
	for (i = 0; i < P; ++i)
	{
		for (j = 0; j < R; ++j)
		{
			need[i][j] = maximum[i][j] - allocation[i][j];
		}
	}
	printf("Available resources \n");
	printAvailable();
	printf("Allocation matrix \n");
	printallocation();
	printf("Need matrix \n");
	printneed();
	pthread_mutex_init(&mutex,NULL);
  pthread_t thread_id[P];
	for(i = 0; i < P; i++)
	{
		pthread_create(&thread_id[i], NULL, customer, &p[i]);
	}
	for(i = 0; i < P; i++)
	{
		pthread_join(thread_id[i],NULL);
	}
	return 0;
}
