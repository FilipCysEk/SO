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

/*
 * Kończenie działania programu
 * End work of program
 */

void exitServer(int sig){
	if(sig == SIGINT || sig == -1){
		if(sig == -1)
			printToScreen("Kończenie działania programu w wyniku błędu");
		else
			printToScreen("Kończenie działania serwera");

		if(idMsg != -1){
			if(msgctl(idMsg, IPC_RMID, 0) == -1){
				printToScreen("Błąd usuwania kolejki");
				exit(2);
			}
			printToScreen("Usówanie kolejki!!");
		}
		else{
			printToScreen("Nie wykryto kolejki, kończenie działania programu");
		}
		exit(2);
	}
}


int main(){
  /*
   * Tworzenie kolejki
   * Creating msg
   */
   struct msgVal kom;
   signal(SIGINT, exitServer);

   if(key < 0){
     if((key = ftok(".", 'j')) == -1){
       printToScreen("Błąd tworzenia klucza");
       exit(1);
     }
   }

   if((idMsg = msgget(key, 0660|IPC_CREAT|IPC_EXCL)) == -1){
     printToScreen("Błąd tworzenia kolejki");
     exit(1);
   }

   /*
    * Proces serwera
    * Server process
    */
		int strLen;

    printToScreen("Serwer poprawnie uruchomiony");
    while(1){
      kom.mtype = SERVER_ID;
      if(msgrcv(idMsg, (struct msgVal *)&kom, MAX_LINE_SIZE + sizeof(long), kom.mtype, 0) == -1){
        printToScreen("Błąd pobierania danych z kolejki");
				exitServer(-1);
      }

			snprintf(tempText, sizeof(tempText), "Klient: %ld - Przesłał tekst %s", kom.mtypeClnt, kom.text);
			printToScreen(tempText);

			strLen = strlen(kom.text);

			for(int i = 0; i < strLen; i++){
				kom.text[i] = toupper(kom.text[i]);
			}

			kom.mtype = kom.mtypeClnt;

			if(msgsnd(idMsg, (struct msgVal *)&kom, sizeof(kom.text) + sizeof(long), 0) == -1){
				printToScreen("Błąd wysyłania komunikatu");
				exitServer(-1);
			}


    }

  return 0;
}
