#include<bits/stdc++.h>
#include<semaphore.h>
#include<pthread.h>
#include<unistd.h>

using namespace std;

#define N 5
int phil_num[N]={0,1,2,3,4};
#define LEFT (ph_num+4)%N
#define RIGHT (ph_num+1)%N

class Monitor{
public:
  enum {THINKING, HUNGRY, EATING} state[5];
  void pick_up(int);
  void put_down(int);
  void test(int);
  void initialization_code();
private:
  sem_t mutex;
  sem_t S[N];
}obj;

void Monitor::pick_up(int ph_num)
{
  sem_wait(&mutex);
  state[ph_num] = HUNGRY;
  printf("\nPhilosopher %d is Hungry",ph_num+1);
  test(ph_num);
  sem_post(&mutex);
  sem_wait(&S[ph_num]);
  sleep(1);
}

void Monitor::test(int ph_num)
{
  if (state[ph_num] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING)
  {
      state[ph_num] = EATING;
      sleep(2);
      printf("\nPhilosopher %d takes fork %d and fork %d",ph_num+1,LEFT+1,ph_num+1);
      printf("\nPhilosopher %d is Eating",ph_num+1);
      sem_post(&S[ph_num]);
  }
}

void Monitor::put_down(int ph_num)
{
      sem_wait(&mutex);
      state[ph_num] = THINKING;
      printf("\nPhilosopher %d is putting fork %d and fork %d down",ph_num+1,LEFT+1,ph_num+1);
      printf("\nPhilosopher %d is thinking",ph_num+1);
      test(LEFT);
      test(RIGHT);
      sem_post(&mutex);
}

void Monitor::initialization_code(){
  int i;
  sem_init(&mutex,0,1);
  for(i=0;i<N;i++)
      sem_init(&S[i],0,0);
  for(i=0;i<N;i++)
	state[i]=THINKING;
}

void* philospher(void *num)
{
    while(1)
    {
        int i = *(int *)num;
        sleep(1);
        obj.pick_up(i);
        sleep(0);
        obj.put_down(i);
    }
}

int main()
{
    int i;
    obj.initialization_code();
    pthread_t thread_id[N];
    for(i=0;i<N;i++)
    {
        pthread_create(&thread_id[i],NULL,philospher,&phil_num[i]);
        printf("Philosopher %d is thinking\n",i+1);
    }
    for(i=0;i<N;i++)
        pthread_join(thread_id[i],NULL);
    return 0;
}
