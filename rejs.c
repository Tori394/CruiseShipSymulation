#include "rejs.h"

int wyswietl_bledy = 1; // Flaga wyświetlania błędów

// Funkcja do tworzenia semafora
int utworz_semafor(key_t klucz, int nr) {
    int s = semget(klucz, nr, 0600 | IPC_CREAT);
    if (s == -1) {
        if (wyswietl_bledy) {
            perror("Nie udało się utworzyć semafora");
        }
        exit(EXIT_FAILURE);
    }
    return s;
}

// Funkcja do ustawiania wartości semafora
void ustaw_wartosc_semafora(int wartosc, int nr, int sem) {
    if (semctl(sem, nr, SETVAL, wartosc) == -1) {
        if (wyswietl_bledy) {
            perror("Błąd ustawienia semafora");
        }
        exit(EXIT_FAILURE);
    }
    return;
}

// Funkcja do sprawdzania wartości semafora
int sprawdz_wartosc_semafora(int nr, int s) {
    return semctl(s, nr, GETVAL);
}

// Funkcja do czyszczenia zasobów przed zakończeniem
void zakoncz(int kolejka, int semafor, pid_t pid) {
    wyswietl_bledy = 0;
    msgctl(kolejka, IPC_RMID, NULL);
    semctl(semafor, 0, IPC_RMID);
    unlink("./fifo2");
    kill(pid, SIGINT);
    return;
}

// Funkcja do otwierania FIFO w określonym trybie
void otworz_fifo(const char *fifo_path, int *fd, int mode) {
    *fd = open(fifo_path, mode);
    if (*fd == -1) {
        if (wyswietl_bledy) {
            perror("Błąd otwarcia FIFO");
        }
        exit(EXIT_FAILURE);
    }
    return;
}

// Funkcja do odbierania PID z FIFO
pid_t odbierz_pid(const char *fifo_path) {
    pid_t pid;
    int fd;
    otworz_fifo(fifo_path, &fd, O_RDONLY);

    if (read(fd, &pid, sizeof(pid_t)) == -1) {
        if (wyswietl_bledy) {
            perror("Błąd odczytu PID");
        }
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
    return pid;
}

// Funkcja do wysyłania PID do FIFO
void wyslij_pid(pid_t pid, const char *fifo_path) {
    int fd;
    otworz_fifo(fifo_path, &fd, O_WRONLY);

    if (write(fd, &pid, sizeof(pid_t)) == -1) {
        if (wyswietl_bledy) {
            perror("Błąd wysyłania PID");
        }
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
    return;
}

// Funkcja do wysyłania sygnału do procesu
void wyslij_sygnal(pid_t pid, int sygnal) {
    if (kill(pid, sygnal) == -1) {
        if (wyswietl_bledy) {
            //perror("Błąd wysyłania sygnału");
        }
        kill(getpid(), SIGINT);
    }
    return;
}

// Funkcja do łączenia z kolejką wiadomości
int polacz_kolejke(int s) {
    int m = msgget(123, 0600);
    if (m == -1) {
        semctl(s, 0, IPC_RMID);
        exit(EXIT_FAILURE);
    }
    return m;
}
