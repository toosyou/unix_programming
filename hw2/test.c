#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/socket.h>
int main(){
    getlogin();
    getpid();
    int _socket = socket(PF_INET, SOCK_STREAM, 0);
    close(_socket);
    struct stat st;
    setuid(getuid());
    setgid(getgid());
    seteuid(geteuid());
    setegid(getegid());
    mkdir("_test_dir", 0775);
    chdir("_test_dir");
    char _tmpnam[L_tmpnam];
    tmpnam(_tmpnam);
    char _mkstemp_template[] = "temp-XXXXXX";
    int _mkstemp = mkstemp(_mkstemp_template);
    char p_buf[1024];
    pwrite(_mkstemp, "test", 5, 0);
    pread(_mkstemp, p_buf, 1024, 0);
    fchmod(_mkstemp, 0666);
    fchown(_mkstemp, getuid(), getgid());
    int _dup = dup(_mkstemp);
    int _dup2 = dup2(_dup, _mkstemp);
    close(_dup);
    ftruncate(_mkstemp, 0);
    fsync(_mkstemp);
    fstat(_mkstemp, &st);
    close(_mkstemp);
    symlink(_mkstemp_template, "symlink");
    lstat("symlink", &st);
    link(_mkstemp_template, "hardlink");
    stat("hardlink", &st);
    char _readlink[1024];
    readlink("symlink", _readlink, 1024);
    unlink("hardlink");
    remove(_mkstemp_template);
    char _mkdtemp_template[] = "temp-XXXXXX";
    char *_mkdtemp = mkdtemp(_mkdtemp_template);
    remove(_mkdtemp);
    char *_tempnam = tempnam(".", "tmp");
    free(_tempnam);
    FILE *_tmpfile = tmpfile();
    fputs_unlocked("test", _tmpfile);
    fputc_unlocked('c', _tmpfile);
    char _setbuf[1024];
    setbuf(_tmpfile, _setbuf);
    setvbuf(_tmpfile, _setbuf, _IOLBF, 1024);
    fclose(_tmpfile);
    int creatfd = creat("CREAT", 0664);
    close(creatfd);
    rename("CREAT", "RENAME");
    chmod("RENAME", 0666);
    remove("RENAME");
    int openfd = open("OPEN CREAT", O_WRONLY | O_CREAT, 0664);
    close(openfd);
    int openrdfd = open("OPEN CREAT", O_RDONLY);
    close(openrdfd);
    remove("OPEN CREAT");
    umask(0000);
    geteuid();
    getegid();
    srand(time(0));
    unsigned int seed = rand();
    rand_r(&seed);
    setenv("TEST", "TESTENV1", 1);
    getenv("TEST");
    putenv("TEST=TESTENV2");
    getenv("TEST");
    int p[2];
    pipe(p);
    close(p[0]);
    close(p[1]);
    mkfifo("fifo", 0664);
    int fifofd = open("fifo", O_RDWR);
    write(fifofd, "\x01\x02\x03\x04", 5);
    char buf[PATH_MAX];
    read(fifofd, buf, PATH_MAX);
    close(fifofd);
    remove("fifo");
    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);
    for(int i = 0 ; i < 5 ; i++) {
        char _mkstemp_template[] = "tmp-XXXXXX";
        int _mkstemp = mkstemp(_mkstemp_template);
        close(_mkstemp);
    }
    DIR *dir = opendir(".");
    struct dirent *dirp;
    while((dirp = readdir(dir))) {
    }
    telldir(dir);
    seekdir(dir, 0);
    rewinddir(dir);
    struct dirent _dirp, *__dirp = &_dirp;
    while(readdir_r(dir, &_dirp, &__dirp) == 0 && __dirp) {
        if(strcmp(_dirp.d_name, ".") && strcmp(_dirp.d_name, ".."))
            remove(_dirp.d_name);
    }
    closedir(dir);
    int _fchdir = open("..", O_RDONLY);
    fchdir(_fchdir);
    DIR *_fdopendir = fdopendir(_fchdir);
    closedir(_fdopendir);
    rmdir("_test_dir");
    system("echo -n \"\"");
    sleep(1);
    int pid;
    if(!(pid = fork())){
        exit(0);
    }
    waitpid(pid, NULL, 0);
    if(!(pid = fork())){
        _exit(0);
    }
    waitpid(pid, NULL, 0);
    char *envp[] = {"PATH=/tmp", "STATUS=testing", NULL};
    char *argv_execv[] = {"echo", "excuted by execv", NULL};
    char *argv_execvp[] = {"echo", "executed by execvp", NULL};
    char *argv_execve[] = {"env", NULL};
    if(!(pid = fork())){
        execl("/bin/echo", "echo", "executed by execl", NULL);
        exit(0);
    }
    waitpid(pid, NULL, 0);
    if(!(pid = fork())) {
        execle("/usr/bin/env", "env", NULL, envp);
        exit(0);
    }
    waitpid(pid, NULL, 0);
    if(!(pid = fork())) {
        execlp("echo", "echo", "executed by execlp", NULL);
        exit(0);
    }
    waitpid(pid, NULL, 0);
    if(!(pid = fork())) {
        execv("/bin/echo", argv_execv);
        exit(0);
    }
    waitpid(pid, NULL, 0);
    if(!(pid = fork())) {
        execve("/usr/bin/env", argv_execve, envp);
        exit(0);
    }
    waitpid(pid, NULL, 0);
    if(!(pid = fork())) {
        execvp("echo", argv_execvp);
        exit(0);
    }
    waitpid(pid, NULL, 0);
    return 0;
}
