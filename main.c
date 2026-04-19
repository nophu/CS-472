#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define PHILOSOPHER_NUM 5
#define  MAX_MEALS 10
#define MAX_THINK_EAT_SEC 4

// states
enum {THINKING, HUNGRY, EATING} state[PHILOSOPHER_NUM];

// track the meals
int meals[PHILOSOPHER_NUM];

// flag to stop all threads when time is up
int running = 1;

// for each philosopher exists one thread and one condition
pthread_mutex_t mutex;
pthread_cond_t cond[PHILOSOPHER_NUM];
pthread_t philosophers[PHILOSOPHER_NUM];

// check if philosopher i can eat
void test(int i) {

    // check left & right neighbors if they are eating
    if ( (state[ (i+4) % PHILOSOPHER_NUM] != EATING) && (state[i] == HUNGRY) && (state[ (i+1) % PHILOSOPHER_NUM] != EATING) ) {

        // update state of philosopher & let the philosopher eat
        state[i] = EATING;
        pthread_cond_signal(&cond[i]);
    }
}

// philosopher i wants to eat
void pickup(int i) {

    pthread_mutex_lock(&mutex);

    // update state of philosopher & check if that philosopher can eat
    state[i] = HUNGRY;
    test(i);

    // philosopher cannot eat due to waiting on neighbor(s)
    if (state[i] != EATING) { pthread_cond_wait(&cond[i], &mutex); }

    pthread_mutex_unlock(&mutex);
}

// philosopher i is done eating
void putdown(int i) {

    pthread_mutex_lock(&mutex);

    // update state, check if both left & right neighbors are eating
    state[i] = THINKING;
    test( (i + 4) % PHILOSOPHER_NUM );
    test( (i + 1) % PHILOSOPHER_NUM );

    pthread_mutex_unlock(&mutex);
}

// thread function for each philosopher
void *philo_run(void *param) {

    int i = *(int *) param;

    // stop when the max meals is reached or time limit is up
    while (running && meals[i] < MAX_MEALS) {
        printf("Philosopher %d is thinking\n ", i);
        sleep(rand() % MAX_THINK_EAT_SEC + 1); // random amount of time ranging from 1 to MAX_THINK_EAT_SEC

        // pick up chopsticks to eat
        pickup(i);

        printf("Philosopher %d is eating %d\n", i, meals[i] + 1);
        sleep(rand() % MAX_THINK_EAT_SEC + 1); // random amount of time ranging from 1 to MAX_THINK_EAT_SEC

        // update the meal count
        meals[i]++;

        // put down the chopsticks
        putdown(i);
    }

    printf("Philosopher %d is done eating %d meals\n", i, meals[i]);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("%s <run_time>\n", argv[0]);
        return 1;
    }
    int run_time = atoi(argv[1]);

    // seed random number generator
    srand(time(NULL));

    // initialize mutex & condition variables
    pthread_mutex_init(&mutex, NULL);
    for (int i = 0; i < PHILOSOPHER_NUM; i++) {
        pthread_cond_init(&cond[i], NULL);
        state[i] = THINKING;
        meals[i] = 0;
    }

    // create philosopher threads
    int ids[PHILOSOPHER_NUM];
    for (int i = 0; i < PHILOSOPHER_NUM; i++) {
        ids[i] = i;
        pthread_create(&philosophers[i], NULL, philo_run, &ids[i]);
    }

    // run for run_time amount of seconds
    sleep(run_time);
    running = 0;

    // wait for ALL threads to finish
    for (int i = 0; i < PHILOSOPHER_NUM; i++) { pthread_join(philosophers[i], NULL); }

    // find min & max
    int min = meals[0], max = meals[0], total = 0;
    for (int i = 0; i < PHILOSOPHER_NUM; i++) {
        if (meals[i] < min) { min = meals[i]; }
        if (meals[i] > max) { max = meals[i]; }
        total += meals[i];
        printf("Philosopher %d ate %d meals\n", i, meals[i]);
    }

    printf("Minimum number of meals is %d\n", min);
    printf("Maximum number of meals is %d\n", max);
    printf("Average number of meals is %.2f\n", (float)total / PHILOSOPHER_NUM );

    // cleanup process = destroy threads
    pthread_mutex_destroy(&mutex);
    for (int i = 0; i < PHILOSOPHER_NUM; i++) { pthread_cond_destroy(&cond[i]); }
    return 0;
}