#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#define NA_STATEK 3
#define ZE_STATKU 1


int dziala = 1;
int mostek;

// Struktura pasażera
struct pasazer {
    long type;
    pid_t pas_pid;
};

#define ROZMIAR_PASAZERA (sizeof(pid_t))

// Obsługa sygnału SIGINT
void handler(int sig) {
    printf("Sygnal SIGINT! Zakonczenie dzialania\n");
    dziala = 0;
    return;
}

int main() {
    int liczba_pasazerow=0;
    pid_t pasazerowie[5]; //ZAŁOŻENIE DO TESTOW ZE POJEMNOSC STATKU TO 5
    struct pasazer pass;

    // Ustawienie obsługi sygnału
    signal(SIGINT, handler);

    // Tworzenie kolejki komunikatów
    if ((mostek = msgget(123, IPC_CREAT | 0666)) == -1) {
        perror("Blad tworzenia kolejki");
        exit(EXIT_FAILURE);
    }
    printf("ID kolejki: %d\n", mostek);

    while (liczba_pasazerow < 5 && dziala) {
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
        sleep(10); // Symulacja rejsu
        printf("Rejs zakończony\n\n");

        // Wypuszczenie pasażerow ze statku
        for (int i = 0; i < liczba_pasazerow; i++) {
        pass.type = ZE_STATKU;
        pass.pas_pid = pasazerowie[i];
        if (msgsnd(mostek, &pass, ROZMIAR_PASAZERA, 0) == -1) {
            perror("Blad wyslania pasażera ze statku");
        } else {
            printf("Kapitan wypuścił ze statku pasażera %d\n", pass.pas_pid);
        }
    }  

        sleep(5); // Oczekiwanie przed kolejnym pasażerem

    // Usuwanie kolejki (jeśli nie usunięto w handlerze)
    if (msgctl(mostek, IPC_RMID, NULL) == -1) {
        perror("Blad usuniecia kolejki");
    }

    return 0;
}
