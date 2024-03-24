#define _POSIX_C_SOURCE 199309L

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>
#include <time.h>
#include <unistd.h>

void msleep(uint64_t mseconds) {
    struct timespec ts = {.tv_sec = (mseconds / 1000),
                          .tv_nsec = (mseconds % 1000) * 1000000};

    nanosleep(&ts, nullptr);
}
uint64_t rand_in_range(uint64_t min, uint64_t max) {
    int64_t r = rand();
    int64_t r_in_range = r % (max - min + 1) + min;
    return r_in_range;
}

typedef struct Fork {
    uint32_t id;
    bool in_use;
} Fork;
Fork fork_new(uint32_t id) {
    Fork f = {.id = id};
    //printf("Fork: id: %d, in_use: %d\n", f.id, f.in_use);
    return f;
}
typedef struct ForksArray {
    Fork *ptr;
    size_t len;
} ForksArray;

typedef struct Philosopher {
    uint32_t id;
    bool lfork;
    bool rfork;

    // test fields
    //
    uint64_t hit;
    uint64_t miss;
    uint64_t consecutive_miss_max;
    uint64_t consecutive_miss_curr;
} Philosopher;

Philosopher philosopher_new(uint32_t id) {
    Philosopher p = {.id = id};
    //printf("Philosopher: id: %d, lfork: %d, rfork: %d\n", p.id, p.lfork,
    //       p.rfork);
    return p;
}
bool philosopher_try_rfork(const Philosopher *philosopher,
                           const ForksArray forks) {
    Fork *rfork = &forks.ptr[(philosopher->id + 1) % forks.len];
    return !rfork->in_use;
}
void philosopher_take_rfork(Philosopher *philosopher, ForksArray forks) {
    Fork *rfork = &forks.ptr[(philosopher->id + 1) % forks.len];
    if (!rfork->in_use) {
        rfork->in_use = true;
        philosopher->rfork = true;
    }
}
void philosopher_release_rfork(Philosopher *philosopher, ForksArray forks) {
    Fork *rfork = &forks.ptr[(philosopher->id + 1) % forks.len];
    rfork->in_use = false;
    philosopher->rfork = false;
}

bool philosopher_try_lfork(const Philosopher *philosopher,
                           const ForksArray forks) {
    Fork *lfork = &forks.ptr[philosopher->id];
    return !lfork->in_use;
}
void philosopher_take_lfork(Philosopher *philosopher, ForksArray forks) {
    Fork *lfork = &forks.ptr[philosopher->id];
    if (!lfork->in_use) {
        lfork->in_use = true;
        philosopher->lfork = true;
    }
}
void philosopher_release_lfork(Philosopher *philosopher, ForksArray forks) {
    Fork *lfork = &forks.ptr[philosopher->id];
    lfork->in_use = false;
    philosopher->lfork = false;
}
void philosopher_think(Philosopher *philosopher, pthread_mutex_t *mutex,
                       uint64_t min_mseconds, uint64_t max_mseconds) {
    pthread_mutex_lock(mutex);

    //printf("Philosopher %d is thinking\n", philosopher->id);
    uint64_t mseconds = rand_in_range(min_mseconds, max_mseconds);

    // thinking time reduction
    //if (philosopher->consecutive_miss_curr > 9) {
    //    mseconds -= (mseconds * 20) / 100;
    //}

    pthread_mutex_unlock(mutex);

    msleep(mseconds);
    // sleep(time);
}
void phisosopher_eat(Philosopher *philosopher, ForksArray forks,
                     pthread_mutex_t *mutex, uint64_t min_mseconds,
                     uint64_t max_mseconds) {

    pthread_mutex_lock(mutex);

    fprintf(stderr, "Before Philosopher %d trying to eat\n", philosopher->id);
    for (size_t i = 0; i < forks.len; i++) {
        fprintf(stderr, "Fork %d is: %d\n", forks.ptr[i].id,
                forks.ptr[i].in_use);
    }
    bool rfork = philosopher_try_rfork(philosopher, forks);
    bool lfork = philosopher_try_lfork(philosopher, forks);
    if (!(lfork && rfork)) {
        fprintf(stderr, "Philosopher %d couldn't eat\n", philosopher->id);
        philosopher->miss += 1;
        philosopher->consecutive_miss_curr += 1;
        if (philosopher->consecutive_miss_max <
            philosopher->consecutive_miss_curr) {
            philosopher->consecutive_miss_max =
                philosopher->consecutive_miss_curr;
        }

        pthread_mutex_unlock(mutex);
        return;
    }
    philosopher_take_rfork(philosopher, forks);
    philosopher_take_lfork(philosopher, forks);

    uint64_t mseconds = rand_in_range(min_mseconds, max_mseconds);

    pthread_mutex_unlock(mutex);

    if (philosopher->lfork && philosopher->rfork) {
        //printf("Philosopher %d is eating\n", philosopher->id);
        philosopher->hit += 1;
        philosopher->consecutive_miss_curr = 0;
        msleep(mseconds);
        // sleep(time);
    }

    pthread_mutex_lock(mutex);

    philosopher_release_lfork(philosopher, forks);
    philosopher_release_rfork(philosopher, forks);
    fprintf(stderr, "Philosopher %d released forks\n", philosopher->id);

    pthread_mutex_unlock(mutex);
}

