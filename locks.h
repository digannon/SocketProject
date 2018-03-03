#include	<arpa/inet.h>
#include	<unistd.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include        <pthread.h>

#define CLOCK 0
#define PENDING 1
#define BLOCK 2
#define MINER 3
#define IDX 4
#define WORKER 5
#define MAX_MS   11 // 10 miners + 1 Manager

int locks[WORKER + MAX_MS/2-1];

void init_locks(){
    for(int i = 0; i < WORKER + MAX_MS/2-1; i++)
        locks[i] = 0;
}

void aquire_lock(int idx){
    while(locks[idx] == 1){}
    locks[idx] = 1;
}

void release_lock(int idx){
    locks[idx] = 0;
}


