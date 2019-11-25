#include "threadpool.h"
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/stat.h>
#include <netinet/in.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>
#include <time.h>  
#include <dirent.h>
#include <fcntl.h>


#define FIRST_LINE 4000
#define ENTITY_LINE 500

#define STATUS302 "302"
#define STATUS400 "400"
#define STATUS403 "403"
#define STATUS404 "404"
#define STATUS500 "500"
#define STATUS501 "501"
#define STATUS200 "200"

#define PHRASE302 "Found"
#define PHRASE400 "Bad Request"
#define PHRASE403 "Forbidden"
#define PHRASE404 "Not Found"
#define PHRASE500 "Internal Server Error"
#define PHRASE501 "Not supported"
#define PHRASE200 "Ok"

#define MAX_TIME 128
#define RFC1123FMT "%A, %D %B %Y %H:%M:%S GMT"

void error(char *msg)
{      
	perror(msg);      
	exit(1); 
}

void dir_content_body(char* answeRequest,char* name,char* file_name[],char* file_time[],char file_size[][22],int size)
{
	int i;
	strcat(answeRequest,"\r\n");
	strcat(answeRequest,"<HTML>");
	strcat(answeRequest,"\r\n");
	strcat(answeRequest,"<HEAD><TITLE>Index of ");
	strcat(answeRequest,name);
	strcat(answeRequest,"</TITLE></HEAD>\r\n");
	strcat(answeRequest,"\r\n");
	strcat(answeRequest,"<BODY>\r\n");
	strcat(answeRequest,"<H4>Index of ");
	strcat(answeRequest,name);
	strcat(answeRequest,"</H4>\r\n");
	strcat(answeRequest,"\r\n");
	strcat(answeRequest,"<table CELLSPACING=8>\r\n");
	strcat(answeRequest,"<tr><th>Name</th><th>Last Modified</th><th>Size</th></tr>\r\n");
	strcat(answeRequest,"\r\n");
	
	for(i=0;i<size;i++)
	{
		strcat(answeRequest,"<tr>\r\n");
		strcat(answeRequest,"\r\n");

		if(strcmp(file_size[i],"1")!=0)//file
		{

			strcat(answeRequest,"<td><A HREF=\"");
			strcat(answeRequest,file_name[i]);
			strcat(answeRequest,"\">");
			strcat(answeRequest,file_name[i]);
			strcat(answeRequest,"</A></td>");


		}
		else
		{

			strcat(answeRequest,"<td><A HREF=\"");
			strcat(answeRequest,file_name[i]);
			strcat(answeRequest,"/\">");
			strcat(answeRequest,file_name[i]);
			strcat(answeRequest,"</A></td>");
		}
		strcat(answeRequest,"<td>");
		strcat(answeRequest,file_time[i]);
		strcat(answeRequest,"</td><td>");
		if(strcmp(file_size[i],"1")!=0)
		{
			strcat(answeRequest,file_size[i]);
			
		}
		
		strcat(answeRequest,"</td>");

	}
	strcat(answeRequest,"\r\n");
	strcat(answeRequest,"</tr>\r\n");
	strcat(answeRequest,"\r\n");
	strcat(answeRequest,"</table>\r\n");
	strcat(answeRequest,"\r\n");
	strcat(answeRequest,"<HR>\r\n");
	strcat(answeRequest,"\r\n");
	strcat(answeRequest,"<ADDRESS>webserver/1.1</ADDRESS>\r\n");
	strcat(answeRequest,"\r\n");
	strcat(answeRequest,"</BODY></HTML>\r\n");
	
}

void internal_server_body(char* answeRequest)
{
	strcat(answeRequest,"\r\n");
	strcat(answeRequest,"<HTML><HEAD><TITLE><500 Internal Server Error</TITLE></HEAD>\r\n\r\n");
	strcat(answeRequest,"<BODY><H4>500 Internal Server Erro</H4>\r\n");
	strcat(answeRequest,"Some server side error.\r\n");
	strcat(answeRequest,"</BODY></HTML>\r\n");
}



