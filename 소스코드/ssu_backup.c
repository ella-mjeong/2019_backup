#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <ctype.h>
#include "singlelinkedlist.h"

void command_add(char* command); 
void command_remove(char* command);
void command_compare(char* command); 
void command_recover(char* command); 
void command_list(); 
void printLog(char *name, int index); 
void *backupFile(void *arg);
void optionAddN(char *filename,int number);
void optionAddD(char *filename,int period,char* option);
void recoverFileRead(char* fname);
void makeTable();
void readTable();

char dirname[PATH_MAX];
int checkD=0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
LinkedList list;

#define MODE R_OK|X_OK|W_OK
#define MAX_SIZE 1024

int main(int argc, char* argv[]){

	struct timeval begin_t, end_t;
	char command[MAX_SIZE];
	char b_directory[MAX_SIZE];
	char pathname[MAX_SIZE];
	Node *temp;
	char csvfile[MAX_SIZE];
	char *ddummy;
	char tempname[MAX_SIZE];
	char pname[MAX_SIZE];

	Init(&list);

	if(argc>2){
		fprintf(stderr, "usage: ./ssu_backup <directory>\n");
		exit(1);
	}

	if(argc==1){
		getcwd(pathname,MAX_SIZE);
		if(access("backupDir",F_OK)){
			mkdir("backupDir",0777);
		}
		sprintf(dirname,"%s/backupDir",pathname);
	}

	else{
		if(argv[1][0]=='/')
			sprintf(pathname,"%s",argv[1]);
		else if(argv[1][0]=='.'){
			strcpy(tempname,argv[1]);
			ddummy=tempname+2;
			strcpy(tempname,ddummy);
			getcwd(pname,MAX_SIZE);
			sprintf(pathname,"%s/%s",pname,tempname);
		}
		else{
			getcwd(pname,MAX_SIZE);
			sprintf(pathname,"%s/%s",pname,argv[1]);

		}

		if(access(pathname, F_OK)){
			fprintf(stderr, "usage : directory is not exist\n");
			exit(1);
		}

		struct stat st;
		stat(pathname, &st);
		if(!S_ISDIR(st.st_mode)){
			fprintf(stderr, "usage : No directory\n");
			exit(1);
		}

		if(access(pathname,MODE)){
			fprintf(stderr, "usage : directory에 접근권한이 없습니다.\n");
			exit(1);
		}

		sprintf(dirname,"%s/backupDir",pathname);

		if(access(dirname,F_OK)){
			mkdir(dirname,0777);
		}
	}

	sprintf(csvfile,"%s/listLog.csv",dirname);
	if(!access(csvfile,F_OK)){
		readTable();
	}

	while(1){
		printf("20160433>");
		strcpy(command,"");

		scanf("%[^\n]s",command);
		getc(stdin);
		if(command[0]==0)
			continue;

		if(!strcmp(command,"exit")){
			temp=list.head;
			for(int i=0;i<list.size;i++){
				if(i!=0){
					temp=temp->next;
				}
				pthread_cancel(temp->tid);
			}

			pthread_mutex_destroy(&mutex); 
			makeTable();

			delAllNode(&list);
			break;
		}

		if(strstr(command," ")==NULL){
			if(!strcmp(command,"list")){
				command_list();
			}
			else if(!strcmp(command,"ls")){
				system(command);
			}
			else if(!strcmp(command,"vi")){
				system(command);
			}
			else if(!strcmp(command,"vim")){
				system(command);
			}

			else if(!strcmp(command, "add")){
				fprintf(stderr, "usage : add <FILENAME> [PERIOD] [OPTION]\n");
				continue;
			}
			else if(!strcmp(command, "remove")){
				fprintf(stderr, "usage : remove <FILENAME> [OPTION]\n");
				continue;
			}
			else if(!strcmp(command, "compare")){
				fprintf(stderr, "usage : compare <FILENAME1> <FILENAME2>\n");
				continue;
			}
			else if(!strcmp(command, "recover")){
				fprintf(stderr, "usage : recover <FILENAME> [OPTION]\n");
				continue;
			}
			else{
				fprintf(stderr, "존재하지 않는 명령어입니다.\n");
				continue;
			}
		}

		else{
			char dummy_command1[1024];
			char* dummy_command2;

			strcpy(dummy_command1,command);
			dummy_command2 = strtok(dummy_command1," ");

			if(!strcmp(dummy_command1,"add")){
				command_add(command);
			}
			else if(!strcmp(dummy_command1,"remove")){
				command_remove(command);
			}
			else if(!strcmp(dummy_command1,"compare")){
				command_compare(command);
			}
			else if(!strcmp(dummy_command1,"recover")){
				command_recover(command);
			}
			else if(!strcmp(dummy_command1,"vi")){
				system(command);
			}
			else if(!strcmp(dummy_command1,"vim")){
				system(command);
			}
			else if(!strcmp(dummy_command1,"ls")){
				system(command);
			}
			else{
				fprintf(stderr, "존재하지 않는 명령어입니다.\n");
				continue;
			}
		}
	}
	exit(0);
}

