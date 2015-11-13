#define _GNU_SOURCE
#include <pthread.h>
#include <numa.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <fcntl.h>
#include <getopt.h>

// #define NUM_OF_THREADS 31
#define ITERATION 100000000
//-------------separate cache lines
#define CLIENTFLAG0(core) *(client0 + ((core%8) * 128))
#define CLIENTFLAG1(core) *(client1 + ((core%8) * 128))
#define CLIENTFLAG2(core) *(client2 + ((core%8) * 128))
#define CLIENTFLAG3(core) *(client3 + ((core%8) * 128))

//-------------shared between each 2 cores
// #define CLIENTFLAG0(core) *(client0 + ((core/2) * 128) + (core%2))
// #define CLIENTFLAG1(core) *(client1 + (((core%8)/2) * 128) + (core%2))
// #define CLIENTFLAG2(core) *(client2 + (((core%8)/2) * 128) + (core%2))
// #define CLIENTFLAG3(core) *(client3 + (((core%8)/2) * 128) + (core%2))

//-------------shared between each 4 cores
// #define CLIENTFLAG0(core) *(client0 + ((core/4) * 128) + (core%4))
// #define CLIENTFLAG1(core) *(client1 + (((core%8)/4) * 128) + (core%4))
// #define CLIENTFLAG2(core) *(client2 + (((core%8)/4) * 128) + (core%4))
// #define CLIENTFLAG3(core) *(client3 + (((core%8)/4) * 128) + (core%4))

//-------------shared between each 8 cores
// #define CLIENTFLAG0(core) *(client0 + (core%8))
// #define CLIENTFLAG1(core) *(client1 + (core%8))
// #define CLIENTFLAG2(core) *(client2 + (core%8))
// #define CLIENTFLAG3(core) *(client3 + (core%8))

// #define SERVERFLAG(core) *(server+ (core*128))
#define SERVERFLAG(core) *(server+ (core/8)*128 + (core%8))
// #define SERVERFLAG(core) *(server+ (core/4)*128 + (core%4))

char *server; // 64 * 32 * 2
char *client0; // 64 * 8 * 2
char *client1;
char *client2;
char *client3;

volatile int finished;
int num_of_threads;
int num_of_cpuid;
struct timespec t_start, t_end;
pthread_barrier_t barrier_start;
pthread_barrier_t safe_id;
int server_id;

static inline void cpuid(int a, int b, int c, int d){
	__asm__ __volatile__ ("cpuid"
                            : "=a" (a),
                            "=b" (b),
                            "=c" (c),
                            "=d" (d)
                            : "" (a), "2" (c)
                            :"memory");
}

void* increment(void * id){
	int i;
	int a, b, c, d;
	a = 0;
	b = 0;
	int j;
	int _id = (int)*(int*)id;
	int local_client_flag;

	int num_cpu = numa_num_configured_cpus();	
	struct bitmask * cpumask = numa_bitmask_alloc(num_cpu);
	numa_bitmask_setbit(cpumask, _id);
	numa_sched_setaffinity(0, cpumask);

	pthread_barrier_wait(&safe_id);

	pthread_barrier_wait(&barrier_start);

	local_client_flag = SERVERFLAG(_id);

	switch(_id/8){
		case 0:
			for(i = 0; i < ITERATION; i++){
				local_client_flag = !local_client_flag;
				CLIENTFLAG0(_id) = local_client_flag;
				__asm__ __volatile__("rep;nop": : :"memory");

				while(local_client_flag != SERVERFLAG(_id)){
					for (j = 0 ; j < num_of_cpuid; j++){
						cpuid(a, b, c, d);
					}
					__asm__ __volatile__("rep;nop": : :"memory");
				}
			}
		break;
		case 1:
			for(i = 0; i < ITERATION; i++){
				local_client_flag = !local_client_flag;
				CLIENTFLAG1(_id) = local_client_flag;
				__asm__ __volatile__("rep;nop": : :"memory");

				while(local_client_flag != SERVERFLAG(_id)){
					for (j = 0 ; j < num_of_cpuid; j++){
						cpuid(a, b, c, d);
					}

					__asm__ __volatile__("rep;nop": : :"memory");
				}
			}
		break;
		case 2:
			for(i = 0; i < ITERATION; i++){
				local_client_flag = !local_client_flag;
				CLIENTFLAG2(_id) = local_client_flag;
				__asm__ __volatile__("rep;nop": : :"memory");

				while(local_client_flag != SERVERFLAG(_id)){
					for (j = 0 ; j < num_of_cpuid; j++){
						cpuid(a, b, c, d);
					}

					__asm__ __volatile__("rep;nop": : :"memory");
				}
			}
		break;
		case 3:
			for(i = 0; i < ITERATION; i++){
				local_client_flag = !local_client_flag;
				CLIENTFLAG3(_id) = local_client_flag;
				__asm__ __volatile__("rep;nop": : :"memory");

				while(local_client_flag != SERVERFLAG(_id)){
					for (j = 0 ; j < num_of_cpuid; j++){
						cpuid(a, b, c, d);
					}

					__asm__ __volatile__("rep;nop": : :"memory");
				}
			}
		break;
	}
	
	return 0;
}

