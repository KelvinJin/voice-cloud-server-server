#include "stddefine.h"
int cli_core_num[max_client];
char *cli_ip_port[max_client];
struct taskrecfunc symtab[] = {
	{"soap_call_ns__ReceiveTaskList", soap_call_ns2__ReceiveTaskList},
	{"soap_call_ns__TaskSolve", soap_call_ns2__TaskSolve},
};
void myfree(void *temp)
{
    if (temp != NULL)
    {
        free(temp);
        temp = NULL;
    }
}
union senum
{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};

/*创建信号量 */
int sem_creat(key_t  key)
{
	union senum sem;
	int         semid;
	sem.val = 0;
	semid = semget(key, 1, IPC_CREAT | 0666);

	if(semid == -1)
	{
		printf("Create semaphore error\n");
		exit(-1);
	}

	semctl(semid, 0, SETVAL, sem);

	return semid;
}

/* 删除信号量*/
int del_sem(int semid)
{
	union senum  sem;
	sem.val = 0;
	semctl(semid, 0, IPC_RMID, sem);
}

/*信号量的P操作，使得信号量的值加1*/
int p(int semid)
{
	struct sembuf sops = {0, +1, SEM_UNDO};
	return (semop(semid, &sops, 1));
}

/*信号量的v操作,使得信号量的值减1*/
int v(int semid)
{
	struct sembuf sops = {0, -1, SEM_UNDO};
	return (semop(semid, &sops, 1));
}
void *dime_read_open(struct soap *soap, void *handle, const char *id, const char *type, const char *options)
{
	return handle;
}
void dime_read_close(struct soap *soap, void *handle)
{
	fclose((FILE*)handle);
}
size_t dime_read(struct soap *soap, void *handle, char *buf, size_t len)
{
	return fread(buf, 1, len, (FILE*)handle);
}
//打印可用客户端
int countClientNumber(char *clientinfo)
{

	FILE *fp_client;
	char *buf_client;
	int temp = 0;
	fp_client = fopen(clientinfo, "r+");

	if(fp_client == NULL)
	{
		printf("fail to open Clientinfo.txt\n");
		exit(1);
	}

	buf_client = (char *)malloc(BUFFER);

	while(!feof(fp_client))
	{
		cli_ip_port[temp] = (char *)malloc(BUFFER);
		bzero(cli_ip_port[temp], BUFFER);
		bzero(buf_client, BUFFER);
		fgets(buf_client, BUFFER, fp_client);

		if(strlen(buf_client) == 1)
		{
			break;
		}

		buf_client[strlen(buf_client)-1] = '\0';
		strcat(cli_ip_port[temp], buf_client);
		bzero(buf_client, BUFFER);
		fgets(buf_client, BUFFER, fp_client);
		buf_client[strlen(buf_client)-1] = '\0';
		cli_core_num[temp] = atoi(buf_client);
		printf("client %d's IP:PORT : %s\n", temp + 1, cli_ip_port[temp]);
		printf("client %d's CORENUM : %s\n", temp + 1, buf_client);
		temp++;
	}

	fclose(fp_client);
	myfree(buf_client);
	return temp;
}
//结果处理
char *getTaskByLine(char *filename, int line)
{
    FILE *fp_task;
    char *task;
    int i = 1;
    char *tempname;
    tempname = (char *)malloc(BUFFER);
    bzero(tempname, BUFFER);
    strcat(tempname, server_webfiles_root);
    strcat(tempname, task_Location);
	strcat(tempname, filename);
	strcat(tempname, ".txt");
    fp_task = fopen(tempname, "rt+");
    myfree(tempname);
    if (fp_task == NULL)
    {
        printf("open file %s erro in getTaskByLine!\n", filename);
        return NULL;
    }
    task = (char *)malloc(BUFFER);
    while (i <= line)
    {
        bzero(task, BUFFER);
        fgets(task, BUFFER, fp_task);
        i++;
    }
    task[strlen(task)-1] = '\0';
    fclose(fp_task);
    return task;

}

