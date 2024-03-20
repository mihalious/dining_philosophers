#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct Philosopher {
    int32_t id;
    bool lfork;
    bool rfork;
} Philosopher;

typedef struct Fork {
    int32_t id;
    bool in_use;
} Fork;

typedef struct ForksArray {
    Fork *ptr;
    size_t len;
} ForksArray;

typedef struct ThreadData {
    Philosopher *philosopher;
    ForksArray forks;
} ThreadArgs;

Philosopher philosopher_new(int32_t id, bool lfork, bool rfork) {
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
                     uint64_t time) {
}
void *philosopher_run(void *data) {
    ThreadArgs *args = (ThreadArgs *)data;
    Philosopher *philosopher = args->philosopher;
    ForksArray forks = args->forks;

    printf("before run with Philosopher  %d\n", philosopher->id);
    while (true) {
        // time in seconds
        uint64_t time = 3;
        philosopher_think(philosopher, time);
        phisosopher_eat(philosopher, forks, time * 2);
    }
}

Fork fork_new(int32_t id, bool in_use) {
    Fork f = {id, in_use};
    printf("Fork: id: %d, in_use: %d\n", f.id, f.in_use);
    return f;
}

int main(int argc, char *argv[]) {
    const uint64_t N = 2;

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

    pthread_t *handles = malloc(sizeof(pthread_t) * N);
    if (!handles) {
        return 1;
    }
    for (size_t i = 0; i < N; i++) {
        printf("Thread Philosopher with id: %d\n", philosophers[i].id);
        ThreadArgs data = {.philosopher = &philosophers[i], .forks = forks};
        printf("thread args philosopher: id: %d\n", data.philosopher->id);
        pthread_create(&handles[i], nullptr, philosopher_run, (void *)&data);
        // philosophers[i] = philosepher_new(i, false, false);
    }
    for (size_t i = 0; i < N; i++) {
        pthread_join(handles[i], nullptr);
    }

    free(philosophers);
    free(forks_ptr);
    free(handles);
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
