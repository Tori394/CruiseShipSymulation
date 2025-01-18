#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>

pid_t pid_kapitana; 
int czas = 30; // Czas rejsu
int plyn = 1;  // Flaga działania programu

void zakoncz_program(int sig) {
    printf("\nOtrzymano sygnał zakończenia (SIGINT). Kończenie programu...\n");
    plyn = 0;
}

// Funkcja pobierająca PID kapitana z FIFO
void pobierz_dane() {
    int fd = open("./fifo", O_RDONLY);
    if (fd == -1) {
        perror("Błąd otwarcia FIFO");
        exit(EXIT_FAILURE);
    }

    if (read(fd, &pid_kapitana, sizeof(pid_t)) == -1) {
        perror("Błąd odczytu PID");
        close(fd);
        exit(EXIT_FAILURE);
    }

    unlink("./fifo");
    close(fd);
}

// Funkcja wątku, który wysyła sygnały startu co określony czas
void* wyslij_sygnal_start(void* arg) {
    while (plyn) {
        sleep(czas); // Czekaj czas trwania rejsu
        if (kill(pid_kapitana, SIGUSR1) == -1) {
            perror("Błąd wysyłania sygnału startu");
            exit(EXIT_FAILURE);
        }
        printf("Wysłano sygnał startu do kapitana (PID: %d)\n", pid_kapitana);
    }
    return NULL;
}

// Funkcja wątku, który wypatruje burzy i wysyła sygnał zakończenia
void* wyslij_sygnal_stop(void* arg) {
    while (plyn) {
        int czas_oczekiwania = rand() % (czas * 2) + (czas / 2); // Losowanie czasu między czas/2 a czas*2 sekund
        sleep(czas_oczekiwania);

        if (rand() % 10 == 1) { // Szansa na wykrycie burzy (10%)
            printf("Nadchodzi burza! Kapitan portu nakazał zakończenie rejsów!\n");
            if (kill(pid_kapitana, SIGINT) == -1) {
                perror("Błąd wysyłania sygnału zakończenia (SIGINT)");
                exit(EXIT_FAILURE);
            }
            printf("Wysłano sygnał zakończenia (SIGINT) do kapitana (PID: %d)\n", pid_kapitana);
            plyn = 0; // Przerwanie działania programu
        }
    }
    return NULL;
}

int main() {
    pthread_t sygnal_start, sygnal_stop;

    // Inicjalizacja generatora liczb losowych
    srand(time(NULL));

    // Obsługa sygnału SIGINT
    signal(SIGINT, zakoncz_program);

    // Pobranie PID kapitana
    pobierz_dane();
    printf("Odczytano PID kapitana: %d\n", pid_kapitana);

    // Tworzenie wątku wysyłającego sygnał startu
    if (pthread_create(&sygnal_start, NULL, wyslij_sygnal_start, NULL) != 0) {
        perror("Błąd tworzenia wątku sygnału startu");
        exit(EXIT_FAILURE);
    }

    // Tworzenie wątku wypatrującego burzy
    if (pthread_create(&sygnal_stop, NULL, wyslij_sygnal_stop, NULL) != 0) {
        perror("Błąd tworzenia wątku sygnału stop");
        exit(EXIT_FAILURE);
    }

    // Oczekiwanie na zakończenie wątków
    pthread_join(sygnal_start, NULL);
    pthread_join(sygnal_stop, NULL);

    printf("Program zakończony poprawnie.\n");
    return 0;
}