void* hyperthread(void* input){

	while(finished != 1){
		SERVERFLAG(0) = CLIENTFLAG0(0);
		SERVERFLAG(1) = CLIENTFLAG0(1);
		SERVERFLAG(2) = CLIENTFLAG0(2);
		SERVERFLAG(3) = CLIENTFLAG0(3);
		SERVERFLAG(4) = CLIENTFLAG0(4);
		SERVERFLAG(5) = CLIENTFLAG0(5);
		SERVERFLAG(6) = CLIENTFLAG0(6);
		SERVERFLAG(7) = CLIENTFLAG0(7);
		SERVERFLAG(8) = CLIENTFLAG1(8);
		SERVERFLAG(9) = CLIENTFLAG1(9);
		SERVERFLAG(10) = CLIENTFLAG1(10);
		SERVERFLAG(11) = CLIENTFLAG1(11);
		SERVERFLAG(12) = CLIENTFLAG1(12);
		SERVERFLAG(13) = CLIENTFLAG1(13);
		SERVERFLAG(14) = CLIENTFLAG1(14);
		SERVERFLAG(15) = CLIENTFLAG1(15);
		SERVERFLAG(16) = CLIENTFLAG2(16);
		SERVERFLAG(17) = CLIENTFLAG2(17);
		SERVERFLAG(18) = CLIENTFLAG2(18);
		SERVERFLAG(19) = CLIENTFLAG2(19);
		SERVERFLAG(20) = CLIENTFLAG2(20);
		SERVERFLAG(21) = CLIENTFLAG2(21);
		SERVERFLAG(22) = CLIENTFLAG2(22);
		SERVERFLAG(23) = CLIENTFLAG2(23);
		SERVERFLAG(24) = CLIENTFLAG3(24);
		SERVERFLAG(25) = CLIENTFLAG3(25);
		SERVERFLAG(26) = CLIENTFLAG3(26);
		SERVERFLAG(27) = CLIENTFLAG3(27);
		SERVERFLAG(28) = CLIENTFLAG3(28);
		SERVERFLAG(29) = CLIENTFLAG3(29);
		SERVERFLAG(30) = CLIENTFLAG3(30);
		// SERVERFLAG(31) = CLIENTFLAG3(31);

		__asm__ __volatile__("rep;nop": : :"memory");
	}

	return 0;
}

pthread_t * create_pinned_thread(int cpu, void *(* func) (void *), int i){
	// cpu_set_t cpuset;
	pthread_t * thread = malloc(sizeof(pthread_t));
	// CPU_ZERO(&cpuset);
	// CPU_SET(cpu, &cpuset);
	// pthread_attr_t * attr = malloc(sizeof(pthread_attr_t));
	// printf("2\n");
	// pthread_attr_setaffinity_np(attr, sizeof(cpuset), &cpuset);
	// printf("3\n");
	pthread_create(thread, 0, func, &i);
	// printf("4\n");
	
	return thread;
}

int main (int argc, char ** argv){
	
	if(numa_available() < 0){
        printf("System does not support NUMA API!\n");
    }

	int c;
	while((c = getopt(argc, argv, "t:c:")) != -1){
		switch (c)
		{
			case 't':
			{
				num_of_threads = atoi(optarg);
				break;
			}
			case 'c':
			{
				num_of_cpuid = atoi(optarg);
				break;
			}
		}
	}
	
	// numa_set_strict(0);
	server = (char*)numa_alloc_onnode(4096, 3);
	
	// numa_set_strict(0);
	client0 = (char*)numa_alloc_onnode(1024, 0);
	// numa_set_strict(1);
	client1 = (char*)numa_alloc_onnode(1024, 1);
	// numa_set_strict(2);
	client2 = (char*)numa_alloc_onnode(1024, 2);
	// numa_set_strict(3);
	client3 = (char*)numa_alloc_onnode(1024, 3);

	int i;
	pthread_t * t[31];
	pthread_t * ht;
	pthread_barrier_init(&safe_id, 0, 2);
	pthread_barrier_init(&barrier_start, 0, num_of_threads+1);

    ht = create_pinned_thread(31, hyperthread, 0);

    // for (i = 0; i < num_of_threads; i++){
    for (i = 30; i >= 31-num_of_threads; i--){
		t[i] = create_pinned_thread(i, increment, i);
		pthread_barrier_wait(&safe_id);
	}

	pthread_barrier_wait(&barrier_start);
	clock_gettime(CLOCK_MONOTONIC, &t_start);

	// for (i = 0; i < num_of_threads; i++){
	for (i = 30; i >= 31-num_of_threads; i--){
		pthread_join(*t[i], 0);
	}

	finished = 1;
	pthread_join(*ht, 0);
	clock_gettime(CLOCK_MONOTONIC, &t_end);		
	
	uint64_t start = (t_start.tv_sec * 1000000000LL) + t_start.tv_nsec;
    uint64_t finish = (t_end.tv_sec * 1000000000LL) + t_end.tv_nsec;
    uint64_t duration = finish - start;
    printf("%.3f\n", (double)(duration)/ITERATION);

    numa_free(server, 4096);
    numa_free(client0, 1024);
    numa_free(client1, 1024);
    numa_free(client2, 1024);
    numa_free(client3, 1024);

	return 0;
}

