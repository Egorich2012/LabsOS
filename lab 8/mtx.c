#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define READER_COUNT 10
#define BUFFER_CAPACITY 256

typedef struct {
    char message_buffer[BUFFER_CAPACITY];
    int write_counter;
    pthread_mutex_t buffer_mutex;
} SharedBuffer;

typedef struct {
    int reader_id;
    SharedBuffer *buffer_ptr;
} ReaderParameters;

void* reader_function(void* thread_arg) {
    ReaderParameters* params = (ReaderParameters*)thread_arg;
    SharedBuffer* buffer = params->buffer_ptr;
    
    while (1) {
        pthread_mutex_lock(&buffer->buffer_mutex);
        
        printf("Reader %d (Thread ID: %lu): %s (Total writes: %d)\n", 
               params->reader_id, (unsigned long)pthread_self(), 
               buffer->message_buffer, buffer->write_counter);
        
        pthread_mutex_unlock(&buffer->buffer_mutex);
        
        usleep(100000 + (rand() % 100000));
    }
    
    return NULL;
}

void* writer_function(void* thread_arg) {
    SharedBuffer* buffer = (SharedBuffer*)thread_arg;
    
    while (1) {
        pthread_mutex_lock(&buffer->buffer_mutex);
        
        buffer->write_counter++;
        snprintf(buffer->message_buffer, BUFFER_CAPACITY, 
                "Write operation #%d performed by writer (Thread ID: %lu)", 
                buffer->write_counter, (unsigned long)pthread_self());
        
        printf("Writer (Thread ID: %lu) completed: %s\n", 
               (unsigned long)pthread_self(), buffer->message_buffer);
        
        pthread_mutex_unlock(&buffer->buffer_mutex);
        
        usleep(500000);
    }
    
    return NULL;
}

int main() {
    pthread_t reader_threads[READER_COUNT];
    pthread_t writer_thread;
    ReaderParameters reader_params[READER_COUNT];
    
    SharedBuffer shared_buffer;
    memset(shared_buffer.message_buffer, 0, BUFFER_CAPACITY);
    shared_buffer.write_counter = 0;
    
    if (pthread_mutex_init(&shared_buffer.buffer_mutex, NULL) != 0) {
        fprintf(stderr, "Error: Failed to initialize mutex\n");
        return EXIT_FAILURE;
    }
    
    srand((unsigned int)time(NULL));
    
    printf("Starting multi-threaded program\n");
    printf("Configuration: %d reader threads, 1 writer thread\n\n", READER_COUNT);
    
    for (int i = 0; i < READER_COUNT; i++) {
        reader_params[i].reader_id = i + 1;
        reader_params[i].buffer_ptr = &shared_buffer;
        
        if (pthread_create(&reader_threads[i], NULL, reader_function, &reader_params[i]) != 0) {
            fprintf(stderr, "Error: Failed to create reader thread %d\n", i + 1);
            return EXIT_FAILURE;
        }
        printf("Reader thread %d created successfully\n", i + 1);
    }
    
    if (pthread_create(&writer_thread, NULL, writer_function, &shared_buffer) != 0) {
        fprintf(stderr, "Error: Failed to create writer thread\n");
        return EXIT_FAILURE;
    }
    printf("Writer thread created successfully\n\n");
    
    printf("All threads are now running...\n");
    printf("Press Ctrl+C to terminate the program\n\n");
    
    for (int i = 0; i < READER_COUNT; i++) {
        pthread_join(reader_threads[i], NULL);
    }
    pthread_join(writer_thread, NULL);
    
    pthread_mutex_destroy(&shared_buffer.buffer_mutex);
    
    return EXIT_SUCCESS;
}