void getNumByString(char *temp, int *start, int *end)
{
    int i,j;
    *start = 0;
    *end = 0;
    for(i = 0,j = 0; i < strlen(temp); i++)
    {
        if ('0' <= temp[i] && temp[i] <= '9')
        {
            if (j == 0)
            {
                *start = (*start)*10 + (temp[i] - '0');
            }
            if (j == 1)
            {
                *end = (*end)*10 + (temp[i] - '0');
            }
        }
        else if (temp[i] == ' ')
        {
            j++;
            continue;
        }
    }
}

int resultDeal(char *filename, int status[])
{

	FILE *fp_result_in;
	FILE *fp_result_out;
	FILE *fp_result_out_fail;
	char *tempname;
	char *append;
	char *temp;
	int i;
	int start,end;
	tempname = (char *)malloc(BUFFER);
	bzero(tempname, BUFFER);
	strcat(tempname, server_webfiles_root);
	strcat(tempname, task_final_result_location);
	strcat(tempname, filename);
	strcat(tempname, "_result.txt");
    fp_result_out = fopen(tempname, "wt+");
	if(fp_result_out == NULL)
	{
		printf("result file of %s cannot creat!\n", tempname);
		myfree(tempname);
		return -1;
	}
	bzero(tempname, BUFFER);
	strcat(tempname, server_webfiles_root);
	strcat(tempname, task_final_result_location);
	strcat(tempname, filename);
	strcat(tempname, "_result_fail.txt");

	if((fp_result_out_fail = fopen(tempname, "wt+")) == NULL)
	{
		printf("result fail file of %s cannot creat!\n", filename);
		myfree(tempname);
		return -1;
	}
    append = (char *)malloc(BUFFER);
	temp = (char *)malloc(1000);
	for(i = 0; i < max_client; i++)
	{
		if(status[i] == 0||status[i] == 2)
		{
			bzero(tempname, BUFFER);
			bzero(append, BUFFER);
			strcat(tempname, server_webfiles_root);
			strcat(tempname, task_result_location);
			strcat(tempname, filename);
			sprintf(append, "_client%d_result.txt", i);
			strcat(tempname, append);

			if((fp_result_in = fopen(tempname, "rt+")) == NULL)
			{
				printf("result file of client %d not find!\n", i);
				continue;
			}
			while(!feof(fp_result_in))
			{
				bzero(temp, 1000);
				fgets(temp, 1000, fp_result_in);
                fputs(temp, fp_result_out);
			}
			fclose(fp_result_in);

			bzero(tempname, BUFFER);
			bzero(append, BUFFER);
			strcat(tempname, server_webfiles_root);
			strcat(tempname, task_result_location);
			strcat(tempname, filename);
			sprintf(append, "_client%d_result_fail.txt", i);
			strcat(tempname, append);

			if((fp_result_in = fopen(tempname, "rt+")) == NULL)
			{
				printf("result fail file of client %d not find!\n", i);
				continue;
			}
			while(!feof(fp_result_in))
			{
				bzero(temp, 1000);
				fgets(temp, 1000, fp_result_in);
				fputs(temp, fp_result_out_fail);
			}
			fclose(fp_result_in);
		}
	}
    fclose(fp_result_out);
	fclose(fp_result_out_fail);
	myfree(temp);
	myfree(append);
	if (checkResult(filename) == -1)
	{
        printf("check Result Error of%s:\n", filename);
        myfree(tempname);
		return -1;
	}
    bzero(tempname, BUFFER);
    strcat(tempname, server_webfiles_root);
	strcat(tempname, task_Location);
	strcat(tempname, filename);
	strcat(tempname, ".txt");
	printf("deal result success:%s\n", tempname);
	myfree(tempname);
    return 0;
}

