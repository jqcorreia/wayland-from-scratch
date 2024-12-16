#define _POSIX_C_SOURCE 200112L
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

void randname(char* buf);

int create_shm_file(void);

int allocate_shm_file(size_t size);
