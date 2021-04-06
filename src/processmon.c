#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#include "processmon.h"

static const char short_options[] = "l:i:";
static const struct option long_options[] = {
    {"learntime", required_argument, NULL, 'l'},
    {"sleepinterval", required_argument, NULL, 'i'},

    {"help", no_argument, NULL, 'h'},

    { 0, 0, 0, 0},
};

static int learntime = 60;
static int sleepinterval = 5;

static CMDLINE_INFO *g_process_while_list[MAX_PROCESSES_ALLOWED];

static CMDLINE_INFO *g_process_lock_list[MAX_PROCESSES_LOCKED];

static int process_report_interval[MAX_PROCESS_REPORT_CNT] = {600, 1800};

static int time_seconds(void)
{
    return time((time_t*)NULL);
}

static CMDLINE_INFO *process_new(void)
{
    return calloc(1, sizeof(CMDLINE_INFO));
}

static int find_empty_slot(CMDLINE_INFO **list, int list_size)
{
    int i;

    for(i=0;i<list_size;i++){
        if(!list[i]){
            return i;
        }
    }

    return -1;
}

static int find_cmdline(CMDLINE_INFO **list, int list_size, const char *cmdlinestr, int cmdlinestr_len)
{
    int i;
    CMDLINE_INFO *info;

    for(i=0;i<list_size;i++){
        info = list[i];
        if(!info){
            continue;
        }
        if(info->cmdline_len != cmdlinestr_len){
            continue;
        }
        if(!memcmp(info->cmdline_str, cmdlinestr, cmdlinestr_len)){
            return i;
        }
    }

    return -1;
}


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
    char pidpath[2048] = {0};
    char buff[CMDLINE_MAX_LEN + 1] = {0};
    int fd, ret;
    char pid[64] = {0};

    if(!(d = opendir(PROC_PATH))) {
        printf("opendir %s error.\n", PROC_PATH);
        return;
    }

    while((dp = readdir(d)) != NULL) {
        if((!strncmp(dp->d_name, ".", 1)) || (!strncmp(dp->d_name, "..", 2))){
            continue;
        }
        snprintf(pidpath, sizeof(pidpath) - 1, "%s/%s", PROC_PATH, dp->d_name);
        if(stat(pidpath, &st) < 0){
            printf("stat [%s] err.\n", pidpath);
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

        snprintf(buff, sizeof(buff) - 1, "%s/cmdline", pidpath);

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

static int processes_learning(const char *pid, char *cmdline, int reallen)
{
    int i = find_cmdline(g_process_while_list, MAX_PROCESSES_ALLOWED, cmdline, reallen);
    CMDLINE_INFO *process;

    if(i >= 0){
        return 0;
    }

    printf("Learned new process pid=[%s] cmd=[%s] cmdlen=[%d]\n", pid, cmdline, reallen);

    i = find_empty_slot(g_process_while_list, MAX_PROCESSES_ALLOWED);

    if(i < 0){
        printf("No empty slot.\n");
        return -1;
    }

    process = process_new();
    if(!process){
        printf("No memory.\n");
        return -1;
    }

    process->type = PROCESS_TYPE_LEARNED;
    strcpy(process->pid, pid);
    process->cmdline_len = reallen;
    memcpy(process->cmdline_str, cmdline, reallen);

    g_process_while_list[i] = process;

    return 0;
}

static int report_process(CMDLINE_INFO *process)
{
    int time_now;

    if(process->report_cnt >= MAX_PROCESS_REPORT_CNT){
        return 0;
    }

    time_now = time_seconds();

    if(!process->report_cnt || ((time_now - process->report_last_second) >= process_report_interval[process->report_cnt - 1])){

        printf("Report process lock: pid=[%s] cmd=[%s] cmdlen=[%d]\n", process->pid, process->cmdline_str, process->cmdline_len);

        process->report_last_second = time_now;
        process->report_cnt++;
    }

    return 0;
}

static int processes_locking(const char *pid, char *cmdline, int reallen)
{
    int i = find_cmdline(g_process_while_list, MAX_PROCESSES_ALLOWED, cmdline, reallen);
    int pidint;
    CMDLINE_INFO *process;

    if(i >= 0){
        return 0;
    }

    printf("Unknown process pid=[%s] cmd=[%s] cmdlen=[%d]\n", pid, cmdline, reallen);

    pidint = atoi(pid);

    i = kill(pidint, SIGKILL);
    if(i < 0){
        printf("Process pid=[%d] kill err=[%d]\n", pidint, i);
    }else{
        printf("Process pid=[%d] killed\n", pidint);
    }

    i = find_cmdline(g_process_lock_list, MAX_PROCESSES_LOCKED, cmdline, reallen);
    if(i < 0){

        i = find_empty_slot(g_process_lock_list, MAX_PROCESSES_LOCKED);
        if(i < 0){
            printf("No empty slot.\n");
            return -1;
        }

        process = process_new();
        if(!process){
            printf("No memory.\n");
            return -1;
        }
        process->type = PROCESS_TYPE_LOCKED;
        strcpy(process->pid, pid);
        process->cmdline_len = reallen;
        memcpy(process->cmdline_str, cmdline, reallen);

        g_process_lock_list[i] = process;

    }else{
        process = g_process_lock_list[i];
    }

    report_process(process);

    return 0;
}


static int cmd_parse_args(int argc, char **argv)
{
    int opt;
    char **argvopt;
    int option_index;

    argvopt = argv;
    while((opt = getopt_long(argc, argvopt, short_options, long_options, &option_index)) != EOF) {
        switch(opt){
            case 'l':
                learntime = atoi(optarg);
                break;
            case 'i':
                sleepinterval = atoi(optarg);
                break;
            default:
                break;
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    int i;

    cmd_parse_args(argc, argv);

    memset(g_process_while_list, 0, sizeof(g_process_while_list));
    memset(g_process_lock_list, 0, sizeof(g_process_lock_list));

    printf("===== process lock starts ... learn=[%d], interval=[%d] [%d] =====\n", learntime, sleepinterval, time_seconds());

    printf("===== process learning starts =====\n");

    i = 0;
    while(1){
        trave_dir(&processes_learning);
        sleep(sleepinterval);
        i += sleepinterval;
        if(i > learntime){
            printf("\n");
            break;
        }
        printf(".");
        fflush(stdout);
    }

    printf("===== process learning over =====\n");

    printf("===== process locking starts =====\n");

    i = 0;
    while(1){
        trave_dir(&processes_locking);
        sleep(sleepinterval);
        printf(".");
        fflush(stdout);
    }

    printf("===== process locking over =====\n");

    return 0;
}