void command_add(char* command){
	pthread_t tid;
	int add_m_flag=0;
	int add_n_flag=0;
	int add_t_flag=0;
	int add_d_flag=0;
	char dummy_command1[1024];
	char filename[MAX_SIZE];
	char pathname[MAX_SIZE];
	int period;
	char option[100]=" ";
	char optionPrint[100]=" ";
	int number;
	int addTime;
	double temp;
	double temp2;
	char tempName[100];
	struct stat s;
	char *dum2;
	char dum[300];
	char *token;
	char *ptr[2];
	char *token_data[4];
	char per[5];
	char data[5]="";
	char tsec[5];
	int k=0;
	int check_f=0;
	int check_f2=0;
	int check_count=0;
	char *ddummy;

	strcpy(dummy_command1,command);
	token =strtok_r(dummy_command1," ",&ptr[0]);

	token_data[0] =strtok_r(ptr[0]," ",&ptr[1]);
	token_data[1]=strtok_r(ptr[1]," ",&ptr[0]);
	token_data[2]=strtok_r(ptr[0]," ",&ptr[1]);

	if(token_data[2] != NULL){
		sprintf(token_data[2],"%s %s",token_data[2],ptr[1]);
	}

	strcpy(tempName, token_data[0]);
	if(tempName[0]=='.'){
		ddummy=token_data[0]+2;
		strcpy(token_data[0],ddummy);

		strcpy(dum,token_data[0]);
		dum2 = strtok(dum,"/");

		while(dum2!=NULL){
			if((strstr(dum2,"."))!=NULL){
				check_f=1;
				check_f2=1;
				break;
			}
			dum2=strtok(NULL,"/");
		}

		if(!check_f){
			strcpy(dum,token_data[0]);
			dum2 = strtok(dum,"/");

			while(dum2!=NULL){
				if((strstr(dum2,"_"))!=NULL){
					check_f=0;
					check_f2=1;
					break;
				}
				dum2=strtok(NULL,"/");
			}
		}
		if(!check_f2){
			strcpy(dum,token_data[0]);
			dum2 = strtok(dum,"/");

			while(dum2!=NULL){
				check_count++;
				dum2=strtok(NULL,"/");
			}
			strcpy(dum,token_data[0]);
			dum2 = strtok(dum,"/");

			check_count--;
			while(dum2!=NULL){
				if(check_count==0){
					check_f2=0;
					break;
				}
				check_count--;
				dum2=strtok(NULL,"/");
			}
		}

		strcpy(tempName,dum2);
		getcwd(pathname,MAX_SIZE);
		sprintf(filename,"%s/%s",pathname,token_data[0]);
	}
	else if(tempName[0]!='/'){
		getcwd(pathname,MAX_SIZE);
		sprintf(filename,"%s/%s",pathname,tempName);
	}
	else{
		strcpy(dum,token_data[0]);
		dum2 = strtok(dum,"/");

		while(dum2!=NULL){
			if((strstr(dum2,"."))!=NULL){
				check_f=1;
				check_f2=1;
				break;
			}
			dum2=strtok(NULL,"/");
		}

		if(!check_f){
			strcpy(dum,token_data[0]);
			dum2 = strtok(dum,"/");

			while(dum2!=NULL){
				if((strstr(dum2,"_"))!=NULL){
					check_f=0;
					check_f2=1;
					break;
				}
				dum2=strtok(NULL,"/");
			}
		}
		if(!check_f2){
			strcpy(dum,token_data[0]);
			dum2 = strtok(dum,"/");

			while(dum2!=NULL){
				check_count++;
				dum2=strtok(NULL,"/");
			}
			strcpy(dum,token_data[0]);
			dum2 = strtok(dum,"/");

			check_count--;
			while(dum2!=NULL){
				if(check_count==0){
					check_f2=0;
					break;
				}
				check_count--;
				dum2=strtok(NULL,"/");
			}
		}
		strcpy(tempName,dum2);
		strcpy(filename,token_data[0]);
	}

	for(int i=0;i<strlen(filename);i++){
		if(filename[i]>=33 && filename[i]<=122){
			continue;
		}
		else{
			fprintf(stderr, "can't use Hangul for %c\n",filename[i]);
			return;
		}
	}

	if(strlen(filename)>255){
		fprintf(stderr, "filename is over 255byte\n");
		return;
	}

	if(access(filename, F_OK)){
		fprintf(stderr, "usage : file is not exist\n");
		return;
	}

	add_m_flag=0;
	add_t_flag=0;
	add_n_flag = 0;

	if(token_data[1]==NULL){ 
		fprintf(stderr, "usage: <FILENAME> [period] [option]\n");
		return;
	}

	else{ 

		if(!isdigit(token_data[1][0])){ 
			fprintf(stderr, "usage: <FILENAME> [period] [option]\n");
			return;
		}
		else{ 
			strcpy(per,token_data[1]);
			temp = atof(per);
			if((temp - (int)temp)!=0){
				fprintf(stderr, "period는 정수형이어야합니다\n");
				return;
			}
			else{
				period = atoi(per);
				if(period<5 || period>10){
					fprintf(stderr, "5 <= period <= 10\n");
					return;
				}

				if(token_data[2] == NULL){
					strcpy(option , " ");
					strcpy(optionPrint," ");
				}
				else{
					strcpy(option," ");
					strcpy(optionPrint," ");

					if(token_data[2][0]!='-'){
						fprintf(stderr,"usage: <FILENAME> [period] [option]\n");
						return;
					}

					for(int i=0;i<strlen(token_data[2]);i++){
						if(token_data[2][i]=='-'){
							i++;
							if(token_data[2][i]=='m'){
								strcat(option,"-m ");
								strcat(optionPrint,"-m ");
								add_m_flag=1;
							}
							else if(token_data[2][i]=='n'){
								strcat(option, "-n ");
								strcat(optionPrint, "-n ");
								add_n_flag=1;
								if(token_data[2][i+1]=='\n'){
									fprintf(stderr, "usage : -n NUMBER\n");
									return;
								}
								if(token_data[2][i+1]!=' '){
									fprintf(stderr, "usage : -n NUMBER\n");
									return;
								}
								if(!isdigit(token_data[2][i+2])){
									fprintf(stderr, "usage : -n NUMBER\n");
									return;
								}

								i=i+2;
								for(k=i+1;k<strlen(token_data[2]);k++){
									if(token_data[2][k]==' '){
										break;
									}
								}
								strcpy(data,&token_data[2][i]);
								data[k-i]='\0';
								i=k-1;
								temp2 = atof(data);
								if((temp2 - (int)temp2)!=0){
									fprintf(stderr, "NUMBER는 정수형을 입력해야합니다.\n");
									return;
								}
								else{
									number = atoi(data);
									if(number<1 || number>100){
										fprintf(stderr, "1 <= NUMBER <= 100\n");
										return;
									}
									strcat(option, data);
									strcat(option," ");
								}
							}
							else if(token_data[2][i]=='t'){
								strcat(option,"-t ");
								strcat(optionPrint,"-t ");
								add_t_flag=1;	
								if(token_data[2][i+1]=='\n'){
									fprintf(stderr, "usage : -t TIME\n");
									return;
								}
								if(token_data[2][i+1]!=' '){
									fprintf(stderr, "usage: -t TIME\n");
									return;
								}
								if(!isdigit(token_data[2][i+2])){
									fprintf(stderr, "usage : -t TIME\n");
									return;
								}
								i=i+2;
								for(k=i+1;k<strlen(token_data[2]);k++){
									if(token_data[2][k]==' '){
										break;
									}
								}
								strcpy(tsec,&token_data[2][i]);
								tsec[k-i]='\0';
								i=k-1;
								temp2 = atof(tsec);
								if((temp2 - (int)temp2)!=0){
									fprintf(stderr, "TIME은 정수형을 입력해야합니다.\n");
									return;
								}
								else{
									addTime = atoi(tsec);
									if(addTime<60 || addTime>1200){
										fprintf(stderr, "60 <= TIME <= 1200\n");
										return;
									}
									strcat(option, tsec);
									strcat(option," ");
								}
							}
							else if(token_data[2][i]=='d'){
								add_d_flag=1;
							}
							else 
								printf("존재하지 않는 옵션입니다.\n");
						}
					}
				}
			}
		}
	}
	
	if(checkD){		
		strcat(optionPrint,"-d ");
	}

	if(stat(filename,&s)<0){
		fprintf(stderr, "stat error\n");
		exit(1);
	}

	if(isInList(&list,filename)){
		fprintf(stderr, "file is existed in backuplist\n");
		return;
	}

	if(!add_d_flag){

		if(!S_ISREG(s.st_mode)){
			fprintf(stderr, "path is only Regular file\n");
			return;
		}

		InsertNode(&list,filename,period,optionPrint,tempName,s.st_mtime,add_m_flag,add_t_flag,addTime, add_n_flag, number);
		printLog(filename,2);

		Node *tempNode;
		tempNode = (Node*)malloc(sizeof(Node*));
		tempNode= searchNode(&list,filename);

		if(pthread_create(&tid,NULL,backupFile,(void*)tempNode)!=0){
			fprintf(stderr, "pthread_create error\n");
			exit(1);
		}
	}

	if(add_d_flag){ 
		if(!S_ISDIR(s.st_mode)){
			fprintf(stderr, "path is only Directory file\n");
			return;
		}
		add_d_flag=0;
		checkD=1;
		optionAddD(filename,period,option);
		checkD=0;
	}
}

