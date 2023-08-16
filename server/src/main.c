#include "unix_thread.h"
#include "inet_thread.h"
#include "utils.h"
#include "logger.h"
#include "async_tasks.h"
#include "messages_queue.h"

#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>


void stop_server(int signal) {
    log_info("Stopping server...");
    STOP_SERVER = 1;
}


void do_initialization() {
    signal(SIGINT, stop_server);
    init_thread_pool();
    initialize_log_file("server_log.log");
    initialize_messages_queue();
}


void do_cleanup() {
    signal(SIGINT, SIG_DFL);
    destroy_thread_pool();
    close_log_file();
    destroy_messages_queue();
}


int main() {
    pthread_t ptUnix;
    pthread_t ptInet;

    do_initialization();
    int res = pthread_create(&ptUnix, (pthread_attr_t*)NULL, (void * (*)(void *))unix_thread, (void*)NULL);
    if(res != 0) {
        log_error("Could not create unix thread. Error: %s", strerror(errno));
        return -1;
    }
    
    res = pthread_create(&ptInet, (pthread_attr_t*)NULL, (void * (*)(void *))inet_thread, (void*)NULL);
    if(res != 0) {
        log_error("Could not create inet thread. Error: %s", strerror(errno));

        STOP_SERVER = 1;
        pthread_join(ptUnix, (void**)NULL);
        do_cleanup();
        return -1;
    }

    pthread_join(ptUnix, (void**)NULL);
    pthread_join(ptInet, (void**)NULL);
    do_cleanup();
    return 0;
}