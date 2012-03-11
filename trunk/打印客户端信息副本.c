//打印可用客户端
int countClientNumber()
{	
	File* fp_client;
	char* buf_client;
	int temp=0;

	fp_client=fopen("/home/owenwj/prog/client/Clientinfo.txt","rt+");
	if(fp_client==NULL)
	{
		printf("fail to open Clientinfo.txt\n");
		exit(1);
	}
	buf_client=(char *)malloc(MAX_BUFFER);
	while(!feof(fp_client))
	{
		bzero(buf_client,MAX_BUFFER);
		fgets(buf_client,MAX_BUFFER,fp_client);
		if(strlen(buf_client)==1)
		{
			break;
		}
		printf("client %d's IP : %s",cur_client,buf_client);
		bzero(buf_client,MAX_BUFFER);
		fgets(buf_client,MAX_BUFFER,fp_client);
		printf("client %d's PORT : %s",cur_client,buf_client);
		bzero(buf_client,MAX_BUFFER);
		fgets(buf_client,MAX_BUFFER,fp_client);
		printf("client %d's CORENUM : %s",cur_client,buf_client);
		cli_core_num[cur_client]=atoi(buf_client);
		temp++;
	}
	fclose(fp_client);
	return temp;
}