void fordidden_body(char* answeRequest)
{
	strcat(answeRequest,"\r\n");
	strcat(answeRequest,"<BODY><H4>403 Forbidden</H4>\r\n");
	strcat(answeRequest,"Access denied.\r\n");
	strcat(answeRequest,"</BODY></HTML>\r\n");
}

void found_body(char *answeRequest)
{

	strcat(answeRequest,"\r\n");
	strcat(answeRequest,"<HTML><HEAD><TITLE>302 Found</TITLE></HEAD>\r\n");
	strcat(answeRequest,"<BODY><H4>302 Found</H4>\r\n");
	strcat(answeRequest,"Directories must end with a slash.\r\n");
	strcat(answeRequest,"</BODY></HTML>\r\n");
	strcat(answeRequest,"\r\n");
}

void not_found_body(char *answeRequest)
{
	strcat(answeRequest,"\r\n");
	strcat(answeRequest,"<HTML><HEAD><TITLE>404 Not Found</TITLE></HEAD>\r\n");
	strcat(answeRequest,"<BODY><H4>404 Not Found</H4>\r\n");
	strcat(answeRequest,"File not found.\r\n");
	strcat(answeRequest,"</BODY></HTML>\r\n");
	strcat(answeRequest,"\r\n");
}


void not_supported_body(char * answeRequest)
{
	strcat(answeRequest,"\r\n");
	strcat(answeRequest,"<HTML><HEAD><TITLE>501 Not supported</TITLE></HEAD>\r\n");
	strcat(answeRequest,"<BODY><H4>501 Not supported</H4>\r\n");
	strcat(answeRequest,"Method is not supported.\r\n");
	strcat(answeRequest,"</BODY></HTML>\r\n");
	strcat(answeRequest,"\r\n");
}


void Bad_Request_body(char *answeRequest)
{

	strcat(answeRequest,"\r\n");
	strcat(answeRequest,"<HTML><HEAD><TITLE>400 Bad Request</TITLE></HEAD>\r\n");
	strcat(answeRequest,"<BODY><H4>400 Bad request</H4>\r\n");
	strcat(answeRequest,"Bad Request.\r\n");
	strcat(answeRequest,"</BODY></HTML>\r\n");
	strcat(answeRequest,"\r\n");
}


void connection_server(char *answeRequest)
{	
	strcat(answeRequest,"Connection: close");		
	strcat(answeRequest,"\r\n");


}

char* get_mine_type(char* name)
{
	char *ext = strrchr(name, '.');
	if (!ext) return NULL;
	if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html";
	if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
	if (strcmp(ext, ".gif") == 0) return "image/gif";
	if (strcmp(ext, ".png") == 0) return "image/png";
	if (strcmp(ext, ".css") == 0) return "text/css";
	if (strcmp(ext, ".au") == 0) return "audio/basic";
	if (strcmp(ext, ".wav") == 0) return "audio/wav";
	if (strcmp(ext, ".avi") == 0) return "video/x-msvideo";
	if (strcmp(ext, ".mpeg") == 0 || strcmp(ext, ".mpg") == 0) return "video/mpeg";
	if (strcmp(ext, ".mp3") == 0) return "audio/mpeg";
	return NULL;
}

void last_modified_server(char *answeRequest,char*name)
{
	strcat(answeRequest,"Last-Modified: ");
	char timebuf[MAX_TIME];
	struct stat b;
	if(!stat(name, &b))
	{
	strftime(timebuf,sizeof(timebuf),RFC1123FMT,gmtime(&(b.st_mtime)));
	strcat(answeRequest,timebuf);		
	strcat(answeRequest,"\r\n");
	}
	else
	{
		strcat(answeRequest,"null");		
		strcat(answeRequest,"\r\n");
	}

}

