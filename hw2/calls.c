int closedir(DIR *dirp);
DIR *fdopendir(int fd);
DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);
void rewinddir(DIR *dirp);
void seekdir(DIR *dirp, long loc);
long telldir(DIR *dirp);
int creat(const char *pathname, mode_t mode);
int open(const char *pathname, int flags, mode_t mode);
int remove(const char *pathname);
int rename(const char *oldpath, const char *newpath);
void setbuf(FILE *stream, char *buf);
int setvbuf(FILE * stream, char * buffer, int mode, size_t size);
char *tempnam(const char *dir, const char *pfx);
FILE *tmpfile(void);
char *tmpnam(char *s);
char *tmpnam_r(char *s);
void exit(int status);
char *getenv(const char *name);
char *mkdtemp(char *template);
int mkostemps(char *template, int suffixlen, int flags);
int putenv(char *string);
int rand(void);
int rand_r(unsigned int *seedp);
void srand(unsigned int seed);
int setenv(const char *name, const char *value, int overwrite);
int system(const char *command);
int chdir(const char *path);
int chown(const char *pathname, uid_t owner, gid_t group);
int close(int fd);
int dup(int oldfd);
int dup2(int oldfd, int newfd);
void _exit(int status);
int execl(const char *path, const char *arg, ... );
int execle(const char *path, const char *arg, ... );
int execlp(const char *file, const char *arg, ... );
int execv(const char *path, char *const *argv);
int execve(const char *filename, char *const *argv, char *const *envp);
int fchdir(int fd);
int fchown(int fd, uid_t owner, gid_t group);
pid_t fork(void);
int fsync(int fd);
int ftruncate(int fd, off_t length);
char *getcwd(char *buf, size_t size);
gid_t getegid(void);
uid_t geteuid(void);
gid_t getgid(void);
uid_t getuid(void);
int link(const char *oldpath, const char *newpath);
int pipe(int *pipefd);
ssize_t pread(int fd, void *buf, size_t count, off_t offset);
ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);
ssize_t read(int fd, void *buf, size_t count);
ssize_t readlink(const char *path, char *buf, size_t bufsiz);
int rmdir(const char *pathname);
int setegid(gid_t egid);
int seteuid(uid_t euid);
int setgid(gid_t gid);
int setuid(uid_t uid);
unsigned int sleep(unsigned int seconds);
int symlink(const char *path1, const char *path2);
int unlink(const char *pathname);
ssize_t write(int fd, const void *buf, size_t count);
int chmod(const char *path, mode_t mode);
int fchmod(int fd, mode_t mode);
int fstat(int fd, struct stat *buf);
int lstat(const char *path, struct stat *buf);
int mkdir(const char *pathname, mode_t mode);
int mkfifo(const char *pathname, mode_t mode);
int stat(const char *path, struct stat *buf);
mode_t umask(mode_t mask);
void *realloc(void *ptr, size_t size);
