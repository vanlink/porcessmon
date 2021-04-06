#ifndef _PROCESSMON_H
#define _PROCESSMON_H

#define PROC_PATH "/proc"

#define CMDLINE_MAX_LEN 4096

#define MAX_PROCESSES_ALLOWED 8192

#define PROCESS_TYPE_LEARNED   0
#define PROCESS_TYPE_WHITELIST 1

typedef int (*PROC_FUNC)(const char *pid, char *cmdline, int reallen);

typedef struct CMDLINE_INFO_tag {
    char pid[64];
    int type;
    int cmdline_len;
    char cmdline_str[CMDLINE_MAX_LEN + 1];
} CMDLINE_INFO;

#endif