void command_remove(char* command){
	char dummy_command1[1024];
	char *dummy_command2;
	char *dummy_command3;
	char filename[MAX_SIZE];
	char pathname[MAX_SIZE];
	char tempName[100];
	Node *temp;
	char dum[400];
	char *dum2;
	int check_f=0;
	int check_f2=0;
	int check_count=0;
	char* ddummy;

	strcpy(dummy_command1,command);
	dummy_command2 = strtok(dummy_command1," ");

	dummy_command3 = strtok(NULL," ");
	if(dummy_command3[0]=='-'){
		if(!strcmp(dummy_command3,"-a")){

			temp=list.head;
			for(int i=0;i<list.size;i++){
				if(i!=0){
					temp=temp->next;
				}
				pthread_cancel(temp->tid);
				printLog(temp->file,3);
			}
			delAllNode(&list);
		}
		else{
			fprintf(stderr, "존재하지 않는 옵션입니다.\n");
			return;
		}
	}
	else{
		dummy_command2 = strtok(NULL," ");

		if(dummy_command2 !=NULL){
			if(!strcmp(dummy_command2,"-a")){
				fprintf(stderr, "usage: remove -a\n");
				return;
			}
			else{
				fprintf(stderr, "usage : remove [FILENAME]\n");
				return;
			}
		}
		else{

			strcpy(tempName, dummy_command3);
			if(tempName[0]=='.'){
				ddummy=dummy_command3+2;
				strcpy(dummy_command3,ddummy);

				strcpy(dum,dummy_command3);
				dum2 = strtok(dum,"/");

				while(dum2!=NULL){
					if((strstr(dum2,"."))!=NULL){
						check_f=1;
						check_f2=1;
						break;
					}
					dum2=strtok(NULL,"/");
				}

				if(!check_f){
					strcpy(dum,dummy_command3);
					dum2 = strtok(dum,"/");

					while(dum2!=NULL){
						if((strstr(dum2,"_"))!=NULL){
							check_f=0;
							check_f2=1;
							break;
						}
						dum2=strtok(NULL,"/");
					}
				}
				if(!check_f2){
					strcpy(dum,dummy_command3);
					dum2 = strtok(dum,"/");

					while(dum2!=NULL){
						check_count++;
						dum2=strtok(NULL,"/");
					}
					strcpy(dum,dummy_command3);
					dum2 = strtok(dum,"/");

					check_count--;
					while(dum2!=NULL){
						if(check_count==0){
							check_f2=0;
							break;
						}
						check_count--;
						dum2=strtok(NULL,"/");
					}
				}

				strcpy(tempName,dum2);
				getcwd(pathname,MAX_SIZE);
				sprintf(filename,"%s/%s",pathname,dummy_command3);
			}

			else if(tempName[0]!='/'){
				getcwd(pathname,MAX_SIZE);
				sprintf(filename,"%s/%s",pathname,tempName);
			}
			else{
				strcpy(dum,dummy_command3);
				dum2 = strtok(dum,"/");

				while(dum2!=NULL){
					if((strstr(dum2,"."))!=NULL){
						check_f=1;
						check_f2=1;
						break;
					}
					dum2=strtok(NULL,"/");
				}

				if(!check_f){
					strcpy(dum,dummy_command3);
					dum2 = strtok(dum,"/");

					while(dum2!=NULL){
						if((strstr(dum2,"_"))!=NULL){
							check_f=0;
							check_f2=1;
							break;
						}
						dum2=strtok(NULL,"/");
					}
				}
				if(!check_f2){
					strcpy(dum,dummy_command3);
					dum2 = strtok(dum,"/");

					while(dum2!=NULL){
						check_count++;
						dum2=strtok(NULL,"/");
					}
					strcpy(dum,dummy_command3);
					dum2 = strtok(dum,"/");

					check_count--;
					while(dum2!=NULL){
						if(check_count==0){
							check_f2=0;
							break;
						}
						check_count--;
						dum2=strtok(NULL,"/");
					}
				}

				strcpy(tempName,dum2);
				strcpy(filename,dummy_command3);
			}

			for(int i=0;i<strlen(filename);i++){
				if(filename[i]>=33 && filename[i]<=122){
					continue;
				}
				else{
					fprintf(stderr, "can't use Hangul\n");
					return;
				}
			}
			if(strlen(filename)>255){
				fprintf(stderr, "filename is over 255byte\n");
				return;
			}

			if(!isInList(&list,filename)){
				fprintf(stderr, "filename is not existed in backuplist\n");
				return;
			}
			else{
				pthread_cancel(searchNode(&list,filename)->tid);
				printLog(filename,3);
				delNode(&list,filename);
			}
		}
	}
}

