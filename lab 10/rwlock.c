#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define READER_THREAD_COUNT 10
#define TEXT_BUFFER_SIZE 50

typedef struct {
    pthread_rwlock_t* read_write_lock;
    char* shared_buffer;
    int* operation_counter;
} ThreadSharedData;

void* writer_task(void* thread_argument) {
    ThreadSharedData* thread_data = (ThreadSharedData*)thread_argument;
    
    for (int operation_num = 0; operation_num < 15; operation_num++) {
        pthread_rwlock_wrlock(thread_data->read_write_lock);
        
        thread_data->shared_buffer[0] = '>';
        snprintf(thread_data->shared_buffer + 1, 
                TEXT_BUFFER_SIZE - 1, 
                "Update #%d", 
                (*thread_data->operation_counter)++);
        
        printf("Writer produced: '%s'\n", thread_data->shared_buffer);
        
        pthread_rwlock_unlock(thread_data->read_write_lock);
        
        usleep(200000);
    }
    
    return NULL;
}

void* reader_task(void* thread_argument) {
    ThreadSharedData* thread_data = (ThreadSharedData*)thread_argument;
    pthread_t current_thread_id = pthread_self();
    
    for (int read_attempt = 0; read_attempt < 5; read_attempt++) {
        pthread_rwlock_rdlock(thread_data->read_write_lock);
        
        printf("Reader %lu: content = %s\n", 
               (unsigned long)current_thread_id, 
               thread_data->shared_buffer);
        
        pthread_rwlock_unlock(thread_data->read_write_lock);
        
        usleep(100000 + rand() % 100000);
    }
    
    return NULL;
}

int main() {
    pthread_t writer_task_thread;
    pthread_t reader_task_threads[READER_THREAD_COUNT];
    pthread_rwlock_t read_write_lock;
    
    char* text_buffer = (char*)malloc(TEXT_BUFFER_SIZE * sizeof(char));
    int global_counter = 1;
    
    strncpy(text_buffer, "Initial content", TEXT_BUFFER_SIZE - 1);
    text_buffer[TEXT_BUFFER_SIZE - 1] = '\0';
    
    ThreadSharedData shared_data = {
        .read_write_lock = &read_write_lock,
        .shared_buffer = text_buffer,
        .operation_counter = &global_counter
    };
    
    srand((unsigned int)time(NULL));
    
    if (pthread_rwlock_init(&read_write_lock, NULL) != 0) {
        fprintf(stderr, "Error: Failed to initialize read-write lock\n");
        return EXIT_FAILURE;
    }
    
    if (pthread_create(&writer_task_thread, NULL, writer_task, &shared_data) != 0) {
        fprintf(stderr, "Error: Failed to create writer thread\n");
        return EXIT_FAILURE;
    }
    
    for (int idx = 0; idx < READER_THREAD_COUNT; idx++) {
        if (pthread_create(&reader_task_threads[idx], NULL, reader_task, &shared_data) != 0) {
            fprintf(stderr, "Error: Failed to create reader thread %d\n", idx);
            return EXIT_FAILURE;
        }
    }
    
    pthread_join(writer_task_thread, NULL);
    
    for (int idx = 0; idx < READER_THREAD_COUNT; idx++) {
        pthread_join(reader_task_threads[idx], NULL);
    }
    
    pthread_rwlock_destroy(&read_write_lock);
    
    free(text_buffer);
    
    printf("\nProgram execution completed successfully.\n");
    
    return EXIT_SUCCESS;
}
