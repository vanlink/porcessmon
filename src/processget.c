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
#include <sys/msg.h>
#include <errno.h>

#include "processmon.h"

static int msg_req_q;
static int msg_rsp_q;

int main(int argc, char **argv)
{
    int ret;
    MSG msg;

    msg_req_q = msgget((key_t)MSG_REQ_Q_KEY, IPC_EXCL);
    if(msg_req_q < 0){
        printf("create msg req q error.\n");
        return -1;
    }

    msg_rsp_q = msgget((key_t)MSG_RSP_Q_KEY, IPC_EXCL);
    if(msg_rsp_q < 0){
        printf("create msg rsp q error.\n");
        return -1;
    }

    msg.type = MSG_REQ_GET_PROCESSES;
    ret = msgsnd(msg_req_q, &msg, sizeof(msg), IPC_NOWAIT);
    if(ret < 0){
        printf("send msg q error | errno=%d [%s]\n", errno, strerror(errno));
        return -1;
    }

    return 0;
}

