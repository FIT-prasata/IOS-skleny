// Name: proj2.c
// IOS_proj2 solution
// Author: David Sklenář
// Login: xsklen14
// Faculty: FIT
// Compiled: gcc 9.4.0
// Start date: 21.04.2022

#include "proj2.h"


//counts maximum number of createble molecules and number of atoms that will be left behind
void cnt_max_molecules(args_t args){
    if (args.NO > (args.NH / 2)) {
        *max_molecules = args.NH / 2;
    }
    else {
        *max_molecules = args.NO;
    }
    *IDO_left = args.NO - *max_molecules;
    *IDH_left = args.NH - (*max_molecules * 2);
}

//checks if arguments are correct
void process_args(int argc, const char **argv, args_t *args){
    char *tmp = "";

    if(argc != 5){
        fprintf(stderr, "ERROR: Wrong number of command line arguments.\n");
        exit(1);
    }

    if((args->NO = strtoul(argv[1], &tmp, 10)) <= 0){
        fprintf(stderr, "ERROR: Wrong number of oxygens (must be integer greater than zero).\n");
        exit(1);
    }
    if(strcmp(tmp, "")){
        fprintf(stderr, "ERROR: Wrong number of oxygens (must be integer greater than zero).\n");
        exit(1);
    }

    if((args->NH = strtoul(argv[2], &tmp, 10)) <= 0){
        fprintf(stderr, "ERROR: Wrong number of hydrogens (must be integer greater than zero).\n");
        exit(1);
    }
    if(strcmp(tmp, "")){
        fprintf(stderr, "ERROR: Wrong number of hydrogens (must be integer greater than zero).\n");
        exit(1);
    }

    if((args->TI = strtoul(argv[3], &tmp, 10)) < 0 || args->TI > 1000){
        fprintf(stderr, "ERROR: Wrong maximal waiting time (must be in miliseconds, 0 <= TIME <= 1000)\n");
        exit(1);
    }
    if(strcmp(tmp, "")){
        fprintf(stderr, "ERROR: Wrong maximal waiting time (must be in miliseconds, 0 <= TIME <= 1000)\n");
        exit(1);
    }

    if((args->TB = strtoul(argv[4], &tmp, 10)) < 0 || args->TB > 1000){
        fprintf(stderr, "ERROR: Wrong maximal creating time (must be in miliseconds, 0 <= TIME <= 1000)\n");
        exit(1);
    }
    if(strcmp(tmp, "")){
        fprintf(stderr, "ERROR: Wrong maximal creating time (must be in miliseconds, 0 <= TIME <= 1000)\n");
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
        (some_sem = mmap(NULL, sizeof(sem_t), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED || \
        (mcounter_sem = mmap(NULL, sizeof(sem_t), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED || \
        (printing_sem = mmap(NULL, sizeof(sem_t), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED \
    ){
        fprintf(stderr, "ERROR: Problem with semaphores mapping.\n");
        return false;
    }

    //initializing semaphores
    if(
        sem_init(mutex_sem, 1, 1) == -1 || //main mutex
        sem_init(barrier_turn_sem, 1, 0) == -1 || //barrier semaphore
        sem_init(barrier_turn2_sem, 1, 1) == -1 || //barrier semaphore
        sem_init(oxy_sem, 1, 0) == -1 || //semaphore for oxygen que
        sem_init(hydro_sem, 1, 0) == -1 || //semaphore for hydrogen que
        sem_init(barrier_mut_sem, 1, 1) == -1 || //barrier semaphore
        sem_init(some_sem, 1, 0) == -1 || //temp
        sem_init(mcounter_sem, 1, 0) == -1 || //semaphore for sychronizing output "molecule created"
        sem_init(printing_sem, 1, 1) == -1 //semahore to protect printing
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
        sem_destroy(mutex_sem) == -1 || 
        sem_destroy(barrier_turn_sem) == -1 ||  
        sem_destroy(barrier_turn2_sem) == -1 ||  
        sem_destroy(oxy_sem) == -1 || 
        sem_destroy(hydro_sem) == -1 || 
        sem_destroy(barrier_mut_sem) == -1 ||  
        sem_destroy(some_sem) == -1 ||  
        sem_destroy(mcounter_sem) == -1 ||  
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
        (shm_IDO_left = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1 || \
        (shm_IDH_left = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1 || \
        (shm_count = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1 || \
        (shm_barrier_count = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1 || \
        (shm_max_molecules = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1 || \
        (shm_all_created = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1 || \
        (shm_molecule = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1
    ){
        fprintf(stderr, "ERROR: Problem with allocation of shared memory.\n");
        return false;
    }

    if(
        (IDO_count = shmat(shm_IDO, NULL, 0)) == NULL || \
        (IDH_count = shmat(shm_IDH, NULL, 0)) == NULL || \
        (IDO_left = shmat(shm_IDO_left, NULL, 0)) == NULL || \
        (IDH_left = shmat(shm_IDH_left, NULL, 0)) == NULL || \
        (count = shmat(shm_count, NULL, 0)) == NULL || \
        (barrier_count = shmat(shm_barrier_count, NULL, 0)) == NULL || \
        (max_molecules = shmat(shm_max_molecules, NULL, 0)) == NULL || \
        (all_created = shmat(shm_all_created, NULL, 0)) == NULL || \
        (molecule_count = shmat(shm_molecule, NULL, 0)) == NULL
    ){
        fprintf(stderr, "ERROR: Problem with attachment of shared memory.\n");
        return false;
    }
    //everything went smoothly
    return true;
}

//destroys shared memory
bool shm_dtor(){
    //destroy memory block
    if(
        shmctl(shm_IDO, IPC_RMID, NULL) == -1 || \
        shmctl(shm_IDH, IPC_RMID, NULL) == -1 || \
        shmctl(shm_IDO_left, IPC_RMID, NULL) == -1 || \
        shmctl(shm_IDH_left, IPC_RMID, NULL) == -1 || \
        shmctl(shm_count, IPC_RMID, NULL) == -1 || \
        shmctl(shm_barrier_count, IPC_RMID, NULL) == -1 || \
        shmctl(shm_max_molecules, IPC_RMID, NULL) == -1 || \
        shmctl(shm_all_created, IPC_RMID, NULL) == -1 || \
        shmctl(shm_molecule, IPC_RMID, NULL) == -1
    ){
        fprintf(stderr, "ERROR: Problem with freeing shared memory.\n");
        return false;
    }

    //detach memory block
    if(
        shmdt(IDO_count) == -1 || \
        shmdt(IDH_count) == -1 || \
        shmdt(IDO_left) == -1 || \
        shmdt(IDH_left) == -1 || \
        shmdt(count) == -1 || \
        shmdt(barrier_count) == -1 || \
        shmdt(max_molecules) == -1 || \
        shmdt(all_created) == -1 || \
        shmdt(molecule_count) == -1
    ){
        fprintf(stderr, "ERROR: Problem with detaching shared memory.\n");
        return false;
    }
    //every memory was freed and detached
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

// Function for oxygen process
void create_oxygen(int id, args_t *args, FILE *file){
    srand(getpid());
    //oxygen atom starting
    sem_wait(printing_sem);
    (*count)++;
    fprintf(file, "%i: O %i: started\n", *count, id);
    sem_post(printing_sem);

    //waiting time simulating creating of oxygen atom
    
    usleep(1000 * (rand() % (args->TI + 1)));

    //oxygen atom is created and goes to que
    sem_wait(printing_sem);
    (*count)++;
    fprintf(file, "%i: O %i: going to queue\n", *count, id);
    sem_post(printing_sem);


    //Start of implementation based on solution from The Little Book of Semaphores

    //mutex to make protect oxygen from hydrogen processes and vice versa
    sem_wait(mutex_sem);
    //incrementing count of oxygens
    (*IDO_count)++;

    //signaling that we can create molecule and removing 2 hydrogens and 1 oxygen from que
    if(*IDH_count >= 2 && *IDO_count >= 1){
        sem_post(hydro_sem);
        sem_post(hydro_sem);
        (*IDH_count)-=2;
        sem_post(oxy_sem);
        (*IDO_count)--;
    }
    else{
        sem_post(mutex_sem);
    }
    sem_post(some_sem);

    //here oxygens waits until there are enough resources to create complete molecule
    sem_wait(oxy_sem);

    //check if all molecules were created
    sem_wait(some_sem);
    if(*all_created == 1){
        //message that oxygen couldn't create molecule and had to be released
        sem_wait(printing_sem);
        (*count)++;
        fprintf(file, "%i: O %i: not enough H\n", *count, id);
        sem_post(printing_sem);

        //succesful termination of oxygen process
        exit(0);
    }
    sem_post(some_sem);

    //message that oxygen starts creating molecule
    sem_wait(printing_sem);
    (*count)++;
    fprintf(file, "%i: O %i: creating molecule %i\n", *count, id, (*molecule_count)+1);
    sem_post(printing_sem);

    //waiting time simulating creating of H2O molecule
    
    usleep(1000 * (rand() % (args->TB + 1)));


    //Reusable Barrier - implementation based on the book The Little Book of Semaphores

    //barrier mutex
    sem_wait(barrier_mut_sem);
    (*barrier_count)++;
    if(*barrier_count == 3){
        sem_wait(barrier_turn2_sem);
        sem_post(barrier_turn_sem);
    }
    sem_post(barrier_mut_sem);

    //here oxygen wait for 2 hydrogens
    sem_wait(barrier_turn_sem);
    sem_post(barrier_turn_sem);

    //incrementing count of molecules
    (*molecule_count)++;
    sem_post(mcounter_sem);
    sem_post(mcounter_sem);
    //message that molecule has been created
    sem_wait(printing_sem);
    (*count)++;
    fprintf(file, "%i: O %i: molecule %i created\n", *count, id, *molecule_count);
    sem_post(printing_sem);

    //clearing barrier count back to 0
    sem_wait(barrier_mut_sem);
    (*barrier_count)--;
    if(*barrier_count == 0){
        sem_wait(barrier_turn_sem);
        sem_post(barrier_turn2_sem);
    }
    sem_post(barrier_mut_sem);

    //oxygen wait for reset of barrier count
    sem_wait(barrier_turn2_sem);
    sem_post(barrier_turn2_sem);
    
    //End of barrier


    sem_wait(some_sem);
    //check if any other molecules can be created
    if(*molecule_count == *max_molecules){
        //prevents creating more molecules
        sem_wait(printing_sem);
        *all_created = 1;
        sem_post(printing_sem);
        //releasing of any remaining oxygen processes
        for(int i = 0; i < *IDO_left; i++){
            sem_post(oxy_sem);
        }
        //releasing of any remaining hydrogen processes
        for(int i = 0; i < *IDH_left; i++){
            sem_post(hydro_sem);
        }
    }
    sem_post(some_sem);
    
    //releasing of shared mutex
    sem_post(mutex_sem);
    
}

//Function for hydrogen process
void create_hydrogen(int id, args_t *args, FILE *file){
    srand(getpid());
    //hydrogen atom starting
    sem_wait(printing_sem);
    (*count)++;
    fprintf(file, "%i: H %i: started\n", *count, id);
    sem_post(printing_sem);

    //waiting time simulating creating of hydrogen atom
    
    usleep(1000 * (rand() % (args->TI + 1)));

    //oxygen atom is created and goes to que
    sem_wait(printing_sem);
    (*count)++;
    fprintf(file, "%i: H %i: going to queue\n", *count, id);
    sem_post(printing_sem);

    
    //Start of implementation based on solution from The Little Book of Semaphores

    //mutex to make protect hydrogen from oxygen processes and vice versa
    sem_wait(mutex_sem);
    //incrementing count of hydrogens
    (*IDH_count)++;

    //signaling that we can create molecule and removing 2 hydrogens and 1 oxygen from que
    if(*IDH_count >= 2 && *IDO_count >= 1){
        sem_post(hydro_sem);
        sem_post(hydro_sem);
        (*IDH_count)-=2;
        sem_post(oxy_sem);
        (*IDO_count)--;
    }
    else{
        sem_post(mutex_sem);
    }
    sem_post(some_sem);


    //here hydrogens waits until there are enough resources to create complete molecule
    sem_wait(hydro_sem);
    
    sem_wait(some_sem);
    //check if all molecules were created
    if(*all_created == 1){
        //message that hydrogen couldn't create molecule and had to be released
        sem_wait(printing_sem);
        (*count)++;
        fprintf(file, "%i: H %i: not enough O or H\n", *count, id);
        sem_post(printing_sem);

        //succesful termination of hydrogen process
        exit(0);
    }
    sem_post(some_sem);
    
    //message that hydrogen starts creating molecule
    sem_wait(printing_sem);
    (*count)++;
    fprintf(file, "%i: H %i: creating molecule %i\n", *count, id,(*molecule_count)+1);
    sem_post(printing_sem);
    
    
    //Reusable Barrier - implementation based on the book The Little Book of Semaphores

    //barrier mutex
    sem_wait(barrier_mut_sem);
    (*barrier_count)++;
    if(*barrier_count == 3){
        sem_wait(barrier_turn2_sem);
        sem_post(barrier_turn_sem);
    }
    sem_post(barrier_mut_sem);

    //here hydrogens wait for oxygen
    sem_wait(barrier_turn_sem);
    sem_post(barrier_turn_sem);

    sem_wait(mcounter_sem);
    //message that molecule has been created
    sem_wait(printing_sem);
    (*count)++;
    fprintf(file, "%i: H %i: molecule %i created\n", *count, id, *molecule_count);
    sem_post(printing_sem);

    //clearing barrier count back to 0
    sem_wait(barrier_mut_sem);
    (*barrier_count)--;
    if(*barrier_count == 0){
        sem_wait(barrier_turn_sem);
        sem_post(barrier_turn2_sem);
    }
    sem_post(barrier_mut_sem);

    //hydrogen wait for reset of barrier count
    sem_wait(barrier_turn2_sem);
    sem_post(barrier_turn2_sem);

    //End of barrier

}






int main(int argc, char const *argv[]){
    args_t args;

    process_args(argc, argv, &args);

    FILE *f;
    pid_t oxygen_init, hydrogen_init;

    //file problem handling
    if ((f = fopen("proj2.out", "w")) == NULL){
        fprintf(stderr, "ERROR: Error when opening file.\n");
        exit(1);
    }
    setbuf(f, NULL);

    sem_ctor(); 
    shm_ctor();

    //initialization of shared variables
    *molecule_count = 0;
    *IDH_count = 0;
    *IDO_count = 0;
    *count = 0;
    *barrier_count = 0;
    *all_created = 0;

    //count number of creatable molecules
    cnt_max_molecules(args);

    if(*max_molecules == 0){
        *all_created = 1;
        for(int i = 0; i < *IDO_left; i++){
            sem_post(oxy_sem);
        }
        //releasing of any remaining hydrogen processes
        for(int i = 0; i < *IDH_left; i++){
            sem_post(hydro_sem);
        }
    }


    
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
    }
    
    //hydrogen
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
    }
    

    while(wait(NULL) > 0);
    //close file and cleanup
    fclose(f);
    cleanup();
    exit(0);
}




