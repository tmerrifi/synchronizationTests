
#define _GNU_SOURCE

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <sched.h>

#ifndef SERVER_CPU_ID
#define SERVER_CPU_ID 0
#endif

#ifndef CLIENT_CPU_ID
#define CLIENT_CPU_ID 2
#endif

volatile int flag;

void move_to_cpu(pthread_t id, int cpu){
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpu, &set);
    pthread_setaffinity_np(id, sizeof(cpu_set_t), &set);
}

void * server(void * ptr){
    move_to_cpu(pthread_self(), SERVER_CPU_ID);
                
    while(1){
        flag=1;
        __asm__ __volatile__("mfence": : :"memory");
    }
}

void * client(void * ptr){
    move_to_cpu(pthread_self(), CLIENT_CPU_ID);
    
    while(1){
        while(flag!=1){
            __asm__ __volatile__("rep; nop;");
        }
        flag=0;
        __asm__ __volatile__("mfence": : :"memory");
    }

}

int main(int argc, char ** argv){

    pthread_t server_thread, client_thread;

    pthread_create(&server_thread, NULL, server, NULL);

    pthread_create(&client_thread, NULL, client, NULL);

    sleep(atoi(argv[1]));

    pthread_kill(server_thread, 9);

    pthread_kill(client_thread, 9);
    
}
