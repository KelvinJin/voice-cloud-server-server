#include "soapH.h"
#include "Client_USCORE1.nsmap"
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <signal.h> 
#include <string.h>
#define MAX_TIME 5
#define max_client 10
#define BUFFER 63
#define task_Location "/var/www/TaskList/"
#define task_result_location "/var/www/TaskResult/"
#define task_final_result_location "/var/www/TaskResult/FinalResult/"
int cli_core_num[max_client];
struct taskrecfunc
{
	char *funname;
	int (*funcptr)();
}symtab[]={
	{"soap_call_ns1__ReceiveTaskList",soap_call_ns1__ReceiveTaskList},
	{"soap_call_ns2__ReceiveTaskList",soap_call_ns2__ReceiveTaskList},
	{"soap_call_ns3__ReceiveTaskList",soap_call_ns3__ReceiveTaskList},
	//{"soap_call_ns4__ReceiveTaskList",soap_call_ns4__ReceiveTaskList},
	//{"soap_call_ns5__ReceiveTaskList",soap_call_ns5__ReceiveTaskList},
	//{"soap_call_ns6__ReceiveTaskList",soap_call_ns6__ReceiveTaskList},
	//{"soap_call_ns7__ReceiveTaskList",soap_call_ns7__ReceiveTaskList},
	//{"soap_call_ns8__ReceiveTaskList",soap_call_ns8__ReceiveTaskList},
	//{"soap_call_ns9__ReceiveTaskList",soap_call_ns9__ReceiveTaskList},
	//{"soap_call_ns10__ReceiveTaskList",soap_call_ns10__ReceiveTaskList},
	{"soap_call_ns1__TaskSolve",soap_call_ns1__TaskSolve},
	{"soap_call_ns2__TaskSolve",soap_call_ns2__TaskSolve},
	{"soap_call_ns3__TaskSolve",soap_call_ns3__TaskSolve},
	//{"soap_call_ns4__TaskSolve",soap_call_ns4__TaskSolve},
	//{"soap_call_ns5__TaskSolve",soap_call_ns5__TaskSolve},
	//{"soap_call_ns6__TaskSolve",soap_call_ns6__TaskSolve},
	//{"soap_call_ns7__TaskSolve",soap_call_ns7__TaskSolve},
	//{"soap_call_ns8__TaskSolve",soap_call_ns8__TaskSolve},
	//{"soap_call_ns9__TaskSolve",soap_call_ns9__TaskSolve},
	//{"soap_call_ns10__TaskSolve",soap_call_ns10__TaskSolve},
	//{"soap_call_ns1__isBusy",soap_call_ns1__isBusy},
	//{"soap_call_ns2__isBusy",soap_call_ns2__isBusy},
	//{"soap_call_ns3__isBusy",soap_call_ns3__isBusy},
};
/*struct retLocation
{
	char *retlocation;	
}*/
struct msgtype {
    long mtype;
    char buffer[BUFFER+1];
}; 
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
    sem.val=0;
    semid=semget(key, 1, IPC_CREAT|0666);

    if(semid==-1)
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
   struct sembuf sops = {0,+1,SEM_UNDO};
    return (semop(semid, &sops, 1));
}

