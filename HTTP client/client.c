#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>

#define MAX_STRING 128
#define TIME_PORMAT_STRING 3
#define MAX_TIME 128
#define RFC1123FMT "%A, %D %B %Y %H:%M:%S GMT"

//********function*******************************************************

void catString(char* enteruser,char* path,char* version,char*host,char*hostName,char* space,char* time_interval,char*timebuf,char*close_connection,int starTime)
{//combining string to create request for the server
	strcat(enteruser,path);
	strcat(enteruser,version);
	strcat(enteruser,host);
	strcat(enteruser,hostName);
	if(starTime==0)
	{
		strcat(enteruser,space);
		strcat(enteruser,close_connection);				
	}
	else
	{	
		strcat(enteruser,space);
		strcat(enteruser,time_interval);
		strcat(enteruser,timebuf);
		strcat(enteruser,space);
		strcat(enteruser,close_connection);
		}

}
//-------------------------------------------------------------------------------

void free_all(char* url,char* protocol, char* hostName,char*port,char*path)
{//free string and array after malloc
	free(url);
	free(protocol);
	free(hostName);
	free(port);
	free(path);
	
}

//-------------------------------------------------------------------------------

int *timeFormat(char *argv[],int i,int size,char* url,char* protocol, char* hostName,char*port,char*path)
{//change the time format when user press -d dd:hh:mm
	int j=0;
	int k=0;
	static int time_number[TIME_PORMAT_STRING];
	char *_time[size];

	_time[j]=strtok(argv[i+1],":"); 
	while(_time[j]!=NULL)//cut ':' from the string dd:hh:mm and put them separately in another string
	{
		j++;
		_time[j]=strtok(NULL,":");
	}

	_time[j]='\0';
	if((j)!=TIME_PORMAT_STRING)//time format need to be with only 2- ':'
	{
		printf("worng input\n");
		free_all(url,protocol,hostName,port,path);
		exit(0);
	}

	for(i=0;i<j;i++)//time format need to only with numbers
	{
		for(k=0;k<strlen(_time[i]);k++)
			if(!isdigit(_time[i][k]))
			{
				printf("worng input\n");	
				free_all(url,protocol,hostName,port,path);
				exit(0);
			}
	}
	for(k=0;k<TIME_PORMAT_STRING;k++)//change the number from string to int 
	{
		time_number[k]=atoi(_time[k]);
	}

	return time_number;
}
//-------------------------------------------------------------------------------

void change_url(char *argv[],char* url,int j,int num)//move forward the url after use
{
		url[0]='\0';	
		argv[j]=num+argv[j];
		strcat(url,argv[j]); 

}
//-------------------------------------------------------------------------------

void split_url(char *argv[],char* url,char* protocol, char* hostName,char*port,char*path,int j)
{
	if(strlen(url)==0)//must enter url
	{
		printf("Usage: client [-h] [-d <time-interval>] <URL>\n");
		free_all(url,protocol,hostName,port,path);
		exit(0);
	}
	protocol[0]='\0';
	strncat(protocol,url,7);
	if(strcmp(protocol,"http://")!=0)//the protocol must be "http://"
	{
		printf("worng input\n");
		free_all(url,protocol,hostName,port,path);
		exit(0);
	}
	else
	{	
		change_url(argv,url,j,strlen(protocol));
		char *ptr1=strchr(url,':');//user enter port	
		if(ptr1!=NULL)
		{
			hostName[0]='\0';
			strncat(hostName,url,(int)(ptr1-url));//cat part from string to certain size
			change_url(argv,url,j,strlen(hostName)+1);
			if(strchr(url,'/')!=NULL)
			{
				port[0]='\0';
				char *ptr1=strchr(url,'/');
				strncat(port,url,(int)(ptr1-url));
				if(strcmp(port,"")==0)//after ':' user must enter port number
				{
					printf("wrong input\n");	
					free_all(url,protocol,hostName,port,path);
					exit(0);
				}
				else
				{
					int k;
					for(k=0;k<strlen(port);k++)//port must to be a number
						if(!isdigit(port[k]))
						{
							printf("wrong input\n");	
							free_all(url,protocol,hostName,port,path);
							exit(0);
						}
				}
					change_url(argv,url,j,strlen(port));			
			}
		
		}
		else if(strchr(url,'/')!=NULL)//default port<-80
		{
			hostName[0]='\0';
			port[0]='\0';
			char *ptr1=strchr(url,'/');
			strncat(hostName,url,(int)(ptr1-url));
			strcpy(port,"80");
			change_url(argv,url,j,strlen(hostName));
	
		}
		else
		{
			printf("wrong input\n");
			free_all(url,protocol,hostName,port,path);
			exit(0);
		}
		strcpy(path,url);
		change_url(argv,url,j,strlen(path));
	}
}
//-------------------------------------------------------------------------------
//****************************end function***********************************************

//###########main###########################

