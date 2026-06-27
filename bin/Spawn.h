
static inline pid_t Spawn(const char *com, int *read_fd, bool nonBlocking) {

    DASSERT(read_fd);

    int fd[2] = { -1, -1 };
    ASSERT(pipe(fd) == 0);
    ASSERT(fd[0] >= 3);
    ASSERT(fd[1] >= 3);
    pid_t pid = fork();

    switch(pid) {
        case -1:
            ASSERT(0, "fork() failed");
            exit(EXIT_FAILURE);
        case 0:
            // I'm the child.
            close(fd[0]); // close read fd.
            errno = 0;
            ASSERT(dup2(fd[1], 1) == 1);
            // Now the stdin is this pipe write fd.
            // After execl() this process writes stdout (fd=1) to the
            // write end of the pipe.
            execl("/bin/sh", "sh", "-c", com, NULL);
            ASSERT(0, "execl(,,\"%s\") failed", com);
            exit(EXIT_FAILURE);
        default:
            // I'm the parent
    }
    close(fd[1]); // close write fd.
    int flags = fcntl(fd[0], F_GETFL, 0);
    ASSERT(flags >= 0);

    if(nonBlocking)
        // We need a non-blocking read to it does not hang forever in a
        // read(2) call.
        ASSERT(fcntl(fd[0], F_SETFL, flags|O_NONBLOCK) != -1);

    *read_fd = fd[0]; // pipe read fd.


    return pid;
}
