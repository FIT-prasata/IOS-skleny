// Name: proj2.c
// IOS_proj2 solution
// Author: David Sklenář
// Login: xsklen14
// Faculty: FIT
// Compiled: gcc 9.4.0
// Start date: 21.04.2022

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[]){
    char *strtl_tmp = "";
    long int oxygens, hydrogens, maxt_queque, maxt_create;

    //command line arguments validation
    if(argc != 5){
        fprintf(stderr, "ERROR: Wrong number of command line arguments.\nw");
        return 1;
    }
    if(((oxygens = strtol(argv[1], &strtl_tmp, 10)) <= 0) && (!strcmp(strtl_tmp, ""))){
        fprintf(stderr, "ERROR: Wrong number of oxygens (must be integer greater than zero).\n");
        return 1;
    }
    if(((hydrogens = strtol(argv[2], &strtl_tmp, 10)) <= 0) && (!strcmp(strtl_tmp, ""))){
        fprintf(stderr, "ERROR: Wrong number of hydrogens (must be integer greater than zero).\n");
        return 1;
    }
    if(((maxt_queque = strtol(argv[3], &strtl_tmp, 10)) < 0 || maxt_queque > 1000) && (!strcmp(strtl_tmp, ""))){
        fprintf(stderr, "ERROR: Wrong maximal waiting time (must be in miliseconds, 0 <= TIME <= 1000\n");
        return 1;
    }
    if(((maxt_create = strtol(argv[4], &strtl_tmp, 10)) < 0 || maxt_create > 1000) && (!strcmp(strtl_tmp, ""))){
        fprintf(stderr, "ERROR: Wrong maximal creating time (must be in miliseconds, 0 <= TIME <= 1000\n");
        return 1;
    }

    return 0;
}
