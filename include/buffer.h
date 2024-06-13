typedef struct {
    char* buffer;
    size_t in;  // Next write position
    size_t out; // Next read position
    size_t count; // Number of items in the buffer
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} shared_buffer_t;

void bufferAdd(shared_buffer_t*,char*);
void bufferRemove(shared_buffer_t*);