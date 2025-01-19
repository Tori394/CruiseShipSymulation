#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>
#include <sys/sem.h>

pid_t pid_kapitana; 
int czas = 60; // Czas rejsu
int plyn = 1;  // Flaga działania programu
int szlabany;

void panic_button(int sig) {
    printf("Program przerwany sygnałem nadanym przez uzytkownika\n");
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
    printf("Kaitan portu odczytał PID kapitana: %d\nRozpoczynanie symulacji\n\n", pid_kapitana);

    unlink("./fifo");
    close(fd);
}

void utworz_semafor(key_t klucz, int nr) {
    szlabany = semget(klucz, nr, 0600 | IPC_CREAT);
    if (szlabany == -1) {
        perror("Nie udalo sie utworzyc semafora");
        exit(EXIT_FAILURE);
    }
}

int sprawdz_wartosc_semafora(int nr) {
    int val = semctl(szlabany, nr, GETVAL);
    return val;
}

// Funkcja wątku, który wysyła sygnały startu co określony czas
void* wyslij_sygnal_start(void* arg) {
    while (sprawdz_wartosc_semafora(1)!=-1 && plyn) {
        sleep(czas/2); // Czekaj czas trwania rejsu
        if (sprawdz_wartosc_semafora(1)==0 && sprawdz_wartosc_semafora(0)!=0 && rand() % 2 == 1) { // Jesli statek zapelni sie przed czasem jest 50% szans, że kapitan wysle sygnał do wcześniejszego startu
            if (kill(pid_kapitana, SIGUSR1) == -1) {
                perror("Błąd wysyłania sygnału startu");
                exit(EXIT_FAILURE);
            }
            printf("Kapitan Portu zezwolił na wcześnejsze wypłynięcie\n");
        }
        else {
            sleep(czas/2);
            if (kill(pid_kapitana, SIGUSR1) == -1) {
                perror("Błąd wysyłania sygnału startu");
                exit(EXIT_FAILURE);
            }
            printf("Kapitan Portu nadał sygnał do startu\n");
        }
    }
    return NULL;
}

// Funkcja wątku, który wypatruje burzy i wysyła sygnał zakończenia
void* wyslij_sygnal_stop(void* arg) {
    int czas_oczekiwania;
    while (plyn) {
        czas_oczekiwania = rand() % (czas * 2) + (czas / 2); // Losowanie czasu między czas/2 a czas*2 sekund
        sleep(czas_oczekiwania);

        if (rand() % 20 == 1) { // Szansa na wykrycie burzy (5%)
            printf("Kapitan portu nakazał zakończenie rejsów!\n");
            if (kill(pid_kapitana, SIGINT) == -1) {
                perror("Błąd wysyłania sygnału zakończenia");
                exit(EXIT_FAILURE);
            }
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
    signal(SIGINT, panic_button);

    // Pobranie PID kapitana
    pobierz_dane();
    utworz_semafor(100,2);

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

    return 0;
}
