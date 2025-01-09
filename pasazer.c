#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define NA_STATEK 3
#define ZE_STATKU 1
#define ROZMIAR_PASAZERA (sizeof(pid_t))

int dziala = 1;
int mostek;

// Struktura pasażera
struct pasazer {
    long type;
    pid_t pas_pid;
};

// Obsługa sygnału SIGINT
void handler(int sig) {
    printf("Sygnal SIGINT! Koniec działania\n", getpid());
    dziala = 0;
    exit(EXIT_SUCCESS);
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

    pass.type = NA_STATEK;
    pass.pas_pid = getpid();

    // Próba wejścia na statek
    printf("Pasażer %d Czeka na wejście na statek...\n", pass.pas_pid);
    if (msgsnd(mostek, &pass, ROZMIAR_PASAZERA, 0) == -1) {
        perror("Pasażer %d: Nie udalo sie wejsc na statek\n", pass.pas_pid);
        exit(EXIT_FAILURE);
    }
    printf("Pasażer %d: Wszedłem na statek\n", pass.pas_pid);

    // Czekanie na zejście ze statku
    if (msgrcv(mostek, &pass, ROZMIAR_PASAZERA, ZE_STATKU, 0) == -1) {
        perror("Blad przy czekaniu na zejscie ze statku\n");
        exit(EXIT_FAILURE);
    }
    printf("Pasażer %d: Zszedłem ze statku\n", pass.pas_pid);

    return 0;
}
