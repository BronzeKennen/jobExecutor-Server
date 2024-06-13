typedef struct {
    int socketFd;
    int id;
    char *job;
} job;

typedef struct { //Warning, trying to assign elements that do not exist causes SIGSEGV
    job** buffer;
    size_t in;  // Next write position
    size_t out; // Next read position
    size_t count; // Number of items in the buffer
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} shared_buffer_t;

void bufferAdd(shared_buffer_t*,job*,int);
job* bufferRemove(shared_buffer_t*,int);
void bufferPrint(shared_buffer_t* buf);