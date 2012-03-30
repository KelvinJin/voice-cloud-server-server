#define MAX_TIME 10
#define max_client 10
#define BUFFER 63
#define MAX_BUFFER 100
#define server_webfiles_root "/var/www/"
#define task_Location "/var/www/TaskList/"
#define task_result_location "/var/www/TaskResult/"
#define task_final_result_location "/var/www/TaskResult/FinalResult/"
#define server_server "/home/owenwj/prog/server/"
#define server_server_clientinfo "/home/owenwj/prog/server/Clientinfo.txt"
#define server_server_server "/home/owenwj/prog/server/server"
#define server_webfiles_uploadify "/var/www/uploadify/"
#define server_webfiles_open "/var/www/open/"
#define server_webfiles_close "/var/www/close/"
#define server_webfiles_client "client.html"
#define server_webfiles_database "database.php"
#define server_webfiles_index "index.html"
#define server_webfiles_avaclientinfo "/var/www/client/avaclientinfo.txt"
#define server_webfiles_client_root "/var/www/client/"
#define database_name "wav_test"
#define sharefolder_name "ROOTMENU"
typedef struct msgtype{
	long mtype;
	char buffer[BUFFER+1];
}msgtype;
typedef struct taskrecfunc
{
	char *funname;
	int (*funcptr)();
}taskrecfunc;