void date_server(char *answeRequest)
{
	strcat(answeRequest,"Date: ");	
	time_t now;
	char timebuf[MAX_TIME];
	now=time(NULL);
	strftime(timebuf,sizeof(timebuf),RFC1123FMT,gmtime(&now));
	strcat(answeRequest,timebuf);		
	strcat(answeRequest,"\r\n");

}

void web_server(char *answeRequest)
{
	strcat(answeRequest,"Server: webserver/1.0");		
	strcat(answeRequest,"\r\n");
	date_server(answeRequest);
}

void length_server(char *answeRequest,char* length)
{
	strcat(answeRequest,"Content-Length: ");
	strcat(answeRequest,length);	
	strcat(answeRequest,"\r\n");

}

void content_type_server(char *answeRequest,char* content_type)
{

	if(content_type==NULL)
	{
		return;          
	}
	else
	{
		strcat(answeRequest,"Content-Type: ");		
		strcat(answeRequest,content_type);	
		strcat(answeRequest,"\r\n");
	}

}
void location_server(char *answeRequest,char*name)
{
	strcat(answeRequest,"Location: <");		
	strcat(answeRequest,name);
	strcat(answeRequest,">\\");	
	strcat(answeRequest,"\r\n");
}

void _file1(char*answeRequest,char*name,char*size)
{
	char* content_type;
	strcat(answeRequest,"HTTP/1.1 ");
	strcat(answeRequest,STATUS200);
	strcat(answeRequest," ");
	strcat(answeRequest,PHRASE200);		
	strcat(answeRequest,"\r\n");
	web_server(answeRequest);
	content_type=get_mine_type(name);
	content_type_server(answeRequest,content_type);
	length_server(answeRequest,size);
	last_modified_server(answeRequest,name);
	connection_server(answeRequest);
	strcat(answeRequest,"\r\n");


}


void _file(char*answeRequest,char*name,char*read_file,char*size)
{
	char* content_type;
	strcat(answeRequest,"HTTP/1.1 ");
	strcat(answeRequest,STATUS200);
	strcat(answeRequest," ");
	strcat(answeRequest,PHRASE200);		
	strcat(answeRequest,"\r\n");
	web_server(answeRequest);
	content_type=get_mine_type(name);
	content_type_server(answeRequest,content_type);
	length_server(answeRequest,size);
	last_modified_server(answeRequest,name);
	connection_server(answeRequest);
	strcat(answeRequest,"\r\n");
	strcat(answeRequest,read_file);


}


void dir_content(char *answeRequest,char*name ,char*file_name[],char*file_time[],char file_size[][22],int i,int st_size)
{
	char* content_type;
	char _size[st_size];
	sprintf(_size, "%d", st_size);//file size
	strcat(answeRequest,"HTTP/1.1 ");
	strcat(answeRequest,STATUS200);
	strcat(answeRequest," ");
	strcat(answeRequest,PHRASE200);		
	strcat(answeRequest,"\r\n");
	web_server(answeRequest);
	content_type=get_mine_type(name);
	content_type_server(answeRequest,content_type);
	length_server(answeRequest,_size);
	last_modified_server(answeRequest,name);
	connection_server(answeRequest);
	dir_content_body(answeRequest,name,file_name,file_time,file_size,i);

}

void internal_server(char * answeRequest,char *name)
{
	char* content_type;
	strcat(answeRequest,"HTTP/1.1 ");
	strcat(answeRequest,STATUS500);
	strcat(answeRequest," ");
	strcat(answeRequest,PHRASE500);		
	strcat(answeRequest,"\r\n");
	web_server(answeRequest);
	content_type=get_mine_type(name);
	content_type_server(answeRequest,content_type);
	length_server(answeRequest,"144");
	connection_server(answeRequest);
	internal_server_body(answeRequest);


}



void fordidden(char * answeRequest,char *name)
{
	char* content_type;
	strcat(answeRequest,"HTTP/1.1 ");
	strcat(answeRequest,STATUS403);
	strcat(answeRequest," ");
	strcat(answeRequest,PHRASE403);		
	strcat(answeRequest,"\r\n");
	web_server(answeRequest);
	content_type=get_mine_type(name);
	content_type_server(answeRequest,content_type);
	length_server(answeRequest,"111");
	connection_server(answeRequest);
	fordidden_body(answeRequest);


}


