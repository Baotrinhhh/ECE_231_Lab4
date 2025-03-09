#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "pin_config_lib.h"

void* threadFunction(void *var)

int main(){
    // configure pin P8_8 as input with internal pull-up enabled
    // and enable interrupt for rising edge
    char buffer[5];
    int buffer_size =5;
    char gpio_pin_number[32] = "P8_09";
    int gpio_number = 69;
    configure_interrupt(gpio_number, gpio_pin_number);

    // the following code can be used to receive interrupts on the registered pin
    char InterruptPath[40];
    sprintf(InterruptPath, "/sys/class/gpio/gpio%d/value", gpio_number);

    pthread_t thread_id;
    // Create the thread to handle GPIO interrupt
    if (pthread_create(&thread_id, NULL, threadFunction, (void*)InterruptPath) != 0) {
        perror("Failed to create thread");
        return 1;
    }

    pthread_join(thread_id, NULL);

    printf("Received data=%d inside main from thread\n", data);
    
    // release the thread once finisihed
    pthread_exit(0)
 

    return 0;
}

void* threadFunction(void *var){
    int epfd;
    struct epoll_event ev;

    // (STEP 1) open the interrupt file
    // file pointer (C abstraction to manipulate files)
    char* input = (char*) var;
    FILE* fp = fopen(*input, "r");

    // file descriptor (Unix Linux file identifier used by system calls)
    int fd = fileno(fp);

    // (STEP 2) create epoll instance to monitor I/O events on interrupt file
    epfd = epoll_create(1);

    // (STEP 3) register events that will be monitored
    // detects whenever a new data is available for read (EPOLLIN)
    // signals the read events when the available read value has changed (EPOLLET)
    ev.events = EPOLLIN | EPOLLET;
    // (STEP 4) register interrupt file with epoll interface for monitoring
    ev.data.fd = fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);

    int capture_interrupt;
    struct epoll_event ev_wait;
    struct timespec tm;

    for(int i=0; i < 5; i++){ // Capture interrupt ten times
    // (STEP 5) wait for epoll interface to signal the change
    capture_interrupt = epoll_wait(epfd, &ev_wait, 1, -1);
    clock_gettime(CLOCK_MONOTONIC_RAW, &tm);
    printf("Interrupt received: %d at %ld\n", capture_interrupt, tm.tv_sec);
    }

    // (STEP 6) close the epoll interface
    close(epfd);
    // pause the program for 1 second
    sleep(1);
    printf("Received data=%d inside thread from main\n", *input);
    return NULL;
    }