// Name: proj2.c
// IOS_proj2 solution
// Author: David Sklenář
// Login: xsklen14
// Faculty: FIT
// Compiled: gcc 9.4.0
// Start date: 21.04.2022

#include "proj2.h"


//remove before submitting!!
void print_semaphore_values(){
    int mutex_sem_val;
    int barrier_turn_sem_val;
    int barrier_turn_sem2_val;
    int oxy_sem_val;
    int hydro_sem_val;
    int barrier_mut_sem_val;
    int printing_sem_val;
    sem_getvalue(mutex_sem, &mutex_sem_val);
    sem_getvalue(barrier_turn_sem, &barrier_turn_sem_val);
    sem_getvalue(barrier_turn2_sem, &barrier_turn_sem2_val);
    sem_getvalue(oxy_sem, &oxy_sem_val);
    sem_getvalue(hydro_sem, &hydro_sem_val);
    sem_getvalue(barrier_mut_sem, &barrier_mut_sem_val);
    sem_getvalue(printing_sem, &printing_sem_val);
    printf("\n\tmutex_sem: %i\n", mutex_sem_val);
    printf("\tbarrier_turn_sem: %i\n", barrier_turn_sem_val);
    printf("\tbarrier_turn2_sem: %i\n", barrier_turn_sem2_val);
    printf("\toxy_sem: %i\n", oxy_sem_val);
    printf("\thydro_sem: %i\n", hydro_sem_val);
    printf("\tbarrier_mut_sem: %i\n", barrier_mut_sem_val);
    printf("\tprinting_sem: %i\n\n", printing_sem_val);
}


void cnt_max_molecules(args_t args){
    if (args.NO > (args.NH / 2)) {
        *max_molecules = args.NH / 2;
    }
    else {
        *max_molecules = args.NO;
    }
}


void process_args(int argc, const char **argv, args_t *args){
    char *tmp = "";

    if(argc != 5){
        fprintf(stderr, "ERROR: Wrong number of command line arguments.\nw");
        exit(1);
    }

    if(((args->NO = strtol(argv[1], &tmp, 10)) <= 0) && (!strcmp(tmp, ""))){
        fprintf(stderr, "ERROR: Wrong number of oxygens (must be integer greater than zero).\n");
        exit(1);
    }
    if(((args->NH = strtol(argv[2], &tmp, 10)) <= 0) && (!strcmp(tmp, ""))){
        fprintf(stderr, "ERROR: Wrong number of hydrogens (must be integer greater than zero).\n");
        exit(1);
    }
    if(((args->TI = strtol(argv[3], &tmp, 10)) < 0 || args->TI > 1000) && (!strcmp(tmp, ""))){
        fprintf(stderr, "ERROR: Wrong maximal waiting time (must be in miliseconds, 0 <= TIME <= 1000\n");
        exit(1);
    }
    if(((args->TB = strtol(argv[4], &tmp, 10)) < 0 || args->TB > 1000) && (!strcmp(tmp, ""))){
        fprintf(stderr, "ERROR: Wrong maximal creating time (must be in miliseconds, 0 <= TIME <= 1000\n");
        exit(1);
    }

}

