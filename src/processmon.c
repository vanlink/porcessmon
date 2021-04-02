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

    for (i = 0; i < len; i++){
        if (!(isdigit(str[i]))){
            return 0;
        }
    }
    
    return 1;
}

static void trave_dir(char *path)
{
    DIR *d = NULL;
    struct dirent *dp = NULL;
    struct stat st;
    char p[2048] = {0};
    
    if(stat(path, &st) < 0 || !S_ISDIR(st.st_mode)) {
        printf("invalid path: %s\n", path);
        return;
    }

    if(!(d = opendir(path))) {
        printf("opendir[%s] error: %m\n", path);
        return;
    }

    while((dp = readdir(d)) != NULL) {
        if((!strncmp(dp->d_name, ".", 1)) || (!strncmp(dp->d_name, "..", 2))){
            continue;
        }
        snprintf(p, sizeof(p) - 1, "%s/%s", path, dp->d_name);
        stat(p, &st);
        if(!S_ISDIR(st.st_mode)) {
            continue;
        }
        if(!is_int(dp->d_name)){
            continue;
        }
        printf("DIR: %s\n", p);
    }

    closedir(d);

    return;
}

int main(int argc, char **argv)
{
    trave_dir("/proc");

    return 0;
}

