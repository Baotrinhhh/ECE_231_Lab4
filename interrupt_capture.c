#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/epoll.h>
#include "pin_config_lib.h"

// Global buffer to store 5 timestamp values
long buffer[5];
long buffernsec[5];

void* threadFunction(void *var) {
    char* input = (char*) var;
    FILE* fp = fopen(input, "r");
    if (fp == NULL) {
        perror("Failed to open interrupt file");
        pthread_exit(NULL);
    }
    int fd = fileno(fp);

    int epfd = epoll_create(1);
    if (epfd == -1) {
        perror("Failed to create epoll instance");
        fclose(fp);
        pthread_exit(NULL);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("Failed to add file descriptor to epoll");
        close(epfd);
        fclose(fp);
        pthread_exit(NULL);
    }

    struct epoll_event ev_wait;
    struct timespec tm;
    int cnt = 0;
    
    while (cnt <5) {
        int ret = epoll_wait(epfd, &ev_wait, 1, -1);
         if (cnt < 0) {
              perror("epoll_wait failed");
              close(epfd);
              fclose(fp);
              pthread_exit(NULL);
         }
         // Capture the current timestamp (only seconds part)
         clock_gettime(CLOCK_MONOTONIC_RAW, &tm);
         buffer[cnt] = tm.tv_sec;
         buffernsec[cnt] = tm.tv_nsec;
         cnt += ret;

         // Optionally reset file pointer if required to clear the interrupt:
         // fseek(fp, 0, SEEK_SET);
    }

    close(epfd);
    fclose(fp);
    pthread_exit(NULL);
}

int main() {
    char gpio_pin_number[32] = "P8_09";
    int gpio_number = 69;

    // Configure GPIO input and interrupt on rising edge
    configure_interrupt(gpio_number, gpio_pin_number);

    // Build the file path for the GPIO value file
    char InterruptPath[40];
    sprintf(InterruptPath, "/sys/class/gpio/gpio%d/value", gpio_number);

    pthread_t thread_id;

    // Create a thread to handle GPIO interrupts
    if (pthread_create(&thread_id, NULL, threadFunction, (void*)InterruptPath) != 0) {
        perror("Failed to create thread");
        return 1;
    }

    // Wait for the thread to finish
    if (pthread_join(thread_id, NULL) != 0) {
        perror("Failed to join thread");
        return 1;
    }

    // Print the collected timestamps from the global buffer
    printf("Timestamps captured:\n");
    for (int i = 0; i < 5; i++) {
         printf("%ld sec, %ld nsec\n", buffer[i], buffernsec[i]);
    }

    return 0;
}

