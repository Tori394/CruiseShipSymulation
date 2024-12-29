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
#define NA_STATEK_VIP 2
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
    struct pasazer pass;

    // Ustawienie obsługi sygnału
    signal(SIGINT, handler);

    // Tworzenie kolejki komunikatów
    if ((mostek = msgget(123, IPC_CREAT | 0666)) == -1) {
        perror("Blad tworzenia kolejki");
        exit(EXIT_FAILURE);
    }
    printf("ID kolejki: %d\n", mostek);

    while (dziala) {
        printf("Kapitan czeka na pasazerow...\n");

        // Odbieranie pasażera
        if (msgrcv(mostek, &pass, ROZMIAR_PASAZERA, NA_STATEK, 0) == -1) {
            if (errno == EINTR) continue; // Przerwanie przez sygnał
            perror("Blad pobrania pasazera na statek");
            break;
        }

        printf("Kapitan wpuscil na statek %d\n", pass.pas_pid);

        // Symulacja rejsu
        printf("Rejs sie rozpoczal\n");
        sleep(10);
        printf("Rejs sie zakonczyl\n");

        // Wypuszczenie pasażera ze statku
        pass.type = ZE_STATKU;
        printf("Kapitan wypuscil ze statku %d\n", pass.pas_pid);

        if (msgsnd(mostek, &pass, ROZMIAR_PASAZERA, 0) == -1) {
            perror("Blad zejscia ze statku na mostek");
            break;
        }

        sleep(5); // Oczekiwanie przed kolejnym pasażerem
    }

    // Usuwanie kolejki (jeśli nie usunięto w handlerze)
    if (msgctl(mostek, IPC_RMID, NULL) == -1) {
        perror("Blad usuniecia kolejki");
    }

    return 0;
}