void found(char* answeRequest,char* name)
{
	char* content_type;
	strcat(answeRequest,"HTTP/1.1 ");
	strcat(answeRequest,STATUS302);
	strcat(answeRequest," ");
	strcat(answeRequest,PHRASE302);		
	strcat(answeRequest,"\r\n");
	web_server(answeRequest);
	location_server(answeRequest,name);
	content_type=get_mine_type(name);
	content_type_server(answeRequest,content_type);
	length_server(answeRequest,"123");
	connection_server(answeRequest);
	found_body(answeRequest);
}


void not_found(char* answeRequest,char *name)
{
	char* content_type;
	strcat(answeRequest,"\r\n");
	strcat(answeRequest,"HTTP/1.1 ");
	strcat(answeRequest,STATUS404);
	strcat(answeRequest," ");
	strcat(answeRequest,PHRASE404);		
	strcat(answeRequest,"\r\n");
	web_server(answeRequest);
	content_type=get_mine_type(name);
	content_type_server(answeRequest,content_type);
	length_server(answeRequest,"112");
	connection_server(answeRequest);
	not_found_body(answeRequest);
}


void not_supported(char * answeRequest,char*name)
{
	char* content_type;
	strcat(answeRequest,"\r\n");
	strcat(answeRequest,"HTTP/1.1 ");
	strcat(answeRequest,STATUS501);
	strcat(answeRequest," ");
	strcat(answeRequest,PHRASE501);		
	strcat(answeRequest,"\r\n");
	web_server(answeRequest);
	content_type=get_mine_type(name);
	content_type_server(answeRequest,content_type);
	length_server(answeRequest,"129");
	connection_server(answeRequest);
	not_supported_body(answeRequest);

}


void bad_request(char *answeRequest,char*name)
{
	char* content_type;
	strcat(answeRequest,"\r\n");
	strcat(answeRequest,"HTTP/1.1 ");
	strcat(answeRequest,STATUS400);
	strcat(answeRequest," ");
	strcat(answeRequest,PHRASE400);		
	strcat(answeRequest,"\r\n");
	web_server(answeRequest);
	content_type=get_mine_type(name);
	content_type_server(answeRequest,content_type);
	length_server(answeRequest,"113");
	connection_server(answeRequest);
	Bad_Request_body(answeRequest);
	
}