int returnResult(char *filename)
{
    char *tempname = (char *)malloc(BUFFER);
    key_t key;
    struct msgtype msg;
    int msgid;
    memset(&msg, 0, sizeof(struct msgtype));
    strcpy(msg.buffer, filename);
	msg.mtype = 2;
    bzero(tempname, BUFFER);
    strcat(tempname, server_webfiles_root);
	strcat(tempname, task_Location);
	strcat(tempname, filename);
	strcat(tempname, ".txt");
    key = ftok(tempname, 'a');
    myfree(tempname);
	if((msgid = msgget(key, 0777 | IPC_CREAT)) == -1)
	{
		printf("Creat Message Error of%s:\n", filename);
		return -1;
	}

	msgsnd(msgid, &msg, sizeof(struct msgtype), 0);
    return 0;
}

int findTaskinResultFile(char *filename, char *task)
{
    FILE *fp;
    char *tempname;
    char *temp;
    tempname = (char *)malloc(BUFFER);
    bzero(tempname, BUFFER);
    strcat(tempname, server_webfiles_root);
    strcat(tempname, task_final_result_location);
	strcat(tempname, filename);
	strcat(tempname, "_result.txt");
    fp = fopen(tempname, "rt+");
    if (fp == NULL)
    {
        printf("open file %s erro in getTaskByLine!\n", tempname);
        myfree(tempname);
        return -1;
    }
    temp = (char *)malloc(BUFFER);
    while (!feof(fp))
    {
        bzero(temp, BUFFER);
        fgets(temp, BUFFER, fp);
        if (strlen(temp) < 2)
        {
            continue;
        }
        temp[strlen(temp)-1] = '\0';
        if (strcmp(temp, task) == 0)
        {
            fclose(fp);
            myfree(tempname);
            myfree(temp);
            return 0;
        }
    }
    fclose(fp);
    bzero(tempname, BUFFER);
    strcat(tempname, server_webfiles_root);
    strcat(tempname, task_final_result_location);
	strcat(tempname, filename);
	strcat(tempname, "_result_fail.txt");
    fp = fopen(tempname, "rt+");
    if (fp == NULL)
    {
        printf("open file %s erro in getTaskByLine!\n", tempname);
        myfree(tempname);
        myfree(temp);
        return -1;
    }
    myfree(tempname);
    while (!feof(fp))
    {
        bzero(temp, BUFFER);
        fgets(temp, BUFFER, fp);
        if (strlen(temp) < 2)
        {
            continue;
        }
        temp[strlen(temp)-1] = '\0';
        if (strcmp(temp, task) == 0)
        {
            fclose(fp);
            myfree(temp);
            return 0;
        }
    }
    myfree(temp);
    fclose(fp);
    return -1;
}

int checkResult(char *filename)
{
    FILE *fp_task;
    FILE *fp_result_fail;
	char *tempname;
	char *temp;
    //temp = (char *)malloc(BUFFER);
	tempname = (char *)malloc(BUFFER);
    bzero(tempname, BUFFER);
    strcat(tempname, server_webfiles_root);
	strcat(tempname, task_Location);
	strcat(tempname, filename);
	strcat(tempname, ".txt");
	if((fp_task = fopen(tempname, "rt+")) == NULL)
	{
		printf("result file of %s cannot creat!\n", filename);
		myfree(tempname);
		return -1;
	}
    bzero(tempname, BUFFER);
    strcat(tempname, server_webfiles_root);
	strcat(tempname, task_final_result_location);
	strcat(tempname, filename);
	strcat(tempname, "_result_fail.txt");
	if((fp_result_fail = fopen(tempname, "a+")) == NULL)
	{
		printf("result fail file of %s cannot creat!\n", filename);
		fclose(fp_task);
		myfree(tempname);
		return -1;
	}
	myfree(tempname);
	temp = (char *)malloc(BUFFER);
    while (!feof(fp_task))
    {
        //temp = (char *)malloc(BUFFER);
        bzero(temp, BUFFER);
        fgets(temp, BUFFER, fp_task);
        //fscanf(fp_task, "%s", temp);
        if (strlen(temp) < 2)
        {
            continue;
        }
        temp[strlen(temp)-1] = '\0';
        if (findTaskinResultFile(filename, temp+14) == -1)
        {
            fputs(temp+14, fp_result_fail);
            //fwrite(temp, 1, BUFFER, fp_result_fail);
            fputs("\n", fp_result_fail);
        }
    }
	//php will close the massage que.
	printf("check Result success!\n");
	myfree(temp);
	fclose(fp_task);
	fclose(fp_result_fail);
	return 0;
}

