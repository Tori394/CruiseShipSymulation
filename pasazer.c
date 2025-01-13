#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/sem.h>

#define NA_STATEK 3
#define ZE_STATKU 1
#define ROZMIAR_PASAZERA (sizeof(pid_t))

int dziala = 1;
int mostek;  //kolejka komunikatow
int szlaban; //semafor

// Struktura pasażera
struct pasazer {
    long type;
    pid_t pas_pid;
};

// Obsługa sygnału SIGINT
void handler(int sig) {
    printf("Sygnal SIGINT! Koniec działania\n");
    dziala = 0;
    exit(EXIT_SUCCESS);
}

static void przejdz_przez_szlaban(int nr)
  {
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num=nr;
    bufor_sem.sem_op=-1;
    bufor_sem.sem_flg=0;
    zmien_sem=semop(szlaban,&bufor_sem,1);
    if (zmien_sem==-1)
      {
        printf("Nie moglem zamknac semafora\n");
        exit(EXIT_FAILURE);
      }
  }

int main() {
    struct pasazer pass;

    // Obsługa sygnałów
    signal(SIGINT, handler);

    // Połączenie z kolejką komunikatów
    if ((mostek = msgget(123, 0666)) == -1) {
        perror("Blad dolaczenia do kolejki\n");
        exit(EXIT_FAILURE);
    }

    if ((szlaban=semget(100, 1, 0600|IPC_CREAT))==-1) 
    {
        printf("Nie udalo sie utworzyc semafora\n");
        exit(EXIT_FAILURE);
    }

    //5 pasażerów
    for(int i=0; i<20; i++) {
        if(fork() == 0) {  // Proces dziecka (pasażer)
            pass.type = NA_STATEK;
            pass.pas_pid = getpid();

            // Próba wejścia na statek
            printf("Pasażer %d Czeka na wejście na statek...\n", pass.pas_pid);
            przejdz_przez_szlaban(0);
            if (msgsnd(mostek, &pass, ROZMIAR_PASAZERA, 0) == -1) {
                perror("Nie udalo sie wejsc na statek\n");
                exit(EXIT_FAILURE);
            }
            printf("Pasażer %d: Wszedłem na mostek\n", pass.pas_pid);

            // Czekanie na zejście ze statku
            if (msgrcv(mostek, &pass, ROZMIAR_PASAZERA, ZE_STATKU, 0) == -1) {
                perror("Blad przy czekaniu na zejscie ze statku\n");
                exit(EXIT_FAILURE);
            }
            printf("Pasażer %d: Zszedłem na ląd\n", pass.pas_pid);

            exit(0);  // Kończymy proces dziecka
        }
    }

    // Czekanie na zakończenie wszystkich procesów dzieci
    for(int i=0; i<5; i++) {
        wait(NULL);
    }

    return 0;
}