void check_request_from_client(char *buffer,int size,int newsock)
{
	char *temp=strstr(buffer,"\r\n");
	temp[0]='\0';
	int i=0;
	char *request[size];
	struct stat s;
	char answeRequest1[ENTITY_LINE*7];
	request[i]=strtok(buffer," "); 
	while(request[i]!=NULL)
	{
		i++;
		request[i]=strtok(NULL," ");
	}
	char *ptr=strstr(request[2],"\r\n");
	if(ptr!=NULL)
		request[2][(int)(ptr-request[2])]='\0';//delete /r/n

	if(i!=3)
	{
		char *answeRequest=(char*)malloc(sizeof(char)*ENTITY_LINE*7);
		if(answeRequest==NULL)
		{
			internal_server(answeRequest1,request[1]);//malloc fail
			write(newsock,answeRequest1,strlen(answeRequest1));
			return;
		}
		answeRequest[0]='\0';
		bad_request(answeRequest,request[1]);
		write(newsock,answeRequest,strlen(answeRequest));
		free(answeRequest);
		answeRequest=NULL;
	}
	else if(strcmp(request[2],"HTTP/1.1")!=0)
	{
		char *answeRequest=(char*)malloc(sizeof(char)*ENTITY_LINE*7);
		if(answeRequest==NULL)
		{
			internal_server(answeRequest1,request[1]);//malloc fail
			write(newsock,answeRequest1,strlen(answeRequest1));
			return;
		}
		answeRequest[0]='\0';
		bad_request(answeRequest,request[1]);
		write(newsock,answeRequest,strlen(answeRequest));
		free(answeRequest);
		answeRequest=NULL;
	}
	else if(strcmp(request[0],"GET")!=0)
	{
		char *answeRequest=(char*)malloc(sizeof(char)*ENTITY_LINE*7);
		if(answeRequest==NULL)
		{
			internal_server(answeRequest1,request[1]);//malloc fail
			write(newsock,answeRequest1,strlen(answeRequest1));
			return;
		}
		answeRequest[0]='\0';
		not_supported(answeRequest,request[1]);
		write(newsock,answeRequest,strlen(answeRequest));
		free(answeRequest);
		answeRequest=NULL;
	}

	char cwd[FIRST_LINE];
	getcwd(cwd,sizeof(cwd));
	strcat(cwd,request[1]);
	strcpy(request[1],cwd);

	if((stat(request[1],&s)!=0)&&(S_ISDIR(s.st_mode)==0))//no file or directory
	{
		char *answeRequest=(char*)malloc(sizeof(char)*ENTITY_LINE*7);
		if(answeRequest==NULL)
		{
			internal_server(answeRequest1,request[1]);//malloc fail
			write(newsock,answeRequest1,strlen(answeRequest1));
			return;
		}
		answeRequest[0]='\0';
		not_found(answeRequest,request[1]);
		write(newsock,answeRequest,strlen(answeRequest));
		free(answeRequest);
		answeRequest=NULL;	
    	}
   	else if((stat(request[1], &s) == 0 )&&(S_ISDIR(s.st_mode)!=0))//directory-not file
    	{
		int size=strlen(request[1])-1;

		if(strcmp(&(request[1][size]),"/")!=0)//dir not ent with '/'
		{
			char *answeRequest=(char*)malloc(sizeof(char)*ENTITY_LINE*7);
			if(answeRequest==NULL)
			{
				internal_server(answeRequest1,request[1]);//malloc fail
				write(newsock,answeRequest1,strlen(answeRequest1));
				return;
			}
			answeRequest[0]='\0';
			found(answeRequest,request[1]);
			write(newsock,answeRequest,strlen(answeRequest));
			free(answeRequest);
			answeRequest=NULL;			
		}

		if(strcmp(&(request[1][size]),"/")==0)//dir end with /
		{

			char *answeRequest=(char*)malloc(sizeof(char)*(ENTITY_LINE*s.st_size*7));
			if(answeRequest==NULL)
			{
				internal_server(answeRequest1,request[1]);//malloc fail
				write(newsock,answeRequest1,strlen(answeRequest1));
				return;
			}
			answeRequest[0]='\0';
			DIR *d,*d1;
			int htmlExsit=0;
			int size=0;
			struct dirent *dir;
			d=opendir(request[1]);
			if(d==NULL)//no such dir
			{
				not_found(answeRequest,request[1]);
				write(newsock,answeRequest,strlen(answeRequest));
				free(answeRequest);
				answeRequest=NULL;
				return;
			}
			d1=opendir(request[1]);
			if(d1==NULL)//no such dir
			{ 
				not_found(answeRequest,request[1]);
				write(newsock,answeRequest,strlen(answeRequest));
				free(answeRequest);
				answeRequest=NULL;
				closedir(d);
				return;
				
			}
			dir=readdir(d);
			while(dir!=NULL)
			{
				if(strcmp(dir->d_name,"index.html")!=0)//there is'nt index.html file
				{
					size++;
					dir=readdir(d);
				}
				else//there is index.html
				{
					htmlExsit=1;	
					char *path=request[1];
					char *copy=(char*)malloc(sizeof(char)*ENTITY_LINE*s.st_size*7);//copy index.html to here
					if(copy==NULL)
					{
						internal_server(answeRequest1,request[1]);//malloc fail
						write(newsock,answeRequest1,strlen(answeRequest1));
						return;
					}
					copy[0]='\0';
					char size_file[s.st_size];
					bzero(size_file,sizeof(size_file));
					strcat(path,"index.html");//update path
					FILE *fp=fopen(path, "r");//open the file
					fread(copy, sizeof(char*), (ENTITY_LINE*s.st_size*7), fp);//read file
					 _file(answeRequest,path,copy,size_file);
					write(newsock,answeRequest,strlen(answeRequest));
					free(copy);
					copy=NULL;
					fclose(fp);
					fp=NULL;
					break;
				}
			}

			if(htmlExsit==0)
			{
				char*name[size];//dir/file name
				char*time[size];//dir/file time
				char file_size[size][22];//file size
				char timebuf[128];
				bzero(name,sizeof(name));
				bzero(time,sizeof(time));
				bzero(file_size,sizeof(file_size));
				int i=0;
				dir=readdir(d1);

				while(dir!=NULL)
				{
					name[i]=dir->d_name;
					char path[strlen(request[1])+strlen(dir->d_name)];
					strcpy(path,request[1]);
					strcat(path,dir->d_name);
					struct stat a;
					stat(path, &a);
					strftime(timebuf,sizeof(timebuf),RFC1123FMT,gmtime(&a.st_mtime));
					time[i]=timebuf;
					
					if(S_ISDIR(a.st_mode)==0)
					{
						sprintf(file_size[i], "%lu", a.st_size);//file size
						
					}
					else
					{
						strcpy(file_size[i],"1");
							
					}
					i++;
					dir=readdir(d1);
				}
				dir_content(answeRequest,request[1],name,time,file_size,size,s.st_size);
				write(newsock,answeRequest,strlen(answeRequest));
			}
			closedir(d);
			closedir(d1);
			free(answeRequest);
			answeRequest=NULL;
		}
		
	}

	else if((stat(request[1], &s) == 0 )&&(S_ISDIR(s.st_mode)==0))//file-not directory
	{

		if(S_ISREG(s.st_mode)==0)//not regular file
		{
			char *answeRequest=(char*)malloc(sizeof(char)*ENTITY_LINE*7);
			if(answeRequest==NULL)
			{
				internal_server(answeRequest1,request[1]);//malloc fail
				write(newsock,answeRequest1,strlen(answeRequest1));
				return;
			}
			answeRequest[0]='\0';
			fordidden(answeRequest,request[1]);
			write(newsock,answeRequest,strlen(answeRequest));
			free(answeRequest);
			answeRequest=NULL;
		}
		else if(!(s.st_mode&S_IRUSR &&s.st_mode&S_IRGRP&&s.st_mode&S_IROTH))//cant read from file
		{
			char *answeRequest=(char*)malloc(sizeof(char)*ENTITY_LINE*7);
			if(answeRequest==NULL)
			{
				internal_server(answeRequest1,request[1]);//malloc fail
				write(newsock,answeRequest1,strlen(answeRequest1));
				return;
			}
			answeRequest[0]='\0';
			fordidden(answeRequest,request[1]);
			write(newsock,answeRequest,strlen(answeRequest));
			free(answeRequest);
			answeRequest=NULL;
		}
		else//chack path premission
		{	
			char*ptr;
			char copy[strlen(request[1])];
			ptr=strchr(request[1],'/');
			while(ptr!=NULL)
			{
				copy[0]='\0';
				strncat(copy,request[1],(int)((ptr+1)-request[1]));
				struct stat st1;
				stat(copy,&st1);
				if(!(st1.st_mode&S_IRUSR &&st1.st_mode&S_IRGRP&&st1.st_mode&S_IROTH))
				{
					char *answeRequest=(char*)malloc(sizeof(char)*ENTITY_LINE*7);
					if(answeRequest==NULL)
					{
						internal_server(answeRequest1,request[1]);//malloc fail
						write(newsock,answeRequest1,strlen(answeRequest1));
						return;
					}
					answeRequest[0]='\0';
					fordidden(answeRequest,request[1]);
					write(newsock,answeRequest,strlen(answeRequest));
					free(answeRequest);
					answeRequest=NULL;
				}
				else
				{
					ptr+=1;
					ptr=strchr(ptr,'/');
					if(ptr==NULL)
						break;			
				}
			}
			//regular file
			struct stat stt;
			int n;
			stat(request[1],&stt);
			char *answeRequest=(char*)malloc(sizeof(char)*(ENTITY_LINE*7));
			if(answeRequest==NULL)
			{
				internal_server(answeRequest1,request[1]);//malloc fail
				write(newsock,answeRequest1,strlen(answeRequest1));
				return;
			}
			answeRequest[0]='\0';
			char *number=(char*)malloc(sizeof(char)*(stt.st_size));
			if(number==NULL)
			{
				internal_server(answeRequest1,request[1]);//malloc fail
				write(newsock,answeRequest1,strlen(answeRequest1));
				free(answeRequest);
				return;
			}
			number[0]='\0';
			char *buffer=(char*)malloc(sizeof(char)*(stt.st_size+1));
			if(buffer==NULL)
			{
				internal_server(answeRequest1,request[1]);//malloc fail
				write(newsock,answeRequest1,strlen(answeRequest1));
				free(answeRequest);
				free(number);
				return;
			}
			buffer[0]='\0';
			sprintf(number, "%lu", stt.st_size);//convert to string
			_file1(answeRequest,request[1],number);
			write(newsock,answeRequest,strlen(answeRequest));
			FILE *fp= fopen(request[1], "rb");
			fseek(fp,0,SEEK_SET);
			while((n=fread(buffer,1,stt.st_size+1,fp))>0)
				write(newsock,buffer,n);
			fclose(fp);
			free(buffer);
			free(number);
			free(answeRequest);
			answeRequest=NULL;
		}
	
	}	
			
}




