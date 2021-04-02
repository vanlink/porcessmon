#ifndef _PROCESSMON_H
#define _PROCESSMON_H

#define PROC_PATH "/proc"

typedef int (*PROC_FUNC)(const char *pid, char *cmdline, int reallen);

#endif

