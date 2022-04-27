// Name: proj2.c
// IOS_proj2 solution
// Author: David Sklenář
// Login: xsklen14
// Faculty: FIT
// Compiled: gcc 9.4.0
// Start date: 21.04.2022

#include "proj2.h"

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
        (barrier_sem = mmap(NULL, sizeof(sem_t), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED || \
        (oxy_sem = mmap(NULL, sizeof(sem_t), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED || \
        (hydro_sem = mmap(NULL, sizeof(sem_t), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED || \
        (printing_sem = mmap(NULL, sizeof(sem_t), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED \
    ){
        fprintf(stderr, "ERROR: Problem with semaphores mapping.\n");
        return false;
    }

    //initializing semaphores
    if(
        sem_init(mutex_sem, 1, 1) == -1 || \
        sem_init(barrier_sem, 1, 0) == -1 || \
        sem_init(oxy_sem, 1, 0) == -1 || \
        sem_init(hydro_sem, 1, 0) == -1 || \
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
        sem_destroy(barrier_sem) == -1 || \
        sem_destroy(oxy_sem) == -1 || \
        sem_destroy(hydro_sem) == -1 || \
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
        (shm_molecule = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1
    ){
        fprintf(stderr, "ERROR: Problem with allocation of shared memory.\n");
        return false;
    }

    if(
        (IDO_count = shmat(shm_IDO, NULL, 0)) == NULL || \
        (IDH_count = shmat(shm_IDH, NULL, 0)) == NULL || \
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
        shmctl(shm_molecule, IPC_RMID, NULL) == -1
    ){
        fprintf(stderr, "ERROR: Problem with freeing shared memory.\n");
        return false;
    }

    //detach memory block
    if(
        shmdt(IDO_count) == -1 || \
        shmdt(IDH_count) == -1 || \
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


void create_oxygen(int id, args_t *args){
    
}

void create_hydrogen(int id, args_t *args){

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
                //create_oxygen();
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
                //create_hydrogen();
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
