#define _GNU_SOURCE // for RTLD_NEXT

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <wchar.h>
#include <errno.h>
#include <sys/param.h>


char buf[300];
extern char** environ;

typedef int fd_t;

ssize_t real_readlink(const char *path, char *buf, size_t bufsiz){
    ssize_t (*_func)(const char * _,  char * __,  size_t ___)=\
		(ssize_t(*)(const char * _,  char * __,  size_t ___))dlsym(RTLD_NEXT, "readlink");
	return _func(path, buf, bufsiz);
}

int real_dirfd(DIR* dirp){
    int (*_func)(DIR * _)=\
        (int(*)(DIR * _))dlsym(RTLD_NEXT, "dirfd");
    return _func(dirp);
}

char* get_fn(FILE * f){
    int fno = fileno(f);
    char proclnk[300];
    sprintf(proclnk, "/proc/self/fd/%d", fno);
    int r = real_readlink(proclnk, buf, 300);
    if (r < 0){
        //printf("%d, failed to readlink\n", fno);
        r = 0;
    }
    buf[r] = '\0';
    return buf;
}

char* get_fn_fd(int fd){
    char proclnk[300];
    sprintf(proclnk, "/proc/self/fd/%d", fd);
    int r = real_readlink(proclnk, buf, 300);
    if (r < 0){
        //printf("%d, failed to readlink\n", fd);
        r = 0;
    }
    buf[r] = '\0';
    return buf;
}

char* get_dn(DIR* dirp){
    return get_fn_fd(real_dirfd(dirp));
}

char *mygetenv(char* env){
    for(int i=0;environ[i]!=NULL;++i){
        char* pequal = strstr(environ[i], "=");
        int index_equal = pequal - environ[i];
        if( strncmp(environ[i], env, index_equal) == 0)
            return pequal+1;
    }
    return NULL;
}

FILE* get_oldest_fd(){
    char* addr_out = mygetenv("MONITOR_OUTPUT");
    static FILE *pf = NULL;

    if(pf != NULL)
        return pf;

    if( addr_out == NULL || strcmp(addr_out, "stdout") == 0)
        pf = stdout;
    else if(strcmp(addr_out, "stderr") == 0)
        pf = stderr;
    else
        pf = fopen(addr_out, "a+");

    return pf;
}

int mp(const char* fmt, ...){

    FILE *pf = get_oldest_fd();
    int rtn = -1;

    if(pf == NULL)
        return rtn;

    va_list args;
    va_start(args, fmt);
    rtn = vfprintf(pf, fmt, args);
    va_end(args);

    return rtn;
}