void command_compare(char* command){
	char dummy_command1[1024];
	char *dummy_command2;
	char *dummy_command3;
	char *dummy_command4;
	char filename[MAX_SIZE];
	char pathname[MAX_SIZE];
	char tempName[100];
	struct stat s;
	char filename2[256];
	char pathname2[200];
	char tempName2[100];
	struct stat s2;
	char dum[400];
	char *dum2;
	int check_f=0;
	int check_f2=0;
	int check_count=0;
	int check_ff=0;
	int check_ff2=0;
	int check_count_f=0;
	char *ddummy;

	strcpy(dummy_command1,command);
	dummy_command2 = strtok(dummy_command1," ");

	dummy_command3 = strtok(NULL," ");
	dummy_command2 = strtok(NULL," ");
	if(dummy_command2 == NULL){ 
		fprintf(stderr, "usage : compare <FILENAME1> <FILENAME2>\n");
		return;
	}
	dummy_command4 = strtok(NULL, " ");
	if(dummy_command4 != NULL){
		fprintf(stderr, "usage : compare <FILENAME1> <FILENAME2>\n");
		return;
	}

	strcpy(tempName, dummy_command3);
	strcpy(tempName2, dummy_command2);
	if(tempName[0]=='.'){
		ddummy=dummy_command3+2;
		strcpy(dummy_command3,ddummy);

		strcpy(dum,dummy_command3);
		dum2 = strtok(dum,"/");

		while(dum2!=NULL){
			if((strstr(dum2,"."))!=NULL){
				check_f=1;
				check_f2=1;
				break;
			}
			dum2=strtok(NULL,"/");
		}

		if(!check_f){
			strcpy(dum,dummy_command3);
			dum2 = strtok(dum,"/");

			while(dum2!=NULL){
				if((strstr(dum2,"_"))!=NULL){
					check_f=0;
					check_f2=1;
					break;
				}
				dum2=strtok(NULL,"/");
			}
		}
		if(!check_f2){
			strcpy(dum,dummy_command3);
			dum2 = strtok(dum,"/");

			while(dum2!=NULL){
				check_count++;
				dum2=strtok(NULL,"/");
			}
			strcpy(dum,dummy_command3);
			dum2 = strtok(dum,"/");

			check_count--;
			while(dum2!=NULL){
				if(check_count==0){
					check_f2=0;
					break;
				}
				check_count--;
				dum2=strtok(NULL,"/");
			}
		}

		strcpy(tempName,dum2);
		getcwd(pathname,MAX_SIZE);
		sprintf(filename,"%s/%s",pathname,dummy_command3);
	}

	else if(tempName[0]!='/'){
		getcwd(pathname,MAX_SIZE);
		sprintf(filename,"%s/%s",pathname,tempName);
	}
	else{
		strcpy(dum,dummy_command3);
		dum2 = strtok(dum,"/");

		while(dum2!=NULL){
			if((strstr(dum2,"."))!=NULL){
				check_f=1;
				check_f2=1;
				break;
			}
			dum2=strtok(NULL,"/");
		}

		if(!check_f){
			strcpy(dum,dummy_command3);
			dum2 = strtok(dum,"/");

			while(dum2!=NULL){
				if((strstr(dum2,"_"))!=NULL){
					check_f=0;
					check_f2=1;
					break;
				}
				dum2=strtok(NULL,"/");
			}
		}
		if(!check_f2){
			strcpy(dum,dummy_command3);
			dum2 = strtok(dum,"/");

			while(dum2!=NULL){
				check_count++;
				dum2=strtok(NULL,"/");
			}
			strcpy(dum,dummy_command3);
			dum2 = strtok(dum,"/");

			check_count--;
			while(dum2!=NULL){
				if(check_count==0){
					check_f2=0;
					break;
				}
				check_count--;
				dum2=strtok(NULL,"/");
			}
		}
		strcpy(tempName,dum2);
		strcpy(filename,dummy_command3);
	}

	for(int i=0;i<strlen(filename);i++){
		if(filename[i]>=33 && filename[i]<=122){
			continue;
		}
		else{
			fprintf(stderr, "<FILENAME1> can't use Hangul\n");
			return;
		}
	}

	if(strlen(filename)>255){
		fprintf(stderr, "<FILENAME1> is over 255byte\n");
		return;
	}
	if(access(filename, F_OK)){
		fprintf(stderr, "usage : <FILENAME1> is not exist\n");
		return;
	}

	if(stat(filename,&s)<0){
		fprintf(stderr, "stat error\n");
		exit(1);
	}

	if(!S_ISREG(s.st_mode)){
		fprintf(stderr, "<FILENAME1> is only Regular file\n");
		return;
	}

	if(tempName[0]=='.'){
		ddummy=dummy_command2+2;
		strcpy(dummy_command2,ddummy);

		strcpy(dum,dummy_command2);
		dum2 = strtok(dum,"/");

		while(dum2!=NULL){
			if((strstr(dum2,"."))!=NULL){
				check_f=1;
				check_f2=1;
				break;
			}
			dum2=strtok(NULL,"/");
		}

		if(!check_f){
			strcpy(dum,dummy_command2);
			dum2 = strtok(dum,"/");

			while(dum2!=NULL){
				if((strstr(dum2,"_"))!=NULL){
					check_f=0;
					check_f2=1;
					break;
				}
				dum2=strtok(NULL,"/");
			}
		}
		if(!check_f2){
			strcpy(dum,dummy_command2);
			dum2 = strtok(dum,"/");

			while(dum2!=NULL){
				check_count++;
				dum2=strtok(NULL,"/");
			}
			strcpy(dum,dummy_command2);
			dum2 = strtok(dum,"/");

			check_count--;
			while(dum2!=NULL){
				if(check_count==0){
					check_f2=0;
					break;
				}
				check_count--;
				dum2=strtok(NULL,"/");
			}
		}
		strcpy(tempName,dum2);
		getcwd(pathname,MAX_SIZE);
		sprintf(filename,"%s/%s",pathname,dummy_command2);
	}

	else if(tempName2[0]!='/'){
		getcwd(pathname2,200);
		sprintf(filename2,"%s/%s",pathname2,tempName2);
	}
	else{
		strcpy(dum,dummy_command2);
		dum2 = strtok(dum,"/");

		while(dum2!=NULL){
			if((strstr(dum2,"."))!=NULL){
				check_f=1;
				check_f2=1;
				break;
			}
			dum2=strtok(NULL,"/");
		}

		if(!check_f){
			strcpy(dum,dummy_command2);
			dum2 = strtok(dum,"/");

			while(dum2!=NULL){
				if((strstr(dum2,"_"))!=NULL){
					check_f=0;
					check_f2=1;
					break;
				}
				dum2=strtok(NULL,"/");
			}
		}
		if(!check_f2){
			strcpy(dum,dummy_command2);
			dum2 = strtok(dum,"/");

			while(dum2!=NULL){
				check_count++;
				dum2=strtok(NULL,"/");
			}
			strcpy(dum,dummy_command2);
			dum2 = strtok(dum,"/");

			check_count--;
			while(dum2!=NULL){
				if(check_count==0){
					check_f2=0;
					break;
				}
				check_count--;
				dum2=strtok(NULL,"/");
			}
		}

		strcpy(tempName2,dum2);
		strcpy(filename2,dummy_command2);
	}

	for(int i=0;i<strlen(filename2);i++){
		if(filename2[i]>=33 && filename2[i]<=122){
			continue;
		}
		else{
			fprintf(stderr, "<FILENAME2> can't use Hangul\n");
			return;
		}
	}

	if(strlen(filename2)>255){
		fprintf(stderr, "<FILENAME2> is over 255byte\n");
		return;
	}
	if(access(filename2, F_OK)){
		fprintf(stderr, "usage : <FILENAME2> is not exist\n");
		return;
	}

	if(stat(filename2,&s2)<0){ 
		fprintf(stderr, "stat error\n");
		exit(1);
	}

	if(!S_ISREG(s2.st_mode)){
		fprintf(stderr, "<FILENAME2> is only Regular file\n");
		return;
	}

	if((s.st_size==s2.st_size) && (s.st_mtime==s2.st_mtime)){
		printf("%s와 %s는 동일한 파일입니다.\n",tempName, tempName2);
	}

	else{
		printf("%s %ld %ld\n",tempName, s.st_mtime, s.st_size);
		printf("%s %ld %ld\n",tempName2, s2.st_mtime, s2.st_size);
	}
}

