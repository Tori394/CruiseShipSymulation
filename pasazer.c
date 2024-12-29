#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#define NA_STATEK 3
#define NA_STATEK_VIP 2
#define ZE_STATKU 1


int dziala = 1;
int zaladunek = 1;
int rozladunek = 0;
int mostek;

// Struktura pasazera
struct pasazer {
    long type;
    pid_t pas_pid;
};

#define ROZMIAR_PASAZERA (sizeof(pid_t))


void handler(int sig) {
    printf("Rejsy wstrzymane\n");
    dziala = 0;
    zaladunek = 0;
}

void *watek1(void *x) { //wejdz na statek
    struct pasazer pass;
    pass.type = NA_STATEK;

    //while (zaladunek && dziala) 
   // {
        pass.pas_pid=getpid();
        printf("Pasazer %d wsiada na statek\n", pass.pas_pid);

        while (msgsnd(mostek, &pass, ROZMIAR_PASAZERA, 0) == -1)
        {
            if (errno == EAGAIN) {
                printf("Mostek jest pełny, oczekiwanie...\n");
                sleep(1);
            } else {
                fprintf(stderr, "Pasazer %d nie moze wejsc na mostek\n", pass.pas_pid);
                return NULL;
            }
        }
   // }
    pthread_exit(NULL);
}

void *watek2(void *x) //zejscie ze statku
{
    struct pasazer pass;
    pass.type = ZE_STATKU;

    //while (rozladunek) 
   // {
        if (msgrcv(mostek, &pass, ROZMIAR_PASAZERA, ZE_STATKU, 0) == -1) 
        {
    //        if (errno == EINTR) continue; // Ignoruj przerwania
            perror("Pasazer nie mogl wyjsc ze statku");
     //       continue;
        }
        printf("Pasazer %d zszedl na lad\n", pass.pas_pid);
//}
    pthread_exit(NULL);
}


int main() {
    pthread_t w1, w2;

    signal(SIGINT, handler);

    if ((mostek = msgget(123, IPC_CREAT | 0666)) == -1) {
        perror("Błąd tworzenia mostka");
        exit(EXIT_FAILURE);
    }
    printf("ID kolejki - mostek: %d\n", mostek);

    pthread_create(&w1, NULL, watek1, NULL);
    pthread_create(&w2, NULL, watek2, NULL);

    pthread_join(w1, NULL);
    pthread_join(w2, NULL);

    return 0;
}
