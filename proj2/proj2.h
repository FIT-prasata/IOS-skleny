// Name: proj2.h
// IOS_proj2 solution
// Author: David Sklenář
// Login: xsklen14
// Faculty: FIT
// Compiled: gcc 9.4.0
// Start date: 21.04.2022

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct args{
    int NO;
    int NH;
    int TI;
    int TB;
} args_t;

//Semaphores
sem_t *mutex_sem = NULL;
sem_t *barrier_sem = NULL;
sem_t *oxy_sem = NULL;
sem_t *hydro_sem = NULL;
sem_t *printing_sem = NULL;

//Variables
int *IDO_count, *IDH_count, *molecule_count;

//Shared memory
int shm_IDO, shm_IDH, shm_molecule;

// Function prototypes
void process_args(int argc, const char **argv, args_t *args);
void create_oxygen(int id, args_t *args);
void create_hydrogen(int id, args_t *args);
void cleanup();
bool sem_ctor();
bool sem_dtor();