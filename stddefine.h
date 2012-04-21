#define MAX_TIME 5
#define max_client 10
#define BUFFER 200
#define MAX_BUFFER 1000

#ifndef server_webfiles_root
#define server_webfiles_root "/var/www/"
#endif

#ifndef server_webfiles_uploadify
#define server_webfiles_uploadify stradd(server_webfiles_root, "uploadify/")
#endif

#ifndef server_webfiles_avaclientinfo
#define server_webfiles_avaclientinfo stradd(server_webfiles_root, "client/avaclientinfo.txt")
#endif

#ifndef server_webfiles_client_root
#define server_webfiles_client_root stradd(server_webfiles_root, "client/")
#endif

#ifndef server_server
#define server_server "/home/owenwj/prog/server/"
#endif

#ifndef server_server_server
#define server_server_server stradd(server_server, "server")
#endif

#define task_final_result_location "TaskResult/FinalResult/"
#define task_Location "TaskList/"
#define task_result_location "TaskResult/"
#define server_server_clientinfo "Clientinfo.txt"
#define server_webfiles_open "open/"
#define server_webfiles_close "close/"
#define server_webfiles_client "client.html"
#define server_webfiles_database "database.php"
#define server_webfiles_index "index.html"
#define database_name "wav_test"
#define sharefolder_name "ROOTMENU"

#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <mysql/mysql.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include "soapH.h"
#include "Client.nsmap"

int checkResult(char *filename);
void exit_me();
typedef struct msgtype{
	long mtype;
	char buffer[BUFFER+1];
}msgtype;
typedef struct taskrecfunc
{
	char *funname;
	int (*funcptr)();
}taskrecfunc;
static inline char *stradd(char *des, char *sr)
{
    char *ret = (char *)malloc(BUFFER);
    bzero(ret, BUFFER);
    strcat(ret, des);
    strcat(ret, sr);
    return ret;
}