void command_recover(char* command){
	char dummy_command1[1024];
	char *dummy_command2;
	char *dummy_command3;
	char *dummy_command4;
	char filename[MAX_SIZE];
	char pathname[MAX_SIZE];
	char tempName[100];
	char pathname2[MAX_SIZE];
	char filename2[MAX_SIZE];
	char tempName2[100];
	char tempName3[100];
	char newfile[300];
	char fpath[1024];
	Node *temp;
	char dum[400];
	char dum1[400];
	char *dum2;
	char *dum3;

	int count=0;
	int num=0; 
	struct dirent **dentry;
	char *ptr;
	char path[200][1024];
	int file[1024];
	char buf[200][200];
	struct stat s[200];
	int choose;
	char choose2[5];
	char system_command[1024];
	int check_f=0;
	int check_f2=0;
	int check_count=0;
	char *ddummy;

	strcpy(dummy_command1,command);
	dummy_command2 = strtok(dummy_command1," ");

	dummy_command3 = strtok(NULL," ");
	dummy_command2 = strtok(NULL," ");

	if(dummy_command2 !=NULL){
		if(dummy_command2[0]=='-'){
			if(dummy_command2[1]=='n'){
				dummy_command4 =strtok(NULL," ");
				if(dummy_command4==NULL){
					fprintf(stderr, "usage: recover <FILENAME> [-n <NEWFILE>]\n");
					return;
				}
				else{
					strcpy(tempName2, dummy_command4);
					if(tempName[0]=='.'){
						ddummy=dummy_command4+2;
						strcpy(dummy_command3,ddummy);

						strcpy(dum1,dummy_command4);
						dum3 = strtok(dum1,"/");

						while(dum3!=NULL){
							if((strstr(dum3,"."))!=NULL){
								check_f=1;
								check_f2=1;
								break;
							}
							dum3=strtok(NULL,"/");
						}

						if(!check_f){
							strcpy(dum1,dummy_command4);
							dum3 = strtok(dum1,"/");

							while(dum2!=NULL){
								if((strstr(dum3,"_"))!=NULL){
									check_f=0;
									check_f2=1;
									break;
								}
								dum3=strtok(NULL,"/");
							}
						}
						if(!check_f2){
							strcpy(dum1,dummy_command4);
							dum3 = strtok(dum1,"/");

							while(dum2!=NULL){
								check_count++;
								dum2=strtok(NULL,"/");
							}
							strcpy(dum1,dummy_command4);
							dum3 = strtok(dum1,"/");

							check_count--;
							while(dum3!=NULL){
								if(check_count==0){
									check_f2=0;
									break;
								}
								check_count--;
								dum3=strtok(NULL,"/");
							}
						}

						strcpy(tempName2,dum3);
						getcwd(pathname2,MAX_SIZE);
						sprintf(filename2,"%s/%s",pathname2,dummy_command4);
					}

					else if(tempName2[0]!='/'){
						getcwd(pathname2,MAX_SIZE);
						sprintf(filename2,"%s/%s",pathname2,tempName2);
					}
					else{
						strcpy(dum1,dummy_command4);
						dum3 = strtok(dum1,"/");

						while(dum3!=NULL){
							if((strstr(dum3,"."))!=NULL){
								check_f=1;
								check_f2=1;
								break;
							}
							dum3=strtok(NULL,"/");
						}

						if(!check_f){
							strcpy(dum1,dummy_command4);
							dum3 = strtok(dum1,"/");

							while(dum3!=NULL){
								if((strstr(dum3,"_"))!=NULL){
									check_f=0;
									check_f2=1;
									break;
								}
								dum3=strtok(NULL,"/");
							}
						}
						if(!check_f2){
							strcpy(dum1,dummy_command4);
							dum3 = strtok(dum1,"/");

							while(dum3!=NULL){
								check_count++;
								dum3=strtok(NULL,"/");
							}
							strcpy(dum1,dummy_command4);
							dum3 = strtok(dum1,"/");

							check_count--;
							while(dum3!=NULL){
								if(check_count==0){
									check_f2=0;
									break;
								}
								check_count--;
								dum3=strtok(NULL,"/");
							}
						}

						strcpy(tempName2,dum3);
						strcpy(filename2,dummy_command4);
					}

					if(!access(filename2, F_OK)){
						fprintf(stderr, "usage : <NEWFILE> is existed\n");
						return;
					}
					for(int i=0;i<strlen(filename2);i++){
						if(filename2[i]>=33 && filename2[i]<=122){
							continue;
						}
						else{
							fprintf(stderr, "can't use Hangul\n");
							return;
						}
					}

					if(strlen(filename2)>255){
						fprintf(stderr, "<NEWFILE> is over 255byte\n");
						return;
					}
				}
			}
			else{
				fprintf(stderr, "존재하지않는 옵션입니다\n");
				return;
			}
		}
		else{
			fprintf(stderr, "usage: recover <FILENAME> [-n <NEWFILE>]\n");
			return;
		}

	}
	else{
		dummy_command2=NULL;
	}

	strcpy(tempName, dummy_command3);
	if(tempName[0]=='.'){
		ddummy=dummy_command3+2;
		strcpy(dummy_command3,ddummy);

		strcpy(dum,dummy_command3);
		dum2 = strtok(dum,"/");

		while(dum2!=NULL){
			if((strstr(dum2,"."))!=NULL){
				check_f=1;
				check_f2=1;
				break;
			}
			dum2=strtok(NULL,"/");
		}

		if(!check_f){
			strcpy(dum,dummy_command3);
			dum2 = strtok(dum,"/");

			while(dum2!=NULL){
				if((strstr(dum2,"_"))!=NULL){
					check_f=0;
					check_f2=1;
					break;
				}
				dum2=strtok(NULL,"/");
			}
		}
		if(!check_f2){
			strcpy(dum,dummy_command3);
			dum2 = strtok(dum,"/");

			while(dum2!=NULL){
				check_count++;
				dum2=strtok(NULL,"/");
			}
			strcpy(dum,dummy_command3);
			dum2 = strtok(dum,"/");

			check_count--;
			while(dum2!=NULL){
				if(check_count==0){
					check_f2=0;
					break;
				}
				check_count--;
				dum2=strtok(NULL,"/");
			}
		}

		strcpy(tempName,dum2);
		getcwd(pathname,MAX_SIZE);
		sprintf(filename,"%s/%s",pathname,dummy_command3);
	}

	else if(tempName[0]!='/'){
		getcwd(pathname,MAX_SIZE);
		sprintf(filename,"%s/%s",pathname,tempName);
	}
	else{
		strcpy(dum,dummy_command3);
		dum2 = strtok(dum,"/");

		while(dum2!=NULL){
			if((strstr(dum2,"."))!=NULL){
				check_f=1;
				check_f2=1;
				break;
			}
			dum2=strtok(NULL,"/");
		}

		if(!check_f){
			strcpy(dum,dummy_command3);
			dum2 = strtok(dum,"/");

			while(dum2!=NULL){
				if((strstr(dum2,"_"))!=NULL){
					check_f=0;
					check_f2=1;
					break;
				}
				dum2=strtok(NULL,"/");
			}
		}
		if(!check_f2){
			strcpy(dum,dummy_command3);
			dum2 = strtok(dum,"/");

			while(dum2!=NULL){
				check_count++;
				dum2=strtok(NULL,"/");
			}
			strcpy(dum,dummy_command3);
			dum2 = strtok(dum,"/");

			check_count--;
			while(dum2!=NULL){
				if(check_count==0){
					check_f2=0;
					break;
				}
				check_count--;
				dum2=strtok(NULL,"/");
			}
		}

		strcpy(tempName,dum2);
		strcpy(filename,dummy_command3);
	}

	for(int i=0;i<strlen(filename);i++){
		if(filename[i]>=33 && filename[i]<=122){
			continue;
		}
		else{
			fprintf(stderr, "can't use Hangul\n");
			return;
		}
	}

	if(strlen(filename)>255){
		fprintf(stderr, "filename is over 255byte\n");
		return;
	}

	if(access(filename, F_OK)){
		fprintf(stderr, "usage : <FILENAME> is not exist\n");
		return;
	}

	if(isInList(&list,filename)){
		pthread_cancel(searchNode(&list,filename)->tid);
		printLog(filename,3);
		delNode(&list,filename);
	}

	if((count = scandir(dirname,&dentry, NULL,alphasort)) == -1){
		fprintf(stderr, "scandir error\n");
		exit(1);
	}
	sprintf(tempName3,"/%s_",tempName);
	for(int i=0;i<count;i++){
		realpath(dentry[i]->d_name,fpath);
		if(strstr(fpath,tempName3))
			file[num++]=i;
	}

	if(num==0){
		fprintf(stderr, "복구할 수 있는 백업파일이 존재하지 않습니다.\n");
		return;
	}

	for(int i=0;i<num;i++){
		sprintf(path[i],"%s/%s",dirname,dentry[file[i]]->d_name);
		if(stat(path[i],&s[i])<0){
			fprintf(stderr, "stat error\n");
			return;
		}
		ptr=dentry[file[i]]->d_name+strlen(dentry[file[i]]->d_name)-12;
		strcpy(buf[i],ptr);
	}

	printf("0.   exit\n");
	for(int i=0;i<num;i++){
		char sp[5];
		sprintf(sp,"%d.",i+1);
		printf("%-5s%s\t%ldbytes\n",sp,buf[i],s[i].st_size);
	}
	printf("Choose file to recover : ");
	fgets(choose2,sizeof(choose2),stdin);
	choose=atoi(choose2);

	if(choose==0)
		return;
	else{
		if(dummy_command2!=NULL){
			sprintf(system_command,"cp %s %s", path[choose-1],filename2);
		}
		else{
			sprintf(system_command,"cp %s %s",path[choose-1],filename);
		}
		system(system_command);
		printf("Recovery success\n");
		printLog(filename,4);
		if(dummy_command2!=NULL){
			recoverFileRead(filename2);
		}
		else{
			recoverFileRead(filename);
		}
	}

	for(int i=0;i<count;i++)
		free(dentry[i]);
	free(dentry);
	return;
}

