#include "rejs.h"

// Funkcja do tworzenia semafora
int utworz_semafor(key_t klucz, int nr) {
    int s = semget(klucz, nr, 0600 | IPC_CREAT);
    if (s == -1) {
        perror("Nie udało się utworzyć semafora");
        exit(EXIT_FAILURE);
    }
    return s;
}


// Funkcja do ustawiania wartości semafora
void ustaw_wartosc_semafora(int wartosc, int nr, int sem) {
    if (semctl(sem, nr, SETVAL, wartosc) == -1) {
        perror("Błąd ustawienia semafora");
        exit(EXIT_FAILURE);
    }
}

// Funkcja do sprawdzania wartości semafora
int sprawdz_wartosc_semafora(int nr, int s) {
    int val = semctl(s, nr, GETVAL);
    return val;
}

// Funkcja do czyszczenia zasobów przed zakończeniem
void zakoncz(int kolejka, int semafor) {
    if (msgctl(kolejka, IPC_RMID, NULL) == -1) {
        perror("Błąd usunięcia kolejki\n");
    }
    if (semctl(semafor, 0, IPC_RMID) == -1) {
        perror("Błąd usuwania semafora\n");
    }
    return;
}

void otworz_fifo(const char *fifo_path, int *fd, int mode) {
    *fd = open(fifo_path, mode);
    if (*fd == -1) {
        perror("Błąd otwarcia FIFO");
        exit(EXIT_FAILURE);
    }
}

pid_t odbierz_pid(const char *fifo_path) {
    pid_t pid;
    int fd;
    otworz_fifo(fifo_path, &fd, O_RDONLY);

    if (read(fd, &pid, sizeof(pid_t)) == -1) {
        perror("Błąd odczytu PID");
        close(fd);
        exit(EXIT_FAILURE);
    }
    printf("Odebrano PID: %d\n", pid);
    close(fd);
    return pid;
}

void wyslij_pid(pid_t pid, const char *fifo_path) {
    int fd;
    otworz_fifo(fifo_path, &fd, O_WRONLY);

    if (write(fd, &pid, sizeof(pid_t)) == -1) {
        perror("Błąd zapisu PID");
        close(fd);
        exit(EXIT_FAILURE);
    }
    printf("Wysłano PID: %d\n", pid);
    close(fd);
}