int read_write(void* newsockfd)//read job from the client
{
	char buffer[FIRST_LINE]; 
	int size,n;
	int newsock=*(int*)newsockfd;

	//read
	bzero(buffer,sizeof(buffer));
	n=read(newsock,buffer,FIRST_LINE);  
	if(n<0)
		error("ERROR read from socket");
	size=strlen(buffer);
	check_request_from_client(buffer,size,newsock);
	close(newsock);//close connection after end the job
	return 0;

}

 
int main(int argc, char *argv[]) 
{         
	int sockfd, portno;     
	struct sockaddr_in serv_addr; 
	struct sockaddr cli_addr;
	int pool_size;
	int max_request; 
	socklen_t clilen;    

	if (argc !=4)
	{          
		printf("Usage: server<port> <pool-size> <max-number-of-request>\n"); 
	}
	else
	{

/*========================== connection infrastructure ======================*/

		pool_size=atoi(argv[2]);
		max_request=atoi(argv[3]); 
		threadpool* pool=create_threadpool(pool_size);//init pool
		if(!pool)
			error("pool init fail"); 
	
		sockfd = socket(AF_INET,SOCK_STREAM,0); 
		if (sockfd < 0)  
			error("ERROR opening socket"); 
	
		portno = atoi(argv[1]); 
		serv_addr.sin_family = AF_INET;  
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
		serv_addr.sin_port = htons(portno); 
	
		if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)                 
			error("ERROR on binding");    
	  	
	
		if(listen(sockfd,5)<0)
			error("ERROR on listen"); 
		
		int i;
		int newsockfd[max_request];
		for(i=0;i<max_request;i++)
		{
			clilen=sizeof(cli_addr);
			newsockfd[i]=accept(sockfd,&cli_addr, &clilen);//wait to "job" from the client
			if(newsockfd[i]<0)
				error("ERROR on accept");
			dispatch(pool, read_write, newsockfd+i);
	
		}
		destroy_threadpool(pool);
		close(sockfd);
		return 0;
	}
	return 0;
}