int main(int argc, char *argv[])
{
	int sockfd;//for socket
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[MAX_STRING];//to read server answer
	int rc;

	int size=argc+1,index;

	for(index=0;index<argc;index++)//use for malloc
	{
	 size+=strlen(argv[index]);
	}

	char *url=(char*)malloc(sizeof(char)*size);
	if(url==NULL)
	{
		perror("malloc failed");	
		exit(0);
	}
	char *protocol=(char*)malloc(sizeof(char)*size);
	if(protocol==NULL)
	{
		perror("malloc failed");
		free(url);	
		exit(0);
	}
	char *hostName=(char*)malloc(sizeof(char)*size);
	if(hostName==NULL)
	{
		perror("malloc failed");
		free(url);
		free(protocol);	
		exit(0);
	}
	char *port=(char*)malloc(sizeof(char)*size);
	if(port==NULL)
	{
		perror("malloc failed");
		free(url);
		free(protocol);
		free(hostName);	
		exit(0);
	}
	char *path=(char*)malloc(sizeof(char)*size);
	if(path==NULL)
	{
		perror("malloc failed");
		free(url);
		free(protocol);
		free(hostName);	
		free(port);
		exit(0);
	}
	//request format
	char get[]="GET ";
	char head[]="HEAD ";
	char version[]=" HTTP/1.1\r\n";
	char host[]="Host: ";
	char time_interval[]="If-Modified-Since: ";
	char close_connection[]="Connection: close\r\n\r\n";
	char space[]="\r\n";

	//time
	time_t now;
	char timebuf[MAX_TIME];
	int day,hour,min;
	int size1;

	size+=(strlen(get)+strlen(head)+strlen(host)+strlen(time_interval)+MAX_TIME+strlen(version))+strlen(space)+argc;
	char *enteruser=(char*)malloc(sizeof(char)*size);
	if(enteruser==NULL)
	{
		perror("malloc failed");
		free_all(url,protocol,hostName,port,path);	
		exit(0);
	}
	
	int startHead=0;//flag for -h
	int starTime=0;//flag for -d
	int j=0;//index to argv
	int *ptr;//extract time

	if(argc>5)
	{
		printf("wrong input\n");
		free_all(url,protocol,hostName,port,path);
		exit(0);	
	}
	int i;
	for(i=1;i<argc;i++)
	{
		if(strcmp(argv[i],"-h")==0)// -h to head
		{
			startHead=1;
		}
		else if(strcmp(argv[i],"-d")==0)//-d for time
		{
			if(argv[i+1]==NULL)
			{
				printf("Usage: client [-h] [-d <time-interval>] <URL>\n");
				free_all(url,protocol,hostName,port,path);
				exit(0);
			}
			size1=strlen(argv[i+1]);
			ptr=timeFormat(argv,i,size1,url,protocol,hostName,port,path);
			starTime=1;
			i++;	
		}
		else
		{
			url[0]='\0';//take url from the user
			strcpy(url,argv[i]);
			j=i;
		}

	}
	if((startHead==0)&&(starTime==0)&&argc>2)
	{			
		printf("Usage: client [-h] [-d <time-interval>] <URL>\n");
		free_all(url,protocol,hostName,port,path);
		exit(0);
	}
	if((startHead==0)&&(starTime==1)&&argc>4)
	{
		printf("Usage: client [-h] [-d <time-interval>] <URL>\n");
		free_all(url,protocol,hostName,port,path);
		exit(0);
	}
	if((startHead==1)&&(starTime==0)&&argc>3)
	{
		printf("Usage: client [-h] [-d <time-interval>] <URL>\n");
		free_all(url,protocol,hostName,port,path);
		exit(0);
	}

	if(starTime==1)//change time format
	{
		day=ptr[0];
		hour=ptr[1];
		min=ptr[2];
		now=time(NULL);
		now=now-(day*24*3600+hour*3600*60+min*60);
		strftime(timebuf,sizeof(timebuf),RFC1123FMT,gmtime(&now));
	}
	split_url(argv,url,protocol,hostName,port,path,j);

	if(startHead==1)//format request with head
	{
		enteruser[0]='\0';
		strcat(enteruser,head);
		catString(enteruser,path,version,host,hostName,space,time_interval,timebuf,close_connection,starTime);
		printf("HTTP request=\n%s\nLEN= %lu\n",enteruser,strlen(enteruser));

		
	}
	else//format request with get
	{
		enteruser[0]='\0';
		strcat(enteruser,get);
		catString(enteruser,path,version,host,hostName,space,time_interval,timebuf,close_connection,starTime);
		printf("HTTP request=\n%s\nLEN= %lu\n",enteruser,strlen(enteruser));
	}
	

	//create soket
	int rd=0;
	int sizerd=0;
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd<0)
	{
		perror("socket failed");
		exit(0);	
	}
	server=gethostbyname(hostName);
	if(server==NULL)
	{
		herror("ERROR,no such host\n");
		exit(0);
	}

	serv_addr.sin_family=AF_INET;
	bcopy((char*)server->h_addr,(char*) &serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port=htons(atoi(port));//enter port

	rc=connect(sockfd ,(const struct sockaddr*)&serv_addr, sizeof(serv_addr));//connect with the socket
	if(rc<0)
	{
		perror("connect failed");
		exit(0);
	}

	rc=write(sockfd,enteruser,strlen(enteruser));//write to server
	if(rc<0)
	{
		perror("write failed");
		exit(0);
	}

	while((rd=read(sockfd,buffer,(sizeof(buffer))-1))!=0)//read from server
	{
		if(rd<0)
		{
			perror("read failed");
			exit(0);
		}
		printf("%s\n",buffer);
		sizerd+=rd;
		bzero(buffer,MAX_STRING);
	}
	shutdown(sockfd,SHUT_RDWR);
	close(sockfd);//close socket
	free_all(url,protocol,hostName,port,path);
	free(enteruser);
	printf("\nTotal received response bytes: %d\n",sizerd);
	
	return 0;
}