/*信号量的v操作,使得信号量的值减1*/
int v(int semid)
{
    struct sembuf sops = {0,-1,SEM_UNDO};
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
int countClientNumber()
{	
	FILE *fp_client;
	char *buf_client;
	int temp=0;

	fp_client=fopen("/home/owenwj/prog/client/Clientinfo.txt","rt+");
	if(fp_client==NULL)
	{
		printf("fail to open Clientinfo.txt\n");
		exit(1);
	}
	buf_client=(char *)malloc(BUFFER);
	while(!feof(fp_client))
	{
		bzero(buf_client,BUFFER);
		fgets(buf_client,BUFFER,fp_client);
		if(strlen(buf_client)==1)
		{
			break;
		}
		printf("client %d's IP : %s",temp,buf_client);
		bzero(buf_client,BUFFER);
		fgets(buf_client,BUFFER,fp_client);
		printf("client %d's PORT : %s",temp,buf_client);
		bzero(buf_client,BUFFER);
		fgets(buf_client,BUFFER,fp_client);
		printf("client %d's CORENUM : %s",temp,buf_client);
		cli_core_num[temp]=atoi(buf_client);
		temp++;
	}
	fclose(fp_client);
	return temp;
}

void resultDeal(char *filename,int status[])
{	
	struct msgtype msg;
	FILE *fp_result_in;
	FILE *fp_result_out;
	FILE *fp_result_out_fail;
	char *tempname;
	char *append;
	char *temp;
	int i;
	key_t key;
	int msgid;
	msg.mtype=2;
	strcpy(msg.buffer,filename);
	tempname=(char *)malloc(BUFFER+1);
	append=(char *)malloc(BUFFER+1);
	temp=(char *)malloc(1000);
	bzero(tempname,BUFFER+1);
	strcat(tempname,task_final_result_location);
	strcat(tempname,filename);
	strcat(tempname,"_result.txt");
	if((fp_result_out=fopen(tempname,"wt+"))==NULL)
	{
		printf("result file of %s cannot creat!\n",filename);
	}
	bzero(tempname,BUFFER+1);
	strcat(tempname,task_final_result_location);
	strcat(tempname,filename);
	strcat(tempname,"_result_fail.txt");
	if((fp_result_out_fail=fopen(tempname,"wt+"))==NULL)
	{
		printf("result fail file of %s cannot creat!\n",filename);
	}
	for(i=0;i<max_client;i++)
	{
		if(status[i]==0)
		{
			bzero(tempname,BUFFER+1);
			bzero(append,BUFFER+1);
			strcat(tempname,task_result_location);
			strcat(tempname,filename);
			sprintf(append,"_client%d_result.txt",i);
			strcat(tempname,append);
			if((fp_result_in=fopen(tempname,"rt+"))==NULL)
			{
				printf("result file of client %d not find!\n",i);
			}
			while(!feof(fp_result_in))
			{
			 	bzero(temp,1000);
				fgets(temp,1000,fp_result_in);
				fputs(temp,fp_result_out);
			}
			fclose(fp_result_in);
			bzero(tempname,BUFFER+1);
			bzero(append,BUFFER+1);
			strcat(tempname,task_result_location);
			strcat(tempname,filename);
			sprintf(append,"_client%d_result_fail.txt",i);
			strcat(tempname,append);
			if((fp_result_in=fopen(tempname,"rt+"))==NULL)
			{
				printf("result fail file of client %d not find!\n",i);
			}
			while(!feof(fp_result_in))
			{
			 	bzero(temp,1000);
				fgets(temp,1000,fp_result_in);
				fputs(temp,fp_result_out_fail);
			}
			fclose(fp_result_in);
		}	
	}
	fclose(fp_result_out);
	fclose(fp_result_out_fail);
	bzero(tempname,BUFFER+1);
	strcat(tempname,task_Location);
	strcat(tempname,filename);
	strcat(tempname,".txt");
	printf("tasklist:%s\n",tempname);
	key=ftok(tempname,'a');
	if((msgid=msgget(key, 0777|IPC_CREAT))==-1)
    	{
        	printf("Creat Message Error of%s:\n",filename);
        	exit(1);
    	} 
	msgsnd(msgid,&msg,sizeof(struct msgtype),0);
}

int main()
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
	int cur_client;
	int status[max_client];//子进程是否正常退出
	int dead_client=0;

	//FILE *fp_user;
	FILE *fp_task;
	FILE *fp_msgid;
	//FILE *fp_result;	

	char *buf_user;
	char *buf_result;
	char buf_taskId;
	char buf_temp[BUFFER];
	char *buf_client;
	char *temptask;
	char *shared_memory;
	char *tempname;

	struct msgtype msg;
	struct ns1__data tasklist;
	struct ns1__TaskSolveResponse tasksolveresponse;
	struct ns1__ReceiveTaskListResponse receivetasklistresponse;
	struct shmid_ds shmbuffer;
	struct soap soap;
	struct stat sb;
	struct soap_multipart *attachment;
	//struct retLocation retLoc[max_client]

	cur_client=countClientNumber();
	printf("current available client: %d\n",cur_client);
	for (i=0;i<cur_client;i++)
	{
		flag[i]=1;
		//retLoc[i].retlocation=(char *)malloc(BUFFER+1);
	}
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
	soap.connect_timeout=MAX_TIME;
	soap.send_timeout=MAX_TIME;
	soap.recv_timeout=MAX_TIME;

	shared_memory=(char *)malloc(4);
	bzero(shared_memory,4);
	buf_user=(char *)malloc(BUFFER+1);
	buf_result=(char *)malloc(BUFFER+1);
	tempname=(char *)malloc(1000);
	//buf_taskId=(char *)malloc(BUFFER);
	temptask=(char *)malloc(60);

	key=ftok("/var/www/uploadify/",'a');
	if(key==-1)
	{
		printf("ftok error!\n");
		exit(1);
	}
	//共享内存创建
	segment_id=shmget(key,getpagesize(),IPC_CREAT|IPC_EXCL|S_IRUSR|S_IWUSR|S_IROTH|S_IWOTH);
	if(segment_id==-1)
	{
		printf("create shared memory error!\n");
		exit(1);
	}
	//消息队列创建
	if((msgid=msgget(key, 0777|IPC_CREAT))==-1)
    	{
        	printf("Creat Message Error:\n");
        	exit(1);
    	} 
	//msgid写进文本
	printf("msgid:%d\n",msgid);
	while(1)
	{
		memset(&msg,0,sizeof(struct msgtype)); 
		msgrcv(msgid,&msg,sizeof(struct msgtype),1,0);
		memset(&tasklist,0,sizeof(tasklist));
		for(i=0;i<max_client;i++)
		{
			status[i]=-1;
		}
		bzero(buf_user,BUFFER+1);
		bzero(buf_result,BUFFER+1);
		//fgets(buf_user,BUFFER,fp_user);
		//fgets(buf_taskId,10,fp_user);

		for(i=0,j=0;i<strlen(msg.buffer);i++)
		{
			if(msg.buffer[i]=='\"')
			{
				i++;
				while(msg.buffer[i]!='\"')
				{
					buf_temp[j]=msg.buffer[i];
					j++;
					i++;
				}
				buf_temp[j]='\0';
				break;
			}	
		}
		if(strcmp(buf_temp,"QUIT")==0)
		{
			break;
		}
		strcpy(buf_user,buf_temp);
		buf_taskId=buf_temp[j-1];
		taskId=-1;
		cur_line=1;//初始化当前任务行
		//buf_user[strlen(buf_user)-1]='\0';
		printf("%s\n",buf_user);
		strcpy(buf_result,buf_temp);
		strcat(buf_result,"_result.txt");
		taskId=buf_taskId-48;
		if(taskId==-1)
		{
			printf("fail to read taskId!\n");
			shmctl(segment_id,IPC_RMID,NULL);
			msgctl(msgid,IPC_RMID,(struct msqid_ds*)0);
			//fclose(fp_user);
			exit(1);
		}
		bzero(tempname,1000);
		strcat(tempname,task_Location);
		strcat(tempname,buf_user);
		strcat(tempname,".txt");
		printf("tempname:%s\n",tempname);
		if((fp_task=fopen(tempname,"rt+"))==NULL)
		{
			printf("fail to open task file!\n");
			shmctl(segment_id,IPC_RMID,NULL);
			msgctl(msgid,IPC_RMID,(struct msqid_ds*)0);
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
		if(!fstat(fileno(fp_task),&sb)&&sb.st_size>0)
		{
			//printf("get task size: %d\n",sb.st_size);
			tasklist.__item.__ptr=(unsigned char *)fp_task;
			tasklist.__item.__size=sb.st_size;
			tasklist.__item.id=NULL;
			tasklist.__item.type="text/plain";
			//tasklist.__item.options=soap_dime_option(&soap,0,"tasklist");
		}
		else
		{
			printf("fstat error!\n");
			shmctl(segment_id,IPC_RMID,NULL);
			msgctl(msgid,IPC_RMID,(struct msqid_ds*)0);
			//fclose(fp_user);
			fclose(fp_task);
			exit(1);
		}
		tot_line=-1;
		while(!feof(fp_task))//读取任务行数
		{
			bzero(temptask,60);
			fgets(temptask,60,fp_task);//任务文件每一行最多60个字符
			tot_line++;
		}
		printf("tot_line: %d\n",tot_line);
		rewind(fp_task);//重置任务文件指针

		shared_memory=(char *)shmat(segment_id,NULL,0);
		if(!shared_memory)
		{
			printf("shmat error in father!\n");
			shmctl(segment_id,IPC_RMID,NULL);
			msgctl(msgid,IPC_RMID,(struct msqid_ds*)0);
			exit(1);
		}
		semid=sem_creat(key);//使用共享内存
		p(semid);//增加信号量，锁定共享内存
		shmctl(segment_id,IPC_STAT,&shmbuffer);
		segment_size=shmbuffer.shm_segsz;
		sprintf(shared_memory,"%d",cur_line);//向共享内存写入当前任务行,1,初始化
		//printf("%s\n",shared_memory);
		v(semid);//减少信号量，解锁共享内存
		shmdt(shared_memory);//释放共享内存


		for(i=0;i<cur_client;i++)
		{
			bzero(tempname,1000);
			strcat(tempname,task_Location);
			strcat(tempname,buf_user);
			strcat(tempname,".txt");
			if((fp_task=fopen(tempname,"rt+"))==NULL)//打开任务文件
			{
				printf("fail to open task file!\n");
				shmctl(segment_id,IPC_RMID,NULL);
				msgctl(msgid,IPC_RMID,(struct msqid_ds*)0);
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
			if(!fstat(fileno(fp_task),&sb)&&sb.st_size>0)//初始化dime设置
			{
				//printf("get task size: %d\n",sb.st_size);
				tasklist.__item.__ptr=(unsigned char *)fp_task;
				tasklist.__item.__size=sb.st_size;
				tasklist.__item.id=NULL;
				tasklist.__item.type="text/plain";
				//tasklist.__item.options=soap_dime_option(&soap,0,"tasklist");
			}
			pid[i]=fork();
			if(pid[i]<0)
			{
				printf("error in fork!\n");
				//fclose(fp_user);
				//fclose(fp_result);
				exit(1);
			}
			else if(pid[i]==0)
			{	
				//解决buf_user的问题可以使得这一块代码全部移到外面
				bzero(tempname,1000);				
				strcat(tempname,task_Location);
				strcat(tempname,buf_user);
				strcat(tempname,".txt");
				if((fp_task=fopen(tempname,"rt+"))==NULL)//子进程中重新打开一次任务文件，似乎没有必要
				{
				printf("fail to open task file!\n");
				shmctl(segment_id,IPC_RMID,NULL);
				//fclose(fp_user);
				exit(1);
				}
				int state=1;
				int start;
				int end;
				int result;
				int failNum=0;
				char *filename;
				FILE *fp_result;
				FILE *fp_fail;
				struct ns1__soap_string taskline;
				taskline.str=(char *)malloc(1024);
				filename=(char *)malloc(100);

				state=symtab[i].funcptr(&soap,NULL,NULL,&tasklist,&receivetasklistresponse);//向当前客户端传送任务文件
				//state=symtab[i+1].funcptr(&soap,NULL,NULL,&tasklist,&receivetasklistresponse);
				if(state==SOAP_OK)
				{
					printf("task sent to client %d successful!\n",i);
				}
				else
				{
					printf("task sent to client %d failed!\n",i);
					exit(1);
				}

				shared_memory=(char *)shmat(segment_id,NULL,0);//申请链接共享内存
				if(!shared_memory)
				{
					printf("shmat error in child %d!\n",i);
					exit(1);
				}

				bzero(filename,100);
				sprintf(filename,"%s_client%d_result.txt",buf_user,i);
				bzero(tempname,1000);
				strcat(tempname,task_result_location);
				strcat(tempname,filename);
				fp_result=fopen(tempname,"wt+");//创建当前客户端成功结果文件
				if(fp_result==NULL)
				{
					printf("creat result file error in client %d\n",i);
					exit(1);
				}

				bzero(filename,100);
				sprintf(filename,"%s_client%d_result_fail.txt",buf_user,i);
				bzero(tempname,1000);
				strcat(tempname,task_result_location);
				strcat(tempname,filename);
				fp_fail=fopen(tempname,"wt+");//创建当前客户端失败结果文件
				if(fp_fail==NULL)
				{
					printf("creat fail file erro in client %d\n",i);
					fclose(fp_result);
					exit(1);
				}
	
				while(1)
				{
					bzero(taskline.str,1024);				
					cur_line=atoi(shared_memory);
					if(cur_line>tot_line)break;
					start=cur_line;
					cur_line+=cli_core_num[i];
					end=cur_line-1;
					printf("client %d is doing the following line:%d to %d\n",i,start,end);

					p(semid);//改变共享内存中当前任务行数的值
					bzero(shared_memory,strlen(shared_memory));
					sprintf(shared_memory,"%d",cur_line);
					v(semid);

					if(end>tot_line)
					{
						sprintf(taskline.str,"%d %d",start,tot_line);
					}
					else
					{
						sprintf(taskline.str,"%d %d",start,end);
					}

					taskline.size=strlen(taskline.str);
					result=symtab[cur_client+i].funcptr(&soap,NULL,NULL,taskId,&taskline,&tasksolveresponse);//传给当前客户端需要执行的任务行
					printf("result:%s\n",tasksolveresponse.ret->str);
					if(result==SOAP_OK)
					{
						fputs(tasksolveresponse.ret->str,fp_result);
					}
					else
					{
						fputs(taskline.str,fp_fail);
						fputs("\n",fp_fail);
						failNum++;
					}

					memset(&tasksolveresponse,0,sizeof(tasksolveresponse));
				}
				if(failNum>0)
				{
					printf("%d line(s) failed to get result from client %d!\n",failNum,i);
				}
				shmdt(shared_memory);
				fclose(fp_result);
				fclose(fp_fail);
				fclose(fp_task);
				exit(0);
			}
			else
			{
				//signal(SIGCHLD,SIG_IGN);
			}
		}
		for(i=0;i<cur_client;i++)
		{
			waitpid(pid[i],&status[i],0);
		}
		temppid=fork();
		if(temppid<0)
		{
			printf("result back fork fail of %s\n",buf_user);
		}
		else if(temppid==0)
		{
			resultDeal(buf_user,status);
			exit(0);
		}
		else
		{
		}
		//fclose(fp_result);
		fclose(fp_task);
	}
	shmctl(segment_id,IPC_RMID,NULL);//释放共享内存
	msgctl(msgid,IPC_RMID,(struct msqid_ds*)0);
	del_sem(semid);//删除信号量
	printf("done!\n");
	soap_destroy(&soap);
	soap_end(&soap);
	soap_done(&soap);
	//fclose(fp_user);
	return 0;
}



