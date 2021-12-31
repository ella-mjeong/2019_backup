#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct Node{
	char file[256];
	int period;
	char opt[100];
	char temp_d[7];
	char temp_t[7];
	char filename[100];
	int mtime_record;
	int list_m_flag;
	int list_t_flag;
	int addTime;
	int list_n_flag;
	int number;
	pthread_t tid;
	struct Node *next;
}Node;

typedef struct{
	Node *head; 
	int size; 
} LinkedList;

void Init(LinkedList *list){
	list->head = NULL;
	list->size = 0;
}

int getSize(LinkedList *list){
	return list->size;
}

Node* create(char *file, int period, char *opt,char *filename,int mtime,int list_m_flag,int list_t_flag,int addTime,int list_n_flag, int number){
	Node *newNode = (Node*)malloc(sizeof(Node));
	if(newNode == NULL)
	{
		printf("Error: creating a new node.\n");
		exit(1);
	}
	strcpy(newNode->file,file);
	newNode->period = period;
	strcpy(newNode->opt , opt);
	strcpy(newNode->filename, filename);
	newNode->mtime_record = mtime;
	newNode->list_m_flag = list_m_flag;
	newNode->list_t_flag=list_t_flag;
	newNode->addTime = addTime;
	newNode->list_n_flag = list_n_flag;
	newNode->number = number;
	return newNode;
}

void addPos(LinkedList *list, int pos, char *file, int period, char *opt, char *filename,int mtime, int list_m_flag,int list_t_flag,int addTime, int list_n_flag, int number){
	if(pos > (list->size)+1 || pos < 1){
		printf("Error: Position is out of range!\n");
		exit(1);
	}
	else{
		Node* newNode = create(file, period, opt, filename,mtime,list_m_flag,list_t_flag,addTime, list_n_flag, number);

		if(pos == 1){
			newNode->next = list->head;
			list->head = newNode;
		}
		else{
			Node *temp = list -> head;
			for(int i=1;i<pos-1;i++)
				temp = temp->next;

			newNode->next = temp->next;
			temp->next = newNode;
		}
		(list->size)++;
	}
}

void InsertNode(LinkedList *list, char *file, int period, char *opt,char *filename,int mtime,int list_m_flag,int list_t_flag,int addTime, int list_n_flag, int number){

	addPos(list, (list->size)+1,file, period, opt,filename,mtime,list_m_flag,list_t_flag,addTime,list_n_flag, number);
}

void delNode(LinkedList *list, char*file){
	Node* cursor = list->head;
	Node* prev = NULL;
	if(!strcmp(cursor->file, file)){
		list->head = cursor->next;
		free(cursor);
		(list->size)--;
	}
	else{
		while(cursor != NULL){
			if(!strcmp(cursor->file, file)) break;
			prev = cursor;
			cursor = cursor->next;
		}
		if(cursor != NULL){
			prev->next = cursor->next;
			free(cursor);
			(list->size)--;
		}
	}
}

void delAllNode(LinkedList *list){
	Node *temp;
	Node *cursor = list->head;
	list->head = NULL;

	while(cursor != NULL){
		temp = cursor->next;
		free(cursor);
		cursor = temp;
	}
	list->size = 0;
}

bool isEmpty(LinkedList *list){
	return (list->head == NULL);
}

bool isInList(LinkedList *list, char*file){
	Node *cursor = list->head;
	while(cursor != NULL){
		if(!strcmp(cursor->file, file)) return true;
		cursor = cursor->next;
	}
	return false;
}

void print(LinkedList *list){
	Node *current = list->head;
	while(current != NULL){
		printf("%s %7d  %-15s\n",current->file,current->period,current->opt);
		current = current->next;
	}
}

Node* searchNode(LinkedList *list, char*file){
	Node* cursor = list->head;

	if(!strcmp(cursor->file, file)){
		return cursor;
	}
	else{
		while(cursor != NULL){
			if(!strcmp(cursor->file, file)) break;
			cursor = cursor->next;
		}
		if(cursor != NULL){
			return cursor;
		}
	}
	return NULL;
}