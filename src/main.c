#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct Fork {
    uint32_t id;
    bool in_use;
} Fork;
Fork fork_new(uint32_t id, bool in_use) {
    Fork f = {id, in_use};
    printf("Fork: id: %d, in_use: %d\n", f.id, f.in_use);
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
} Philosopher;

Philosopher philosopher_new(uint32_t id, bool lfork, bool rfork) {
    Philosopher p = {id, lfork, rfork};
    printf("Philosopher: id: %d, lfork: %d, rfork: %d\n", p.id, p.lfork,
           p.rfork);
    return p;
}
void philosopher_try_rfork(Philosopher *philosopher, ForksArray forks) {
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
void philosopher_try_lfork(Philosopher *philosopher, ForksArray forks) {
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
void philosopher_think(Philosopher *philosopher, uint64_t time) {
    printf("Philosopher %d is thinking\n", philosopher->id);
    sleep(time);
}
void phisosopher_eat(Philosopher *philosopher, ForksArray forks,
                     pthread_mutex_t *mutex, uint64_t time) {
    pthread_mutex_lock(mutex);

    philosopher_try_rfork(philosopher, forks);
    philosopher_try_lfork(philosopher, forks);

    if (!(philosopher->lfork && philosopher->rfork)) {
        printf("Philosopher %d couldn't eat\n", philosopher->id);

        philosopher_release_lfork(philosopher, forks);
        philosopher_release_rfork(philosopher, forks);

        pthread_mutex_unlock(mutex);
        return;
    }
    pthread_mutex_unlock(mutex);

    if (philosopher->lfork && philosopher->rfork) {
        printf("Philosopher %d is eating\n", philosopher->id);
        sleep(time);
    }

    pthread_mutex_lock(mutex);

    philosopher_release_lfork(philosopher, forks);
    philosopher_release_rfork(philosopher, forks);

    pthread_mutex_unlock(mutex);
}

typedef struct ThreadArgs {
    ForksArray forks;
    Philosopher *philosopher;
    pthread_mutex_t mutex;
} ThreadArgs;
ThreadArgs thread_args_new(ForksArray forks, Philosopher *philosopher,
                           pthread_mutex_t mutex) {
    ThreadArgs t = {forks, philosopher, mutex};
    return t;
}

void *thread_run(void *data) {
    ThreadArgs *args = (ThreadArgs *)data;

    Philosopher *philosopher = args->philosopher;
    ForksArray forks = args->forks;
    pthread_mutex_t mutex = args->mutex;
    printf("Thread for Philosopher with id: %d\n", philosopher->id);

    while (true) {
        // time in seconds
        uint64_t time = 1;
        philosopher_think(philosopher, time);
        phisosopher_eat(philosopher, forks, &mutex, time * 2);
    }
}

int main() {
    const uint64_t N = 5;

    Philosopher *philosophers = malloc(sizeof(Philosopher) * N);
    if (!philosophers) {
        return 1;
    }
    for (size_t i = 0; i < N; i++) {
        philosophers[i] = philosopher_new(i, false, false);
    }

    Fork *forks_ptr = malloc(sizeof(Fork) * N);
    if (!forks_ptr) {
        return 1;
    }
    for (size_t i = 0; i < N; i++) {
        forks_ptr[i] = fork_new(i, false);
    }
    ForksArray forks = {.ptr = forks_ptr, .len = N};

    ThreadArgs *thread_args = malloc(sizeof(ThreadArgs) * N);
    if (!thread_args) {
        return 1;
    }
    for (size_t i = 0; i < N; i++) {
        // thread_args[i].forks = forks;
        // thread_args[i].philosopher = &philosophers[i];
        // pthread_mutex_init(&thread_args[i].mutex, nullptr);
        pthread_mutex_t mutex;
        pthread_mutex_init(&mutex, nullptr);
        thread_args[i] = thread_args_new(forks, &philosophers[i], mutex);
    }

    pthread_t *handles = malloc(sizeof(pthread_t) * N);
    if (!handles) {
        return 1;
    }
    // spawning the treads
    for (size_t i = 0; i < N; i++) {
        pthread_create(&handles[i], nullptr, thread_run,
                       (void *)&thread_args[i]);
    }
    for (size_t i = 0; i < N; i++) {
        pthread_join(handles[i], nullptr);
    }

    free(philosophers);
    free(forks_ptr);
    free(handles);
    free(thread_args);
}
// void philosopher_try_rfork(Philosopher *philosopher, Fork *forks,
//                            size_t forks_len) {
//     Fork *rfork = &forks[(philosopher->id + 1) % forks_len];
//     if (!rfork->in_use) {
//         rfork->in_use = true;
//         philosopher->rfork = true;
//     }
// }
// void philosopher_try_lfork(Philosopher *philosopher, Fork *forks,
//                            size_t forks_len) {
//     Fork *lfork = &forks[philosopher->id];
//     if (!lfork->in_use) {
//         lfork->in_use = true;
//         philosopher->lfork = true;
//     }
// }
// void philosopher_release_lfork(Philosopher *philosopher, Fork *forks,
//                                size_t forks_len) {
//     Fork *lfork = &forks[philosopher->id];
//     lfork->in_use = false;
//     philosopher->lfork = false;
// }
// void philosopher_release_rfork(Philosopher *philosopher, Fork *forks,
//                                size_t forks_len) {
//     Fork *rfork = &forks[(philosopher->id + 1) % forks_len];
//     rfork->in_use = false;
//     philosopher->rfork = false;
// }