int main(int argc, char *argv[])
{
	key_t key;
	int i;
	int j;
	int semid;
	int msgid;
	int segment_size;
	int pid[max_client];
	int cur_line;
	int tot_line;
	int flag[max_client];
	int segment_id;
	int taskId;
	int temppid;
	int cur_client = 0;
	int status[max_client];//子进程是否正常退出
	int dead_client = 0;

	//FILE *fp_user;
	FILE *fp_task;
	FILE *fp_msgid;
	//FILE *fp_result;

	char buf_taskId;
	char buf_temp[BUFFER];
	char *buf_user;
	char *buf_result;
	char *temptask;
	char *shared_memory;
	char *tempname;

	struct msgtype msg;
	struct ns2__data tasklist;
	struct ns2__TaskSolveResponse tasksolveresponse;
	struct ns2__ReceiveTaskListResponse receivetasklistresponse;
	struct shmid_ds shmbuffer;
	struct soap soap;
	struct stat sb;
	struct soap_multipart *attachment;
	//struct retLocation retLoc[max_client]

	//cur_client = countClientNumber();

	//fp_user=fopen("/home/owenwj/prog/client/Userinfo.txt","rt+");
	/*if(fp_user==NULL)
	{
		printf("fail to open Userinfo.txt!\n");
		exit(1);
	}*/

	soap_init(&soap);
	soap.fdimereadopen = dime_read_open;
	soap.fdimereadclose = dime_read_close;
	soap.fdimeread = dime_read;
	soap.connect_timeout = MAX_TIME;
	soap.send_timeout = MAX_TIME;
	soap.recv_timeout = MAX_TIME;
	soap.accept_timeout = MAX_TIME;



	//buf_taskId=(char *)malloc(BUFFER);


	key = ftok(server_webfiles_uploadify, 'a');

	if(key == -1)
	{
		printf("ftok error!\n");
		exit(1);
	}

	//共享内存创建
	segment_id = shmget(key, getpagesize(), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH);

	if(segment_id == -1)
	{
		printf("create shared memory error!\n");
		exit(1);
	}

	//消息队列创建
	if((msgid = msgget(key, 0777 | IPC_CREAT)) == -1)
	{
		printf("Creat Message Error:\n");
		exit(1);
	}

	//msgid写进文本
	printf("msgid:%d\n", msgid);
	buf_user = (char *)malloc(BUFFER + 1);
    buf_result = (char *)malloc(BUFFER + 1);
    temptask = (char *)malloc(200);
    tempname = (char *)malloc(1000);
	while(1)
	{
		memset(&msg, 0, sizeof(struct msgtype));
		msgrcv(msgid, &msg, sizeof(struct msgtype), 1, 0);
		memset(&tasklist, 0, sizeof(tasklist));

		for(i = 0; i < max_client; i++)
		{
			status[i] = -1;
		}

		bzero(buf_user, BUFFER + 1);
		bzero(buf_result, BUFFER + 1);
		//fgets(buf_user,BUFFER,fp_user);
		//fgets(buf_taskId,10,fp_user);

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

		if(strcmp(buf_temp, "QUIT") == 0)
		{
			break;
		}

		strcpy(buf_user, buf_temp);
		buf_taskId = buf_temp[j-1];
		taskId = -1;
		cur_line = 1; //初始化当前任务行
		//buf_user[strlen(buf_user)-1]='\0';
		strcpy(buf_result, buf_temp);
		strcat(buf_result, "_result.txt");
		taskId = buf_taskId - 48;

		if(taskId == -1)
		{
			printf("fail to read taskId!\n");
			shmctl(segment_id, IPC_RMID, NULL);
			msgctl(msgid, IPC_RMID, (struct msqid_ds*)0);
			//fclose(fp_user);
			exit(1);
		}

		bzero(tempname, 1000);
		strcat(tempname, server_webfiles_root);
		strcat(tempname, task_Location);
		strcat(tempname, buf_user);
		strcat(tempname, ".txt");
		printf("current task:%s\n", tempname);

		if((fp_task = fopen(tempname, "rt+")) == NULL)
		{
			printf("fail to open task file!\n");
			shmctl(segment_id, IPC_RMID, NULL);
			msgctl(msgid, IPC_RMID, (struct msqid_ds*)0);
			myfree(buf_user);
			myfree(temptask);
			myfree(tempname);
			myfree(buf_result);
			//fclose(fp_user);
			exit(1);
		}

		/*if((fp_result=fopen(buf_result,"wa+"))==NULL)
		{
			printf("fail to open result file!\n");
			shmctl(segment_id,IPC_RMID,NULL);
			fclose(fp_user);
			fclose(fp_task);
			exit(1);
		}*/
		if(!fstat(fileno(fp_task), &sb) && sb.st_size > 0)
		{
			//printf("get task size: %d\n",sb.st_size);
			tasklist.__item.__ptr = (unsigned char *)fp_task;
			tasklist.__item.__size = sb.st_size;
			tasklist.__item.id = NULL;
			tasklist.__item.type = "text/plain";
			//tasklist.__item.options=soap_dime_option(&soap,0,"tasklist");
		}
		else
		{
			printf("fstat error!\n");
			shmctl(segment_id, IPC_RMID, NULL);
			msgctl(msgid, IPC_RMID, (struct msqid_ds*)0);
			//fclose(fp_user);
			fclose(fp_task);
            myfree(buf_user);
			myfree(temptask);
			myfree(tempname);
			myfree(buf_result);
			exit(1);
		}

		tot_line = 0;

		while(!feof(fp_task))//读取任务行数
		{
			bzero(temptask, 200);
			fgets(temptask, 200, fp_task); //任务文件每一行最多60个字符

			if(strlen(temptask) < 2)
			{
				break;//there is something
			}

			tot_line++;
		}

		printf("tot_line: %d\n", tot_line);
		rewind(fp_task);//重置任务文件指针

		shared_memory = (char *)shmat(segment_id, NULL, 0);

		if(!shared_memory)
		{
			printf("shmat error in father!\n");
			shmctl(segment_id, IPC_RMID, NULL);
			msgctl(msgid, IPC_RMID, (struct msqid_ds*)0);
			myfree(buf_user);
			myfree(temptask);
			myfree(tempname);
			myfree(buf_result);
			exit(1);
		}

		semid = sem_creat(key); //使用共享内存
		p(semid);//增加信号量，锁定共享内存
		shmctl(segment_id, IPC_STAT, &shmbuffer);
		segment_size = shmbuffer.shm_segsz;
		sprintf(shared_memory, "%d", cur_line); //向共享内存写入当前任务行,1,初始化
		//printf("%s\n",shared_memory);
		v(semid);//减少信号量，解锁共享内存
		shmdt(shared_memory);//释放共享内存
        while (1)
        {

            cur_client = countClientNumber(server_webfiles_avaclientinfo);
            if (cur_client == 0)
            {
                printf("no available clients, waiting.....");
                sleep(10);
                continue;
            }
            printf("OK\ncurrent available client: %d\n", cur_client);

            for (i = 0; i < cur_client; i++)
            {
                flag[i] = 1;
            //retLoc[i].retlocation=(char *)malloc(BUFFER+1);
            }
            break;
        }

		for(i = 0; i < cur_client; i++)
		{

			bzero(tempname, 1000);
			strcat(tempname, server_webfiles_root);
			strcat(tempname, task_Location);
			strcat(tempname, buf_user);
			strcat(tempname, ".txt");

			if((fp_task = fopen(tempname, "rt+")) == NULL) //打开任务文件
			{
				printf("fail to open task file!\n");
				shmctl(segment_id, IPC_RMID, NULL);
				msgctl(msgid, IPC_RMID, (struct msqid_ds*)0);
				myfree(buf_user);
                myfree(temptask);
                myfree(tempname);
                myfree(buf_result);
				//fclose(fp_user);
				exit(1);
			}

			/*if((fp_result=fopen(buf_result,"wa+"))==NULL)
			{
				printf("fail to open result file!\n");
				shmctl(segment_id,IPC_RMID,NULL);
				fclose(fp_user);
				fclose(fp_task);
				exit(1);
			}*/
			if(!fstat(fileno(fp_task), &sb) && sb.st_size > 0) //初始化dime设置
			{
				//printf("get task size: %d\n",sb.st_size);
				tasklist.__item.__ptr = (unsigned char *)fp_task;
				tasklist.__item.__size = sb.st_size;
				tasklist.__item.id = NULL;
				tasklist.__item.type = "text/plain";
				//tasklist.__item.options=soap_dime_option(&soap,0,"tasklist");
			}

			pid[i] = fork();

			if(pid[i] < 0)
			{
				printf("error in fork!\n");
				//fclose(fp_user);
				//fclose(fp_result);
				myfree(buf_user);
                myfree(temptask);
                myfree(tempname);
                myfree(buf_result);
				exit(1);
			}
			else if(pid[i] == 0)
			{
				//解决buf_user的问题可以使得这一块代码全部移到外面
				bzero(tempname, 1000);
				strcat(tempname, server_webfiles_root);
				strcat(tempname, task_Location);
				strcat(tempname, buf_user);
				strcat(tempname, ".txt");

				if((fp_task = fopen(tempname, "rt+")) == NULL) //子进程中重新打开一次任务文件，似乎没有必要
				{
					printf("fail to open task file!\n");
					shmctl(segment_id, IPC_RMID, NULL);
					myfree(buf_user);
                    myfree(temptask);
                    myfree(tempname);
                    myfree(buf_result);
					//fclose(fp_user);
					exit(1);
				}

				int state = 1;
				int start;
				int end;
				int line;
				int result;
                int childpid;
				int failNum = 0;
				char *filename;
				char *temptemp = NULL;
				FILE *fp_result;
				FILE *fp_fail;
				struct ns2__soap_string taskline;
				taskline.str = (char *)malloc(1024);


				//state=symtab[i].funcptr(&soap,NULL,NULL,&tasklist,&receivetasklistresponse);//向当前客户端传送任务文件
				state = symtab[0].funcptr(&soap, cli_ip_port[i], NULL, &tasklist, &receivetasklistresponse);

				if(state == SOAP_OK)
				{
					printf("task sent to client %d successful!\n", i);
				}
				else
				{
					printf("task sent to client %d failed!\n", i);
					myfree(buf_user);
                    myfree(temptask);
                    myfree(tempname);
                    myfree(buf_result);
					exit(1);
				}

				shared_memory = (char *)shmat(segment_id, NULL, 0); //申请链接共享内存

				if(!shared_memory)
				{
					printf("shmat error in child %d!\n", i);
					myfree(buf_user);
                    myfree(temptask);
                    myfree(tempname);
                    myfree(buf_result);
					exit(1);
				}
				filename = (char *)malloc(100);
				bzero(filename, 100);
				sprintf(filename, "%s_client%d_result.txt", buf_user, i);
				bzero(tempname, 1000);
				strcat(tempname, server_webfiles_root);
				strcat(tempname, task_result_location);
				strcat(tempname, filename);
				fp_result = fopen(tempname, "wt+"); //创建当前客户端成功结果文件

				if(fp_result == NULL)
				{
					printf("creat result file error in client %d\n", i);
					myfree(buf_user);
                    myfree(temptask);
                    myfree(tempname);
                    myfree(buf_result);
                    myfree(filename);
					exit(1);
				}

				bzero(filename, 100);
				sprintf(filename, "%s_client%d_result_fail.txt", buf_user, i);
				bzero(tempname, 1000);
				strcat(tempname, server_webfiles_root);
				strcat(tempname, task_result_location);
				strcat(tempname, filename);
				fp_fail = fopen(tempname, "wt+"); //创建当前客户端失败结果文件

				if(fp_fail == NULL)
				{
					printf("creat fail file erro in client %d\n", i);
					fclose(fp_result);
					myfree(buf_user);
                    myfree(temptask);
                    myfree(tempname);
                    myfree(buf_result);
                    myfree(filename);
					exit(1);
				}
				/*bzero(tempname, 1000);
				sprintf(tempname, "%d", taskId);
                fputs(tempname, fp_fail);
                fputs("\n", fp_fail);*/
				while(1)
				{
					bzero(taskline.str, 1024);
					cur_line = atoi(shared_memory);

					//printf("%d %d\n",cur_line,tot_line);
					if(cur_line > tot_line)break;

					start = cur_line;
					cur_line += cli_core_num[i];
					end = cur_line - 1;
					printf("client %d is doing the following line:%d to %d\n", i, start, end);

					p(semid);//改变共享内存中当前任务行数的值
					bzero(shared_memory, strlen(shared_memory));
					sprintf(shared_memory, "%d", cur_line);
					v(semid);

					if(end > tot_line)
					{
						sprintf(taskline.str, "%d %d", start, tot_line);
					}
					else
					{
						sprintf(taskline.str, "%d %d", start, end);
					}

					taskline.size = strlen(taskline.str);
					//result=symtab[cur_client+i].funcptr(&soap,NULL,NULL,taskId,&taskline,&tasksolveresponse);//传给当前客户端需要执行的任务行

					if(symtab[1].funcptr(&soap, cli_ip_port[i], NULL, taskId, &taskline, &tasksolveresponse) == SOAP_OK)
					{
					    printf("result:%s\n", tasksolveresponse.ret->str);
						fputs(tasksolveresponse.ret->str, fp_result);
					}
					else
					{
                        soap_print_fault(&soap, stderr);
                        for (line = start; line <= end; line++)
                        {
                            temptemp = getTaskByLine(buf_user, line);
                            if ((temptemp != NULL) && (strlen(temptemp) > 14))
                            {
                                fputs(temptemp+14, fp_fail);
                                fputs("\n", fp_fail);
                            }
                            myfree(temptemp);
                        }
						failNum++;
					}
                    if(failNum > 2)
                    {
                        printf("3 lines failed to get result from client %d!\nthis client will be aborted\n", i);
                        break;
                    }
					memset(&tasksolveresponse, 0, sizeof(tasksolveresponse));
				}

				if(failNum > 0)
				{
					printf("%d line(s) failed to get result from client %d!\n", failNum, i);
				}

				shmdt(shared_memory);
				fclose(fp_result);
				fclose(fp_fail);
				fclose(fp_task);
				myfree(buf_user);
                myfree(temptask);
                myfree(tempname);
                myfree(buf_result);
                myfree(filename);
				exit(0);
			}
			else
			{
				//signal(SIGCHLD,SIG_IGN);
			}
		}

		for(i = 0; i < cur_client; i++)
		{
			waitpid(pid[i], &status[i], 0);
		}
		fclose(fp_task);
        printf("doing result deal of %s\n", buf_user);
        if(resultDeal(buf_user, status) != 0)
        {
            printf("resultDeal fail of %s\n", buf_user);
            continue;
        }
        if(checkResult(buf_user) != 0)
        {
            printf("checkResult fail of %s\n", buf_user);
            continue;
        }
        if(returnResult(buf_user) != 0)
        {
            printf("returnResult fail of %s\n", buf_user);
        }
		//fclose(fp_result);
	}
    myfree(buf_user);
    myfree(temptask);
    myfree(tempname);
    myfree(buf_result);
	shmctl(segment_id, IPC_RMID, NULL); //释放共享内存
	msgctl(msgid, IPC_RMID, (struct msqid_ds*)0);
	del_sem(semid);//删除信号量
	printf("done!\n");
	soap_destroy(&soap);
	soap_end(&soap);
	soap_done(&soap);
	//fclose(fp_user);
	return 0;
}