typedef struct ThreadArgs {
    ForksArray forks;
    Philosopher *philosopher;
    pthread_mutex_t *mutex;
} ThreadArgs;
ThreadArgs thread_args_new(ForksArray forks, Philosopher *philosopher,
                           pthread_mutex_t *mutex) {
    ThreadArgs t = {forks, philosopher, mutex};
    return t;
}

void *thread_run(void *data) {
    ThreadArgs *args = (ThreadArgs *)data;

    Philosopher *philosopher = args->philosopher;
    ForksArray forks = args->forks;
    pthread_mutex_t *mutex = args->mutex;

    //printf("Thread wtith Philosopher %d\n", philosopher->id);
    for (size_t i = 0; i < 1000; i += 1) {
        // time in miliseconds
        uint64_t min = 10;
        uint64_t max = 20;
        philosopher_think(philosopher, mutex, min, max);
        phisosopher_eat(philosopher, forks, mutex, min, max);
    }
    return nullptr;
}

int main() {
    srand(time(nullptr));

    const uint64_t N = 50;

    Philosopher *philosophers = malloc(sizeof(Philosopher) * N);
    if (!philosophers) {
        return 1;
    }
    for (size_t i = 0; i < N; i++) {
        philosophers[i] = philosopher_new(i);
    }

    Fork *forks_ptr = malloc(sizeof(Fork) * N);
    if (!forks_ptr) {
        return 1;
    }
    for (size_t i = 0; i < N; i++) {
        forks_ptr[i] = fork_new(i);
    }
    ForksArray forks = {.ptr = forks_ptr, .len = N};

    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, nullptr);

    ThreadArgs *thread_args = malloc(sizeof(ThreadArgs) * N);
    if (!thread_args) {
        return 1;
    }
    for (size_t i = 0; i < N; i++) {
        thread_args[i] = thread_args_new(forks, &philosophers[i], &mutex);
    }

    pthread_t *handles = malloc(sizeof(pthread_t) * N);
    if (!handles) {
        return 1;
    }
    for (size_t i = 0; i < N; i++) {
        pthread_create(&handles[i], nullptr, thread_run,
                       (void *)&thread_args[i]);
    }
    for (size_t i = 0; i < N; i++) {
        pthread_join(handles[i], nullptr);
    }

    for (size_t i = 0; i < N; i++) {
        fprintf(stderr, "Philosopher %d\n", philosophers[i].id);
        fprintf(stderr, "hits: %ld\n", philosophers[i].hit);
        fprintf(stderr, "misses: %ld\n", philosophers[i].miss);
        fprintf(stderr, "consecutive miss: %ld\n",
                philosophers[i].consecutive_miss_max);
        printf("%ld,", philosophers[i].consecutive_miss_max);
    }

    free(philosophers);
    free(forks.ptr);
    free(handles);
    free(thread_args);
}