void command_list(){
	print(&list);
}

void printLog(char *name, int index){
	time_t timer = time(NULL);
	struct tm *t = localtime(&timer);
	char temp_d[7];
	char temp_t[7];
	sprintf(temp_d,"%02d%02d%02d",t->tm_year-100,t->tm_mon+1,t->tm_mday);
	sprintf(temp_t,"%02d%02d%02d",t->tm_hour, t->tm_min, t->tm_sec);

	FILE *fd;
	char logName[300];
	char logContext[1024];
	Node *temp;
	char backupName[300];

	sprintf(logName,"%s/log.txt",dirname);
	if((fd=fopen(logName,"a+"))==NULL){
		fprintf(stderr, "fopen error\n");
		exit(1);
	}

	switch(index){
		case 1:
			temp = (Node*)malloc(sizeof(Node*));
			temp = searchNode(&list,name);
			sprintf(backupName,"%s/%s_%s%s",dirname,temp->filename,temp->temp_d,temp->temp_t);

			sprintf(logContext,"[%s %s] %s generated\n",temp->temp_d,temp->temp_t,backupName);
			fputs(logContext,fd);
			break;
		case 2:
			sprintf(logContext,"[%s %s] %s added\n",temp_d,temp_t,name);
			fputs(logContext,fd);
			break;
		case 3:
			sprintf(logContext,"[%s %s] %s deleted\n",temp_d,temp_t,name);
			fputs(logContext,fd);
			break;
		case 4:
			sprintf(logContext,"[%s %s] %s recovered\n",temp_d,temp_t,name);
			fputs(logContext,fd);
			break;
	}
	fclose(fd); 
}