//creates semaphores
bool sem_ctor(){
    //mapping memory blocks for semaphores
    if(
        (mutex_sem = mmap(NULL, sizeof(sem_t), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED || \
        (barrier_turn_sem = mmap(NULL, sizeof(sem_t), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED || \
        (barrier_turn2_sem = mmap(NULL, sizeof(sem_t), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED || \
        (oxy_sem = mmap(NULL, sizeof(sem_t), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED || \
        (hydro_sem = mmap(NULL, sizeof(sem_t), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED || \
        (barrier_mut_sem = mmap(NULL, sizeof(sem_t), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED || \
        (printing_sem = mmap(NULL, sizeof(sem_t), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED \
    ){
        fprintf(stderr, "ERROR: Problem with semaphores mapping.\n");
        return false;
    }

    //initializing semaphores
    if(
        sem_init(mutex_sem, 1, 1) == -1 || \
        sem_init(barrier_turn_sem, 1, 0) == -1 || \
        sem_init(barrier_turn2_sem, 1, 1) == -1 || \
        sem_init(oxy_sem, 1, 0) == -1 || \
        sem_init(hydro_sem, 1, 0) == -1 || \
        sem_init(barrier_mut_sem, 1, 1) == -1 || \
        sem_init(printing_sem, 1, 1) == -1
    ){
        fprintf(stderr, "ERROR: Problem with semaphores initialization.\n");
        return false;
    }
    //everything went smoothly
    return true;

}

//destroys all semaphores
bool sem_dtor(){
    //destroying of semahores
    if(
        sem_destroy(mutex_sem) == -1 || \
        sem_destroy(barrier_turn_sem) == -1 || \
        sem_destroy(barrier_turn2_sem) == -1 || \
        sem_destroy(oxy_sem) == -1 || \
        sem_destroy(hydro_sem) == -1 || \
        sem_destroy(barrier_mut_sem) == -1 || \
        sem_destroy(printing_sem) == -1
    ){
        fprintf(stderr, "ERROR: Problem with destroing semaphores.\n");
        return false;
    }
    return true;
}

//creates shared memory
bool shm_ctor(){
    if(
        (shm_IDO = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1 || \
        (shm_IDH = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1 || \
        (shm_count = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1 || \
        (shm_barrier_count = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1 || \
        (shm_max_molecules = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1 || \
        (shm_molecule = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1
    ){
        fprintf(stderr, "ERROR: Problem with allocation of shared memory.\n");
        return false;
    }

    if(
        (IDO_count = shmat(shm_IDO, NULL, 0)) == NULL || \
        (IDH_count = shmat(shm_IDH, NULL, 0)) == NULL || \
        (count = shmat(shm_count, NULL, 0)) == NULL || \
        (barrier_count = shmat(shm_barrier_count, NULL, 0)) == NULL || \
        (max_molecules = shmat(shm_max_molecules, NULL, 0)) == NULL || \
        (molecule_count = shmat(shm_molecule, NULL, 0)) == NULL
    ){
        fprintf(stderr, "ERROR: Problem with attachment of shared memory.\n");
        return false;
    }
    return true;
}

//destroys shared memory
bool shm_dtor(){
    //destroy memory block
    if(
        shmctl(shm_IDO, IPC_RMID, NULL) == -1 || \
        shmctl(shm_IDH, IPC_RMID, NULL) == -1 || \
        shmctl(shm_count, IPC_RMID, NULL) == -1 || \
        shmctl(shm_barrier_count, IPC_RMID, NULL) == -1 || \
        shmctl(shm_max_molecules, IPC_RMID, NULL) == -1 || \
        shmctl(shm_molecule, IPC_RMID, NULL) == -1
    ){
        fprintf(stderr, "ERROR: Problem with freeing shared memory.\n");
        return false;
    }

    //detach memory block
    if(
        shmdt(IDO_count) == -1 || \
        shmdt(IDH_count) == -1 || \
        shmdt(count) == -1 || \
        shmdt(barrier_count) == -1 || \
        shmdt(max_molecules) == -1 || \
        shmdt(molecule_count) == -1
    ){
        fprintf(stderr, "ERROR: Problem with detaching shared memory.\n");
        return false;
    }

    return true;
}


//destroys everything
void cleanup(){
    if(sem_dtor() == false){
          exit(1); 
    }
    if(shm_dtor() == false){
        exit(1);
    }
}


void create_oxygen(int id, args_t *args, FILE *file){
    srand(getpid());

    sem_wait(printing_sem);
    (*count)++;
    fprintf(stdout, "%i: O %i: started\n", *count, id);
    sem_post(printing_sem);

    usleep(1000 * (rand() % (args->TI + 1)));

    sem_wait(printing_sem);
    (*count)++;
    fprintf(stdout, "%i: O %i: going to queque\n", *count, id);
    sem_post(printing_sem);

    //creating molecules UNTESTED
    sem_wait(mutex_sem);

    (*IDO_count)++;

    if(*IDH_count >= 2){
//        printf("inside start O id: %i\n", id);
        sem_post(hydro_sem);
        sem_post(hydro_sem);
        (*IDH_count)--;
        (*IDH_count)--;
        sem_post(oxy_sem);
        (*IDO_count)--;

        //sem_wait(printing_sem);
    //print_semaphore_values();
    //sem_post(printing_sem); 

    }
    else{
        sem_post(mutex_sem);
    }
//    printf("before sem_wait oxy sem id: %i\n", id);
    sem_wait(oxy_sem);

    sem_wait(printing_sem);
    (*count)++;
    fprintf(stdout, "%i: O %i: creating molecule %i\n", *count, id, (*molecule_count)+1);
    sem_post(printing_sem);

//    sem_wait(printing_sem);
//    print_semaphore_values();
//    sem_post(printing_sem);

    usleep(1000 * (rand() % (args->TB + 1)));

    //create molecule

    //insert barrier here
    if(*molecule_count != *max_molecules){

        //barrier - implementation based on the book The Little book of semaphores
        sem_wait(barrier_mut_sem);
        (*barrier_count)++;
        if(*barrier_count == 3){
            sem_wait(barrier_turn2_sem);
            sem_post(barrier_turn_sem);
        }
        sem_post(barrier_mut_sem);

    //    printf("oxy %i waits here.\n", id);
        sem_wait(barrier_turn_sem);
    //    printf("oxy %i waits heredfdfdfdf.\n", id);
        sem_post(barrier_turn_sem);

        sem_wait(printing_sem);
        (*count)++;
        fprintf(stdout, "%i: O %i: molecule %i created\n", *count, id, (*molecule_count)+1);
        sem_post(printing_sem);

        sem_wait(barrier_mut_sem);
        (*barrier_count)--;
        if(*barrier_count == 0){
            sem_wait(barrier_turn_sem);
            sem_post(barrier_turn2_sem);
        }
        sem_post(barrier_mut_sem);

        sem_wait(barrier_turn2_sem);
        sem_post(barrier_turn2_sem);
    //    printf("after barrier O id: %i\n", id);


        (*molecule_count)++;
    }

    sem_post(mutex_sem);
    
}

void create_hydrogen(int id, args_t *args, FILE *file){
    srand(getpid());

    sem_wait(printing_sem);
    (*count)++;
    fprintf(stdout, "%i: H %i: started\n", *count, id);
    sem_post(printing_sem);

    usleep(1000 * (rand() % (args->TI + 1)));

    sem_wait(printing_sem);
    (*count)++;
    fprintf(stdout, "%i: H %i: going to queque\n", *count, id);
    sem_post(printing_sem);

    
    sem_wait(mutex_sem);

    (*IDH_count)++;

//    printf("close call xd H id: %i\n", id);
    if(*IDH_count >= 2 && *IDO_count >= 1){
//        printf("inside start H id: %i\n", id);
        sem_post(hydro_sem);
        sem_post(hydro_sem);
        (*IDH_count)--;
        (*IDH_count)--;
        sem_post(oxy_sem);
        (*IDO_count)--;

    }
    else{
        sem_post(mutex_sem);
    }
//    printf("before sem_wait hydro sem id: %i\n", id);
    sem_wait(hydro_sem);

    sem_wait(printing_sem);
    (*count)++;
    fprintf(stdout, "%i: H %i: creating molecule %i\n", *count, id,(*molecule_count)+1);
    sem_post(printing_sem);

//    sem_wait(printing_sem);
//    print_semaphore_values();
//    sem_post(printing_sem);
    //bonding

    if(*molecule_count != *max_molecules){
    
        //barrier - implementation based on the book The Little book of semaphores
        sem_wait(barrier_mut_sem);
        (*barrier_count)++;
        if(*barrier_count == 3){
            sem_wait(barrier_turn2_sem);
            sem_post(barrier_turn_sem);
        }
        sem_post(barrier_mut_sem);

    //    printf("hydro %i waits here.\n", id);
        sem_wait(barrier_turn_sem);
    //    printf("hydro %i waits heredfdfdfdf.\n", id);
        sem_post(barrier_turn_sem);

        sem_wait(printing_sem);
        (*count)++;
        fprintf(stdout, "%i: H %i: molecule %i created\n", *count, id, (*molecule_count+1));
        sem_post(printing_sem);

        sem_wait(barrier_mut_sem);
        (*barrier_count)--;
        if(*barrier_count == 0){
            sem_wait(barrier_turn_sem);
            sem_post(barrier_turn2_sem);
        }
        sem_post(barrier_mut_sem);

        sem_wait(barrier_turn2_sem);
        sem_post(barrier_turn2_sem);

    }
//    printf("after barrier H id: %i\n", id);
}






int main(int argc, char const *argv[]){
    args_t args;

    process_args(argc, argv, &args);

    FILE *f;
    pid_t init, oxygen_init, hydrogen_init;
    pid_t oxygen_queque[args.NO], hydrogen_queque[args.NH];

    //file problem handling
    if ((f = fopen("proj2.out", "w")) == NULL){
        fprintf(stderr, "ERROR: Error when opening file.\n");
        exit(1);
    }
    setbuf(f, NULL);

    sem_ctor(); //problemky s tim když se to posere tak se musí uvolnit asi jen něco, budu se holt modlit ať se to nikdy neposere xd 
    shm_ctor(); //same problémek jako nahoře

    *molecule_count = 0;
    *IDH_count = 0;
    *IDO_count = 0;
    *count = 0;
    *barrier_count = 0;

    cnt_max_molecules(args);

    //initial fork
    init = fork();
    if(init == -1){
        fprintf(stderr, "ERROR: Error at initial fork.\n");
        fclose(f);
        cleanup();
        exit(1);
    }
    else if(init == 0){
        //oxygen
        for(int i = 0; i < args.NO; i++){
            oxygen_init = fork();
            if(oxygen_init == -1){
                fprintf(stderr, "ERROR: Error at oxygen fork.\n");
                fclose(f);
                cleanup();
                exit(1);
            }
            else if(oxygen_init == 0){
                create_oxygen(i + 1, &args, f);
                exit(0);
            }
            else{
                oxygen_queque[i] = oxygen_init;
            }
        }
        //wait for oxygen childs
        for(int i = 0; i < args.NO; i++){
            waitpid(oxygen_queque[i], NULL, 0);
        }
    }
    else{
        //hydrogen + waiting to end of all processes??
        for(int i = 0; i < args.NH; i++){
            hydrogen_init = fork();
            if(hydrogen_init == -1){
                fprintf(stderr, "ERROR: Error at hydrogen fork.\n");
                fclose(f);
                cleanup();
                exit(1);
            }
            else if(hydrogen_init == 0){
                create_hydrogen(i + 1, &args, f);
                exit(0);
            }
            else{
                hydrogen_queque[i] = hydrogen_init;
            }
        }

        //wait for hydrogen childs
        for(int i = 0; i < args.NH; i++){
            waitpid(hydrogen_queque[i], NULL, 0);
        }
    }



    fclose(f);
    cleanup();
    return 0;
}




