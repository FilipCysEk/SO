#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <pthread.h>

#include "printtoscreen.c"

#define MAX_LINE_SIZE 256
#define SERVER_ID 1
//klient ma PID

struct msgVal{
  long mtype;
  long mtypeClnt;
  char text[MAX_LINE_SIZE];
};

key_t key = 1441;
int idMsg = -1;
int pid;

void * clientSender(){
	struct msgVal kom;
	kom.mtype = SERVER_ID;
	kom.mtypeClnt = pid;

	while(1){
		printf("Napisz komunikat\n");
		scanf("%s", kom.text);

		//if(msgsnd(idMsg, (struct msgVal *)&kom, sizeof(kom.text) + sizeof(long), 0) == -1){
		if(msgsnd(idMsg, (struct msgVal *)&kom, strlen(kom.text) + 1 + sizeof(long), 0) == -1){
			printToScreen("Błąd wysyłania komunikatu");
			exit(4);
		}
	}

	pthread_exit((void *)0);
}

void * clientReciver(){
	struct msgVal kom;
	kom.mtype = pid;

	while(1){
		if(msgrcv(idMsg, (struct msgVal *)&kom, MAX_LINE_SIZE + sizeof(long), kom.mtype, 0) == -1){
			printToScreen("Błąd pobierania danych z kolejki");
			exit(3);
		}

		printf("Serwer zwrócił: %s\n", kom.text);
	}

	pthread_exit((void *)0);
}


int main(){
  /*
   * Tworzenie kolejki
   * Creating msg
   */

	 pid = getpid();

   if(key < 0){
     if((key = ftok(".", 'j')) == -1){
       printToScreen("Błąd tworzenia klucza");
       exit(1);
     }
   }

   if((idMsg = msgget(key, 0660|IPC_CREAT)) == -1){
     printToScreen("Błąd tworzenia kolejki");
     exit(1);
   }

   /*
    * Proces serwera
    * Server process
    */

    printToScreen("Klient poprawnie uruchomiony");

		pthread_t sender, reciver;

		if(pthread_create(&sender, NULL, clientSender, NULL)){
			printToScreen("Błąd tworzenia wątku wysyłającego");
			exit(2);
		}

		if(pthread_create(&reciver, NULL, clientReciver, NULL)){
			printToScreen("Błąd tworzenia wątku odbierającego");
			exit(2);
		}

		if(pthread_join(sender, NULL)){
			printToScreen("Błąd przyłączania wątku wysyłającego");
			exit(3);
		}

		if(pthread_join(reciver, NULL)){
			printToScreen("Błąd przyłączania wątku odbierającego");
			exit(3);
		}

  return 0;
}