void *backupFile(void *arg){
	pthread_t tid;
	Node *tempNode = (Node*)arg;
	char backupName[300];
	char commandname[MAX_SIZE];
	struct stat s;
	int count=0;
	struct dirent **dentry;
	char path[MAX_SIZE]; 
	int file[MAX_SIZE]; 
	char tempName3[100];
	char fpath[MAX_SIZE];
	char *ptr;
	char *tptr;
	char buf[200]; 
	char buf_h[20];
	char buf_m[20];
	char buf_s[20];
	char buf_d[20];

	tid=pthread_self(); 
	tempNode->tid = tid;

	while(1){
		int buf_t=0;
		int tmp_t=0;
		int num=0; 
		sleep(tempNode->period);

		pthread_mutex_lock(&mutex); 

		time_t timer = time(NULL);
		struct tm *t = localtime(&timer);
		char temp_d[7];
		char temp_t[7];

		if(stat(tempNode->file,&s)<0){
			fprintf(stderr, "stat error\n");
			exit(1);
		}
		sprintf(temp_d,"%02d%02d%02d",t->tm_year-100,t->tm_mon+1,t->tm_mday);
		sprintf(temp_t,"%02d%02d%02d",t->tm_hour, t->tm_min, t->tm_sec);
		strcpy(tempNode->temp_d, temp_d);
		strcpy(tempNode->temp_t, temp_t);

		sprintf(backupName,"%s/%s_%s%s",dirname,tempNode->filename,tempNode->temp_d,tempNode->temp_t);
		sprintf(commandname,"cp %s %s",tempNode->file,backupName);

		if(tempNode->list_m_flag){
			if(tempNode->mtime_record!=s.st_mtime){
				system(commandname);
				printLog(tempNode->file,1);
				tempNode->mtime_record = s.st_mtime;
			}
		}
		else{
			system(commandname);
			printLog(tempNode->file,1);
		}
		if(tempNode->list_t_flag){

			if((count = scandir(dirname,&dentry, NULL,alphasort)) == -1){
				fprintf(stderr, "scandir error\n");
				exit(1);
			}

			sprintf(tempName3,"/%s_",tempNode->filename);
			for(int i=0;i<count;i++){
				realpath(dentry[i]->d_name,fpath);
				if(strstr(fpath,tempName3)){
					file[num++]=i;
				}
			}

			for(int i=0;i<num;i++){
				sprintf(path,"%s/%s",dirname,dentry[file[i]]->d_name);
				ptr=dentry[file[i]]->d_name+strlen(dentry[file[i]]->d_name)-8;

				strcpy(buf,ptr);
				strcpy(buf_d,buf);
				tptr=buf+strlen(buf)-6;
				strcpy(buf_h,tptr);
				tptr=buf+strlen(buf)-4;
				strcpy(buf_m,tptr);
				tptr=buf+strlen(buf)-2;
				strcpy(buf_s,tptr);
				buf_d[2]='\0';
				buf_h[2]='\0';
				buf_m[2]='\0';

				buf_t = (atoi(buf_d)*86400)+(atoi(buf_h)*3600)+(atoi(buf_m)*60)+atoi(buf_s);

				tptr=temp_d+strlen(temp_d)-2;
				strcpy(buf_d,tptr);
				strcpy(buf_h,temp_t);
				tptr=temp_t+strlen(temp_t)-4;
				strcpy(buf_m,tptr);
				tptr=temp_t+strlen(temp_t)-2;
				strcpy(buf_s,tptr);
				buf_h[2]='\0';
				buf_m[2]='\0';
				tmp_t = (atoi(buf_d)*86400)+(atoi(buf_h)*3600)+(atoi(buf_m)*60)+atoi(buf_s);

				if((tmp_t - buf_t)>=(tempNode->addTime)){
					remove(path);
				}
			}


			for(int i=0;i<count;i++)
				free(dentry[i]);
			free(dentry);
		}

		if(tempNode->list_n_flag){
			optionAddN(tempNode->filename,tempNode->number);
		}
		pthread_mutex_unlock(&mutex); 

	}
	return NULL;
}

