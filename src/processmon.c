#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

#include "processmon.h"

static int is_int(const char *str)
{
    int len = strlen(str);
    int i = 0;

    if(!str || !len){
        return 0;
    }

    for (i = 0; i < len; i++){
        if (!(isdigit(str[i]))){
            return 0;
        }
    }
    
    return 1;
}

static void trave_dir(PROC_FUNC func)
{
    DIR *d = NULL;
    struct dirent *dp = NULL;
    struct stat st;
    char p[2048] = {0};
    char buff[4097] = {0};
    int fd, ret;
    char pid[2048] = {0};

    if(!(d = opendir(PROC_PATH))) {
        printf("opendir %s error.\n", PROC_PATH);
        return;
    }

    while((dp = readdir(d)) != NULL) {
        if((!strncmp(dp->d_name, ".", 1)) || (!strncmp(dp->d_name, "..", 2))){
            continue;
        }
        snprintf(p, sizeof(p) - 1, "%s/%s", PROC_PATH, dp->d_name);
        if(stat(p, &st) < 0){
            printf("stat [%s] err.\n", p);
            continue;
        }
        if(!S_ISDIR(st.st_mode)) {
            continue;
        }
        if(!is_int(dp->d_name)){
            continue;
        }
        if(!strcmp(dp->d_name, "1")){
            continue;
        }
        strcpy(pid, dp->d_name);

        snprintf(buff, sizeof(buff) - 1, "%s/cmdline", p);

        fd = open(buff, O_RDONLY);
        if(fd < 0){
            printf("open [%s] err.\n", buff);
            continue;
        }
        memset(buff, 0, sizeof(buff));
        ret = read(fd, buff, sizeof(buff) - 1);
        close(fd);

        if(ret < 1){
            continue;
        }

        func(pid, buff, ret);
    }

    closedir(d);

    return;
}

static int show(const char *pid, char *cmdline, int reallen)
{
    printf("%-10s %-10d %s\n", pid, reallen, cmdline);
    return 0;
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    trave_dir(&show);

    return 0;
}

