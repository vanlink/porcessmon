#ifndef _PROCESSMON_H
#define _PROCESSMON_H

#define PROC_PATH "/proc"

#define MSG_REQ_Q_KEY 1232
#define MSG_RSP_Q_KEY 1233

#define CMDLINE_MAX_LEN 4096

#define MAX_PROCESSES_ALLOWED 8192
#define MAX_PROCESSES_LOCKED  8192

#define PROCESS_TYPE_LEARNED   0
#define PROCESS_TYPE_WHITELIST 1
#define PROCESS_TYPE_LOCKED    2

#define MAX_PROCESS_REPORT_CNT 3

typedef int (*PROC_FUNC)(const char *pid, char *cmdline, int reallen);

typedef struct CMDLINE_INFO_tag {
    char pid[64];
    int type;
    int cmdline_len;
    char cmdline_str[CMDLINE_MAX_LEN + 1];

    int killed_cnt;

    int report_cnt;
    int report_last_second;
} CMDLINE_INFO;

#define MSG_REQ_GET_PROCESSES 1
#define MSG_RSP_PROCESSES_CNT 2
#define MSG_RSP_PROCESS_INFO  3

typedef struct MSG_tag {
    long type;

    union {
        int processes_cnt;

        CMDLINE_INFO process_info;
    };
} MSG;

#endif

