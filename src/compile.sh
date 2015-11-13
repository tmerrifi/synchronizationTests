#v1
gcc -g -std=c99 -O0 -o v1 v1.c -lpthread
#numa alloc	
gcc -g -O3 -o numa_alloc numa_alloc.c -lpthread -lnuma
