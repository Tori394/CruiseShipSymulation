#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
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
    printf("Sygnal SIGINT! Zakonczenie dzialania\n");
    dziala = 0;
    if (msgctl(mostek, IPC_RMID, NULL) == -1) {
        perror("Blad usuniecia kolejki\n");
    }
    if ((semctl(szlaban,0,IPC_RMID))==-1)
    {
        printf("Blad usuwania semafora\n");
        exit(EXIT_FAILURE);
    }
    exit(1);
}

int main() {
    int liczba_pasazerow=0;
    int pojemnosc_statku=7;
    int czas_rejsu=10;
    pid_t pasazerowie[pojemnosc_statku]; //ZAŁOŻENIE DO TESTOW ZE POJEMNOSC STATKU TO 7
    struct pasazer pass;
    struct msqid_ds buf;

    // Ustawienie obsługi sygnału
    signal(SIGINT, handler);

    // Tworzenie kolejki komunikatów
    if ((mostek = msgget(123, IPC_CREAT | 0666)) == -1) {
        perror("Blad tworzenia kolejki\n");
        exit(EXIT_FAILURE);
    }
    // Tworzenie semafora
    if ((szlaban=semget(100, 1, 0600|IPC_CREAT))==-1) 
    {
        printf("Nie udalo sie utworzyc semafora\n");
        exit(EXIT_FAILURE);
    }
    if (semctl(szlaban, 0, SETVAL, pojemnosc_statku) == -1) {
        perror("Blad ustawienia semafora\n");
        exit(EXIT_FAILURE);
    }

    while(dziala)
    {
    while (liczba_pasazerow < pojemnosc_statku) {
        if (msgrcv(mostek, &pass, ROZMIAR_PASAZERA, NA_STATEK, 0) == -1) {
            if (errno == EINTR) continue; // Ignorowanie przerwań
            perror("Blad pobrania pasazera na statek");
            break;
        }
        pasazerowie[liczba_pasazerow++] = pass.pas_pid; // Zapis PID pasażera
        printf("Kapitan wpuscil na statek pasażera %d\n", pass.pas_pid);


    }

        // Symulacja rejsu
        printf("\nRejs z %d pasażerami się rozpoczął\n", liczba_pasazerow);
        sleep(czas_rejsu); // Symulacja rejsu
        printf("Rejs zakończony\n\n");

        // Wypuszczenie pasażerow ze statku
        for (int i = 0; i <= liczba_pasazerow; i++) {
        pass.type = ZE_STATKU;
        pass.pas_pid = pasazerowie[i];
        if (msgsnd(mostek, &pass, ROZMIAR_PASAZERA, 0) == -1) {
            perror("Blad wyslania pasażera ze statku");
        } else {
            //printf("Kapitan wypuścił ze statku pasażera %d\n", pass.pas_pid);
        }
    }  
        liczba_pasazerow=0;

        while (1) {
        if (msgctl(mostek, IPC_STAT, &buf) == -1) {
            perror("Blad pobrania informacji o kolejce\n");
            exit(EXIT_FAILURE);
        }
        if (buf.msg_qnum == 0) {
            printf("Mostek jest pusty\n");
            break; // Sprawdz czy kolejka jest pusta
        }
        //printf(buf.msg_qnum);
        sleep(1); // Odczekanie przed ponownym sprawdzeniem
        }

        printf("Szlaban sie otwiera...\n");
        sleep(2);
        if (semctl(szlaban, 0, SETVAL, pojemnosc_statku) == -1) {
        perror("Blad ustawienia semafora\n");
        exit(EXIT_FAILURE);
        }
        
    }

    return 0;
}
