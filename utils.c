#include "utils.h"

char* qx(char** cmd)
{
    int stdout_fds[2];
    pipe(stdout_fds);

    const pid_t pid = fork();
    if (pid == 0) {
        printf("Child\n");
        // Child
        close(stdout_fds[0]);
        dup2(stdout_fds[1], 1);

        close(stdout_fds[1]);

        int res = execvp(*cmd, cmd);
        printf("exec res: %d\n", res);
        exit(0);
    }

    close(stdout_fds[1]);

    const int buf_size = 4096;
    char* out = malloc(buf_size);
    int out_size = buf_size;
    int i = 0;
    do {
        const ssize_t r = read(stdout_fds[0], &out[i], buf_size);
        if (r > 0) {
            i += r;
        }

        if (out_size - i <= 4096) {
            out_size *= 2;
            out = realloc(out, out_size);
        }
    } while (errno == EAGAIN || errno == EINTR);

    close(stdout_fds[0]);

    int r, status;
    do {
        r = waitpid(pid, &status, 0);
    } while (r == -1 && errno == EINTR);

    out[i] = 0;

    return out;
}
