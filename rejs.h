#ifndef REJS_H
#define REJS_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/sem.h>
#include <time.h>
#include <sys/wait.h>

#define NA_STATEK 3
#define ZE_STATKU 1
#define ROZMIAR_PASAZERA (sizeof(pid_t))
#define MIEJSCE_NA_MOSTKU 0
#define SZLABAN 1

int mostek;  // Kolejka komunikatów
int szlabany; // Semafor

// Struktura pasażera
struct pasazer {
    long type;
    pid_t pas_pid;
};

// Funkcja do tworzenia semafora
int utworz_semafor(key_t klucz, int nr) {
    szlabany = semget(klucz, nr, 0600 | IPC_CREAT);
    if (szlabany == -1) {
        perror("Nie udało się utworzyć semafora");
        exit(EXIT_FAILURE);
    }
    return szlabany;
}

// Funkcja do ustawiania wartości semafora
void ustaw_wartosc_semafora(int wartosc, int nr) {
    if (semctl(szlabany, nr, SETVAL, wartosc) == -1) {
        perror("Błąd ustawienia semafora");
        exit(EXIT_FAILURE);
    }
}

// Funkcja do opuszczania semafora (-1)
void opusc_semafor(int nr) {
    struct sembuf op = {nr, -1, 0};
    if (semop(szlabany, &op, 1) == -1) {
        exit(0); // Obsługa błędów
    }
}

// Funkcja do podnoszenia semafora (+1)
void podnies_semafor(int nr) {
    struct sembuf op = {nr, 1, 0};
    if (semop(szlabany, &op, 1) == -1) {
        exit(0); // Obsługa błędów
    }
}

// Funkcja obsługująca sygnał SIGINT
void handler(int sig) {
    exit(EXIT_SUCCESS);
}

// Funkcja do dynamicznego usuwania zakończonych procesów
void usun_podproces_dynamicznie(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

// Funkcja do próby połączenia z kolejką komunikatów
int polacz_z_kolejka(int key) {
    int mostek;
    int i = 0;
    while ((mostek = msgget(key, 0666)) == -1) {
        if (i == 6) exit(1); // Jeśli program kapitana nie uruchomiony w ciągu 30s, zamknij
        i++;
        sleep(5);
    }
    return mostek;
}

// Funkcja do czyszczenia zasobów przed zakończeniem
void zakoncz(int mostek, int szlabany) {
    if (msgctl(mostek, IPC_RMID, NULL) == -1) {
        perror("Błąd usunięcia kolejki\n");
    }
    if (semctl(szlabany, 0, IPC_RMID) == -1) {
        perror("Błąd usuwania semafora\n");
    }
    exit(1);
}

// Funkcja do obsługi sygnału SIGUSR1
void odbierz_sygnal_start(int sig) {
    if (sig == SIGUSR1) {
        printf("Kapitan otrzymał sygnał do startu!\n");
        ustaw_wartosc_semafora(0, SZLABAN);
    }
}

// Funkcja do przekazywania PID
void przekaz_pid(pid_t pid) {
    int fd;

    if (mkfifo("./fifo", 0666) == -1) {
        if (errno != EEXIST) {
            perror("mkfifo");
            exit(1);
        }
    }
    sleep(5);
    fd = open("./fifo", O_WRONLY | O_NONBLOCK);
    if (fd == -1) {
        perror("open");
        exit(1);
    }
    if (write(fd, &pid, sizeof(pid_t)) == -1) {
        perror("write");
        exit(1);
    }

    close(fd);
}

#endif // REJS_H