void optionAddN(char *filename,int number){
	int count=0;
	int num=0;
	struct dirent **dentry;
	char *ptr;
	char path[MAX_SIZE]; 
	int file[MAX_SIZE]; 
	char buf[20];
	struct stat s;
	char tempName3[400];
	char fpath[MAX_SIZE];

	if((count = scandir(dirname,&dentry, NULL,alphasort)) == -1){
		fprintf(stderr, "scandir error\n");
		exit(1);
	}

	sprintf(tempName3,"/%s_",filename);
	for(int i=0;i<count;i++){
		realpath(dentry[i]->d_name,fpath);
		if(strstr(fpath,tempName3))
			file[num++]=i;
	}

	for(int i=0;i<num-number;i++){
		sprintf(path,"%s/%s",dirname,dentry[file[i]]->d_name);
		ptr=dentry[file[i]]->d_name+strlen(dentry[file[i]]->d_name)-12;
		strcpy(buf,ptr);
		remove(path);
	}

	for(int i=0;i<count;i++)
		free(dentry[i]);
	free(dentry);

	return;
}

void optionAddD(char *filename,int period, char*option){
	struct dirent **dentry;
	pthread_t tid;
	int nitems;
	char pathname[MAX_SIZE];
	struct stat fstat;

	char add_command_d[MAX_SIZE];
	nitems=scandir(filename,&dentry,NULL,alphasort);

	for(int i=2;i<nitems;i++){

		sprintf(pathname,"%s/%s",filename,dentry[i]->d_name);
		stat(pathname, &fstat);

		if(S_ISDIR(fstat.st_mode))	{
			optionAddD(pathname,period,option);
		}
		else if(S_ISREG(fstat.st_mode)){
			if(isInList(&list,pathname)){
				continue;
			}
			sprintf(add_command_d,"add %s %d%s",pathname,period,option);
			command_add(add_command_d);
		}
	}
}

void recoverFileRead(char* fname){
	char c_str_read[2048];
	FILE *fd;
	int cLine=0;

	printf("\n");
	if((fd=fopen(fname, "r"))==NULL){
		fprintf(stderr, "fopen error for %s\n",fname);
		exit(1);
	}
	else{
		while(1){
			fgets(c_str_read,2048,fd);
			if(feof(fd))
				break;
			printf("%-4d %s",cLine+1,c_str_read);
			cLine++;
		}
	}
	printf("\n");
	fclose(fd);
}

void makeTable() {
	FILE *pFile;
	char pFilePath[MAX_SIZE];
	sprintf(pFilePath, "%s/listLog.csv", dirname);
	Node *current = list.head;

	pFile = fopen(pFilePath, "w");

	while(current != NULL){
		fprintf(pFile,"%s,%d,%s,%s,%d,%d,%d,%d,%d,%d\n",current->file,current->period,current->opt,current->filename,current->mtime_record,current->list_m_flag,current->list_t_flag,current->addTime,current->list_n_flag,current->number);
		current=current->next;
	}
	fclose(pFile);
}

void readTable(){
	FILE *pFile;
	char pFilePath[MAX_SIZE];
	char strA[MAX_SIZE];
	char strB[MAX_SIZE];
	char file[256];
	int period;
	char opt[100];
	char filename[100];
	int mtime_record;
	int list_m_flag;
	int list_t_flag;
	int addTime;
	int list_n_flag;
	int number;

	sprintf(pFilePath, "%s/listLog.csv", dirname);
	pFile = fopen(pFilePath, "r");

	while(fgets(strA,sizeof(strA),pFile)!=NULL){
		strcpy(strB,strtok(strA,","));
		strcpy(file, strB);

		strcpy(strB,strtok(NULL,","));
		period=atoi(strB);

		strcpy(strB,strtok(NULL,","));
		strcpy(opt, strB);

		strcpy(strB,strtok(NULL,","));
		strcpy(filename, strB);

		strcpy(strB,strtok(NULL,","));
		mtime_record=atoi(strB);

		strcpy(strB,strtok(NULL,","));
		list_m_flag=atoi(strB);

		strcpy(strB,strtok(NULL,","));
		list_t_flag=atoi(strB);

		strcpy(strB,strtok(NULL,","));
		addTime=atoi(strB);

		strcpy(strB,strtok(NULL,","));
		list_n_flag=atoi(strB);

		strcpy(strB,strtok(NULL,","));
		number=atoi(strB);

		InsertNode(&list,file,period,opt,filename,mtime_record,list_m_flag,list_t_flag,addTime,list_n_flag,number);

		if(strtok(NULL,",")=="\n")
			continue;
	}
	fclose(pFile);
}
