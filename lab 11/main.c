#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define READER_COUNT 10
#define TEXT_BUFFER_CAPACITY 50

typedef struct {
    pthread_mutex_t access_lock;
    pthread_cond_t notification;
    char text_buffer[TEXT_BUFFER_CAPACITY];
    int operation_id;
    int new_data_available;
    int readers_completed;
} SharedResource;

void* writer_routine(void* argument) {
    SharedResource* resource = (SharedResource*)argument;
    
    for (int iteration = 0; iteration < 15; iteration++) {
        pthread_mutex_lock(&resource->access_lock);
        
        snprintf(resource->text_buffer, TEXT_BUFFER_CAPACITY, 
                "Data update #%d", ++resource->operation_id);
        
        resource->new_data_available = 1;
        
        printf("Writer generated: '%s'\n", resource->text_buffer);
        
        pthread_cond_broadcast(&resource->notification);
        
        while (resource->readers_completed < READER_COUNT && 
               resource->new_data_available) {
            pthread_cond_wait(&resource->notification, &resource->access_lock);
        }
        
        resource->readers_completed = 0;
        resource->new_data_available = 0;
        
        pthread_mutex_unlock(&resource->access_lock);
        
        usleep(200000);
    }
    
    return NULL;
}

void* reader_routine(void* argument) {
    SharedResource* resource = (SharedResource*)argument;
    pthread_t current_thread = pthread_self();
    
    for (int read_cycle = 0; read_cycle < 5; read_cycle++) {
        pthread_mutex_lock(&resource->access_lock);
        
        while (!resource->new_data_available) {
            pthread_cond_wait(&resource->notification, &resource->access_lock);
        }
        
        printf("Reader %lu: received = %s\n", 
               (unsigned long)current_thread, resource->text_buffer);
        
        resource->readers_completed++;
        
        if (resource->readers_completed == READER_COUNT) {
            pthread_cond_signal(&resource->notification);
        }
        
        pthread_mutex_unlock(&resource->access_lock);
        
        usleep(100000 + rand() % 100000);
    }
    
    return NULL;
}

int main() {
    pthread_t writer;
    pthread_t readers[READER_COUNT];
    SharedResource shared_resource;
    
    shared_resource.operation_id = 0;
    shared_resource.new_data_available = 0;
    shared_resource.readers_completed = 0;
    
    strncpy(shared_resource.text_buffer, "Initial content", 
            TEXT_BUFFER_CAPACITY - 1);
    shared_resource.text_buffer[TEXT_BUFFER_CAPACITY - 1] = '\0';
    
    srand((unsigned int)time(NULL));
    
    pthread_mutex_init(&shared_resource.access_lock, NULL);
    pthread_cond_init(&shared_resource.notification, NULL);
    
    if (pthread_create(&writer, NULL, writer_routine, &shared_resource) != 0) {
        fprintf(stderr, "Failed to initialize writer thread\n");
        return EXIT_FAILURE;
    }
    
    for (int idx = 0; idx < READER_COUNT; idx++) {
        if (pthread_create(&readers[idx], NULL, reader_routine, &shared_resource) != 0) {
            fprintf(stderr, "Failed to initialize reader thread %d\n", idx);
            return EXIT_FAILURE;
        }
    }
    
    pthread_join(writer, NULL);
    
    for (int idx = 0; idx < READER_COUNT; idx++) {
        pthread_join(readers[idx], NULL);
    }
    
    pthread_mutex_destroy(&shared_resource.access_lock);
    pthread_cond_destroy(&shared_resource.notification);
    
    printf("\nProgram execution terminated successfully.\n");
    
    return EXIT_SUCCESS;
}
