#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <mysql/mysql.h>
#include "soapH.h"
#include "Client.nsmap"
#include "stddefine.h"
struct taskrecfunc symtab[] = {
	{"soap_call_ns__IsBusy", soap_call_ns2__IsBusy},
	{"soap_call_ns__BuildShareFolder", soap_call_ns2__BuildShareFolder},
};
int msgid;
int cli_core_num[max_client];
int ava_client = 0;
char *cli_ip_port[max_client];
char *clientinfo;
char *databaseinfo[4];
struct ns2__soap_string sharefolderinfo[3];
int init_sharefolder()
{
    MYSQL conn;
    mysql_init(&conn);
    MYSQL_RES *result;
    MYSQL_ROW row;
    char *comm;
    int i;
    if (!mysql_real_connect(&conn, databaseinfo[0], databaseinfo[2], databaseinfo[3], database_name, 0, NULL, 0))
	{
		fprintf(stderr, "Failed to connect to database: Error: %s\n",
		        mysql_error(&conn));
		printf("mysql connect failed!\n");
		return -1;
	}
	else
	{
	    comm = (char *)malloc(BUFFER);
	    bzero(comm, BUFFER);
	    strcat(comm, "select * from ");
	    strcat(comm, sharefolder_name);
	    if(mysql_real_query(&conn, comm, (unsigned int)strlen(comm)) != 0)
	    {
	        printf("mysql query wrong!%s\n", mysql_error(&conn));
            mysql_close(&conn);
            return -1;
	    }
	    result = mysql_use_result(&conn);
	    row = mysql_fetch_row(result);
	    for (i = 0; i < 3; i++)
	    {
            strcat(sharefolderinfo[i].str, (char *)row[i]);
            sharefolderinfo[i].size = strlen(sharefolderinfo[i].str);
	    }
	    //printf("sharefolder info:\npath:%s lenth:%d\nusername:%s lenth:%d\npassword:%s lenth:%d\n", sharefolderinfo[0].str, sharefolderinfo[0].size, sharefolderinfo[1].str, sharefolderinfo[1].size, sharefolderinfo[2].str, sharefolderinfo[2].size);
	    mysql_close(&conn);
		return 0;
	}
}
int send_sharefolder(char *cli_ip_port)
{
    struct soap soap;
	soap_init(&soap);
	soap.connect_timeout = MAX_TIME;
	soap.send_timeout = MAX_TIME;
	soap.recv_timeout = MAX_TIME;
	struct ns2__BuildShareFolderResponse BuildShareFolderResponse;
    if(symtab[1].funcptr(&soap, cli_ip_port, NULL, &sharefolderinfo[0], &sharefolderinfo[1], &sharefolderinfo[2], &BuildShareFolderResponse) == SOAP_OK)
    {
        printf("send share folder info to %s success!\n", cli_ip_port);
    }
    else
    {
        printf("send share folder info wrong!\n");
        soap_destroy(&soap);
        soap_end(&soap);
        soap_done(&soap);
        return -1;
    }
    soap_destroy(&soap);
    soap_end(&soap);
    soap_done(&soap);
    return 0;
}
void init_data()
{
    int i;
    for (i =0 ; i < max_client; i++)
    {
        cli_ip_port[i] = (char *)malloc(BUFFER);
        bzero(cli_ip_port[i], BUFFER);
    }
    clientinfo = (char *)malloc(BUFFER);
    bzero(clientinfo, BUFFER);
    for (i = 0; i < 4; i++)
    {
        databaseinfo[i] = (char *)malloc(BUFFER);
        bzero(databaseinfo[i], BUFFER);
    }
    for (i = 0; i < 3; i++)
    {
        sharefolderinfo[i].str = (char *)malloc(100);
        bzero(sharefolderinfo[i].str, 100);
    }
}
int countClientNumber(char *clientinfo)
{
	FILE *fp_client;
	char *buf_client;
	int temp = 0;
	int i;
	fp_client = fopen(clientinfo, "rt+");
    printf("read info from %s\n", clientinfo);
	if(fp_client == NULL)
	{
		printf("fail to open Clientinfo.txt\n");
		return -1;
	}

	buf_client = (char *)malloc(BUFFER);

	while(!feof(fp_client))
	{

		bzero(buf_client, BUFFER);
		fgets(buf_client, BUFFER, fp_client);

		if(strlen(buf_client) == 1)
		{
			break;
		}

		buf_client[strlen(buf_client)-1] = '\0';
		for (i = 0; i < max_client; i++)
		{

		    if (strlen(cli_ip_port[i]) == 0)
		    {
                strcat(cli_ip_port[i], "http://");
                strcat(cli_ip_port[i], buf_client);
                strcat(cli_ip_port[i], ":");
                break;
		    }
		}

		bzero(buf_client, BUFFER);
		fgets(buf_client, BUFFER, fp_client);
		buf_client[strlen(buf_client)-1] = '\0';
		strcat(cli_ip_port[i], buf_client);
		strcat(cli_ip_port[i], "/");
		//printf("client %d's IP:PORT : %s\n", temp + 1, cli_ip_port[temp]);
		bzero(buf_client, BUFFER);
		fgets(buf_client, BUFFER, fp_client);
		//printf("client %d's CORENUM : %s", temp, buf_client);
		cli_core_num[i] = atoi(buf_client);
		temp++;
	}

	fclose(fp_client);
	return i;
}
char *com_general(char *status, char *file)
{
	char *command;
	command = (char *)malloc(MAX_BUFFER);
	bzero(command, MAX_BUFFER);
	strcat(command, "cp ");
	strcat(command, status);
	strcat(command, file);
	strcat(command, " ");
	strcat(command, server_webfiles_root);
	printf("%s\n", command);
	return command;
}
void web_control(int com_id)
{


	switch (com_id)
	{
	case 1:
		system(com_general(server_webfiles_open, server_webfiles_client));
		system(com_general(server_webfiles_open, server_webfiles_database));
		break;
	case 2:
		system(com_general(server_webfiles_close, server_webfiles_client));
		system(com_general(server_webfiles_close, server_webfiles_database));
		break;
	case 3:
		system(com_general(server_webfiles_open, server_webfiles_index));
		break;
	case 4:
		system(com_general(server_webfiles_close, server_webfiles_index));
		break;
	case 0:
		break;

	}
}
int record_avaclient()
{
    FILE *fp_client;
    int i;
    char *a;
    a = (char *)malloc(BUFFER);
    bzero(a, BUFFER);
    if ((fp_client = fopen(server_webfiles_avaclientinfo, "w+")) == NULL)
	{
		printf("config file doesn't exist!\n");
		return -1;
	}
    for (i = 0; i < max_client; i++)
    {
        if (strlen(cli_ip_port[i]) != 0)
        {
            fputs(cli_ip_port[i], fp_client);
            fputs("\n", fp_client);
            sprintf(a, "%d", cli_core_num[i]);
            fputs(a, fp_client);
            fputs("\n", fp_client);
        }
    }
    fputs("\n", fp_client);
    fclose(fp_client);
    return 0;
}
int check_database_connect()
{
    MYSQL conn;
    mysql_init(&conn);
//	bzero(temp, BUFFER);
//	strcat(temp, db_ip);
//	strcat(temp, ":");
//	strcat(temp, db_port);
//  db_port is invaild, i don't know what has happened!
//  so just use db_ip and the default port 3306.
    if (!mysql_real_connect(&conn, databaseinfo[0], databaseinfo[2], databaseinfo[3], database_name, 0, NULL, 0))
	{
		fprintf(stderr, "Failed to connect to database: Error: %s\n",
		        mysql_error(&conn));
		printf("mysql connect failed!\n");
		return -1;
	}
	else
	{
	    mysql_close(&conn);
		return 0;
	}
}
//init database by config
int init_database_config(char *filename)
{

	FILE *fp_config;
	char *temp;
	int i;

	if ((fp_config = fopen(filename, "rt+")) == NULL)
	{
		printf("config file doesn't exist!\n");
		return -1;
	}


	temp = (char *)malloc(BUFFER);

	while(!feof(fp_config))
	{
		bzero(temp, BUFFER);
		fgets(temp, BUFFER, fp_config);

		if (strstr(temp, "db_ip") != NULL)
		{
			if ((strstr(temp, "db_ip") - temp) == 0)
			{
				temp[strlen(temp)-1] = '\0';
				strcat(databaseinfo[0], temp + 6);
			}
		}

		if (strstr(temp, "db_port") != NULL)
		{
			if ((strstr(temp, "db_port") - temp) == 0)
			{
				temp[strlen(temp)-1] = '\0';
				strcat(databaseinfo[1], temp + 8);
			}
		}

		if (strstr(temp, "db_username") != NULL)
		{
			if ((strstr(temp, "db_username") - temp) == 0)
			{
				temp[strlen(temp)-1] = '\0';
				strcat(databaseinfo[2], temp + 12);
			}
		}

		if (strstr(temp, "db_password") != NULL)
		{
			if ((strstr(temp, "db_password") - temp) == 0)
			{
				temp[strlen(temp)-1] = '\0';
				strcat(databaseinfo[3], temp + 12);
			}
		}

		if (strstr(temp, "clientinfo") != NULL)
		{
			if ((strstr(temp, "clientinfo") - temp) == 0)
			{
				temp[strlen(temp)-1] = '\0';
				strcat(clientinfo, temp + 11);
			}
		}
	}
    for (i == 0; i < 4; i++)
    {
        if (databaseinfo[i] != NULL)
        {
        }
        else
        {
            printf("database info isn't complete! please check the config!\n");
            return -1;
        }
    }
    return 0;
}
//init database by network
int init_database_network()
{

}
int check_isBusy(char *cli_ip_port)
{
    int status;
	int state = 1;
    struct soap soap;
	soap_init(&soap);
	soap.connect_timeout = MAX_TIME;
	soap.send_timeout = MAX_TIME;
	soap.recv_timeout = MAX_TIME;
	state = symtab[0].funcptr(&soap, cli_ip_port, NULL, &status);
	if (state == SOAP_OK && status == 0)
    {
        soap_destroy(&soap);
        soap_end(&soap);
        soap_done(&soap);
        return 0;
    }
    if (state == SOAP_OK && status == 1)
    {
        soap_destroy(&soap);
        soap_end(&soap);
        soap_done(&soap);
        return 1;
    }
    printf("check isbusy of client %s failed!\n", cli_ip_port);
    soap_destroy(&soap);
	soap_end(&soap);
	soap_done(&soap);
    return -1;
}
//init client
int init_client()
{

	int cur_client;
	int i;
	int status = 1;
	ava_client = 0;

	if (clientinfo != NULL)
	{
		cur_client = countClientNumber(clientinfo);

        for (i = 0; i <= cur_client; i++)
        {
            status = check_isBusy(cli_ip_port[i]);

            if (status == 0)
            {
                ava_client++;
            }
            else
            {
                bzero(cli_ip_port[i], BUFFER);
            }
        }
    }
    else
    {

    }
	if (ava_client >= 1)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}
