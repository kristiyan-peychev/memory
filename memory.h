#ifndef MEMORY_LMD9Q5J4

#define MEMORY_LMD9Q5J4

#ifdef _LINUX
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

class memory {
    size_t size;
#ifdef _LINUX
    int rnd_sht;
    key_t key;
    int shmid;
    char *start;
    char **end_ptr;
    size_t map_size;
    char **map; // bitmap of the segment
    size_t allocated;
#endif
private:
    size_t next_free(size_t);
public:
    memory(void) = delete;
    memory(const memory &) = delete;
    memory &operator=(const memory &) = delete;
public:
    explicit memory(size_t);
    ~memory(void);
public:
    void *alloc(size_t);
    void free(void *);
};

#endif /* end of include guard: MEMORY_LMD9Q5J4 */
