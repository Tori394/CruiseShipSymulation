#include "rejs.h"

pid_t pid_kapitana; 
int czas; // Czas miedzy rejsami
int czas2; // czas rejsu
int plyn = 1;  // Flaga działania programu
int szlabany;
int szansa_na_burze; 

// Funkcja obsługująca sygnał SIGINT
void koniec_pracy(int sig) {
    plyn = 0;
    semctl(szlabany, 0, IPC_RMID) == -1;
    exit(0);
}

// Funkcja wysyłająca sygnał startu do kapitana
void* wyslij_sygnal_start(void* arg) {
    sleep(czas2);
    int pom_czas = czas - czas2; // Odliczenie czasu rejsu - żeby rejsy odbywały się co 'czas' a nie żeby statek stał 'czas'
    while (plyn) {
        for(int i = 0; i < pom_czas; i++) {
            sleep(1);
            if ((sprawdz_wartosc_semafora(SZLABAN, szlabany) == 0) && (sprawdz_wartosc_semafora(MIEJSCE_NA_MOSTKU, szlabany) != 0) && plyn) {
                    printf("\033[32mKapitan Portu zezwala na wcześniejszy start\033[0m\n");
                    break;
                }
            }
        wyslij_sygnal(pid_kapitana, SIGUSR1); // Wysłanie sygnału do kapitana
        // Sprawdzenie semafora i zakończenie programu, jeśli semafor niestnieje
        if (sprawdz_wartosc_semafora(1, szlabany) == -1) kill(getpid(), SIGINT);
        sleep(czas2); // Czekanie na zakończenie rejsu
    }
    return NULL;
}

// Funkcja wysyłająca sygnał o burzy (zakończenie rejsów)
void* wyslij_sygnal_stop(void* arg) {
    while (plyn) {
        for(int i=0; i<czas/2; i++) { //2 razy na rejs wypatruj burzy
            sleep(1);
        }

        if (rand() % 100 < szansa_na_burze) { // % szansa na burzę na podstawie wartości użytkownika
            printf("\n\033[31mNadchodzi sztorm! Kapitan portu nakazał zakończenie rejsów!\033[0m\n\n");
            wyslij_sygnal(pid_kapitana, SIGUSR2); // Wysłanie sygnału do kapitana
            plyn = 0;
            return NULL;
        }
        // Sprawdzenie semafora i zakończenie programu, jeśli semafor nie istnieje
        if (sprawdz_wartosc_semafora(1, szlabany) == -1) kill(getpid(), SIGINT);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Błąd: Niepoprawna liczba argumentów. Oczekiwane 3 argumenty.\n");
        exit(EXIT_FAILURE);
    }

    pthread_t sygnal_start, sygnal_stop;

    // Pobranie wartości argumentów
    czas = atoi(argv[1]);
    czas2 = atoi(argv[2]);
    szansa_na_burze = atoi(argv[3]); 


    srand(time(NULL));
    signal(SIGINT, koniec_pracy);

    // Tworzenie FIFO
    if (mkfifo("./fifo", 0666) == -1 && errno != EEXIST) {
        perror("Błąd tworzenia FIFO");
        exit(EXIT_FAILURE);
    }

    // Odbiór PID kapitana
    pid_kapitana = odbierz_pid("./fifo2");
    wyslij_pid(getpid(), "./fifo");

    // Usunięcie FIFO
    if (unlink("./fifo") == -1) {
        perror("Błąd usuwania FIFO");
        exit(EXIT_FAILURE);
    }


    // Sprawdzenie poprawności danych wejściowych
    if (czas <= 10 || czas > 3600) {
        fprintf(stderr, "Podane wartosci są niepoprawne\n");
        kill(pid_kapitana, SIGINT);
        exit(EXIT_FAILURE);
    }

    if (czas <= czas2 ) {
        fprintf(stderr, "Czas pomiedzy rejsami powinien byc większy od czasu trwania rejsu\n");
        kill(pid_kapitana, SIGINT);
        exit(EXIT_FAILURE);
    }

    if (szansa_na_burze < 0 || szansa_na_burze > 100) {
        fprintf(stderr, "Szansa na burzę musi być liczbą całkowitą w przedziale 0-100.\n");
        kill(pid_kapitana, SIGINT);
        exit(EXIT_FAILURE);
    }

    // Utworzenie semafora
    szlabany = utworz_semafor(100, 2);

    // Tworzenie wątku dla sygnału startu
    if (pthread_create(&sygnal_start, NULL, wyslij_sygnal_start, NULL) != 0) {
        perror("Błąd tworzenia wątku sygnału startu");
        exit(EXIT_FAILURE);
    }

    // Tworzenie wątku dla sygnału stopu
    if (pthread_create(&sygnal_stop, NULL, wyslij_sygnal_stop, NULL) != 0) {
        perror("Błąd tworzenia wątku sygnału stop");
        exit(EXIT_FAILURE);
    }

    // Czekanie na zakończenie wątków
    pthread_join(sygnal_start, NULL);
    pthread_join(sygnal_stop, NULL);

    return 0;
}