int init_client_network()
{
    key_t key;
    struct msgtype msg;
    int i,j;
    int flags;
    char buf_temp[BUFFER];
    char *tempname;
    FILE *fp_client;
    key = ftok(server_webfiles_client_root, 'a');
    if((msgid = msgget(key, 0777 | IPC_CREAT)) == -1)
	{
		printf("Creat Message Error!\n");
		exit(1);
	}
    memset(&msg, 0, sizeof(struct msgtype));
    while (1)
    {
        if (ava_client == 0)
        {
            msgrcv(msgid, &msg, sizeof(struct msgtype), 1, 0);
        }
        else
        {
            msgrcv(msgid, &msg, sizeof(struct msgtype), 1, IPC_NOWAIT);
        }
        if (errno == EAGAIN || errno == ENOMSG)
        {
            if (ava_client > 0)
            {
                return 0;
            }
            else
            {
                continue;
            }
        }
        for(i = 0, j = 0; i < strlen(msg.buffer); i++)
        {
            if(msg.buffer[i] == '\"')
            {
                i++;

                while(msg.buffer[i] != '\"')
                {
                    buf_temp[j] = msg.buffer[i];
                    j++;
                    i++;
                }

                buf_temp[j] = '\0';
                break;
            }
        }
        tempname = (char *)malloc(1000);
        bzero(tempname, 1000);
        strcat(tempname, server_webfiles_client_root);
        strcat(tempname, buf_temp);
        strcat(tempname, ".txt");
        i = countClientNumber(tempname);
        if (i > -1)
        {
            if (check_isBusy(cli_ip_port[i]) == 0)
            {
                if(send_sharefolder(cli_ip_port[i]) == 0)
                {
                    ava_client++;
                }
                else
                {
                    printf("send sharefolder to %s failed!\n", cli_ip_port[i]);
                    bzero(cli_ip_port[i], BUFFER);
                }
            }
            else
            {
                printf("isBusy return not 0 when check client %s!\n", cli_ip_port[i]);
                bzero(cli_ip_port[i], BUFFER);
            }
        }
        else
        {

        }
        if (ava_client > 0)
        {
            return 0;
        }
    }
}
//check database status
int check_database()
{
    return 0;
}
//check clients status
int check_client()
{
    int i;
    for (i = 0; i < max_client; i++)
    {
        if (strlen(cli_ip_port[i]) != 0)
        {
            if (check_isBusy(cli_ip_port[i]) == -1)
            {
                bzero(cli_ip_port[i], BUFFER);
                ava_client--;
            }
        }
    }
    if (ava_client == 0)
    {
        return -1;
    }
    else
    {
        init_client_network();
        if (record_avaclient() != 0)
        {
            printf("record avaclient fail!\n");
            exit(1);
        }
        return 0;
    }
}
int do_while_server_exit(int pid)
{
    if (waitpid(pid, NULL, WNOHANG) > 0)
    {
        web_control(2);
        web_control(4);
        msgctl(msgid, IPC_RMID, (struct msqid_ds*)0);
        exit(0);
    }
    return 0;
}
int main(int argc, char *argv[])
{
	int com_id;
	printf("please input command id:\n");
	printf("1 for open client and database php:\n");
	printf("2 for close client and database php:\n");
	printf("3 for open index html:\n");
	printf("4 for close index html:\n");
	printf("0 for do nothing\n");
	scanf("%d", &com_id);
	web_control(com_id);
	printf("%s\n", argv[1]);
    init_data();
	if (strcmp(argv[1], "config") == 0)
	{
		if (init_database_config(argv[2]) == 0)
		{
			printf("read database info success!\nnow try to connect\n");
			if (check_database_connect() == 0)
			{
                printf("init database successful!\n");
                if (init_sharefolder() == 0)
                {
                    printf("init sharefolder successful!\n");
                }
                else
                {
                    printf("init sharefolder failed!\n");
                    exit(1);
                }
			}
			else
            {
                printf("init database failed!\n");
                exit(1);
            }
		}
		else
		{
			printf("read database info failed!\n");
			exit(1);
		}
	}

	if (strcmp(argv[1], "network") == 0)
	{
		init_database_network();
	}

	if (init_client() == 0)
	{
		printf("init client from file success!\n");

	}
	else
	{
		printf("init client from file fail!\nnow try to wait for connections from network!\n");
        init_client_network();
	}
    int pid;
    if (record_avaclient() != 0)
    {
        printf("record avaclient fail!\n");
        exit(1);
    }
    pid = fork();
    if (pid < 0)
    {
        printf("fork fail!\n");
        exit(1);
    }
    else if (pid == 0)
    {
        if (!execl(server_server_server, "./server", server_webfiles_avaclientinfo, NULL))
        {
            printf("execl error!\n");
            exit(1);
        }
    }
    else
    {
        web_control(3);
    }
    while(1)
    {
        sleep(10);
        printf("checking database connect...");
        if (check_database_connect() != 0)
        {
            printf("connect database fail!\nclose index.html!\ntry to wait for connection from network!\n");
            web_control(4);
            init_database_network();
            web_control(3);
        }
        else
        {
            printf("OK\nchecking client...");
        }
        if (check_client() != 0)
        {
            printf("no available client!\nclose index.html!\n");
            web_control(4);
            printf("try to wait for connection from network!\n");
            init_client_network();
            if (record_avaclient() != 0)
            {
                printf("record avaclient fail!\n");
                exit(1);
            }
            web_control(3);
        }
        else
        {
            printf("OK\n");
        }
        do_while_server_exit(pid);
    }
	return 0;
}
