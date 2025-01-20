#include "rejs.h"

pid_t pid_kapitana; 
int czas; // Czas miedzy rejsami
int czas2; // czas rejsu
int plyn = 1;  // Flaga działania programu
int szlabany;
int szansa_na_burze; // Nowa zmienna do przechowywania szansy na burzę

void koniec_pracy(int sig) {
    plyn = 0;
    semctl(szlabany, 0, IPC_RMID) == -1;
    exit(0);
}

void* wyslij_sygnal_start(void* arg) {
    int wczesniej_check=1;
    sleep(czas2);
    int pom_czas = czas - czas2;
    while (plyn) {
        for(int i = 0; i < pom_czas; i++) {
            sleep(1);
            if ((sprawdz_wartosc_semafora(SZLABAN, szlabany) == 0) && (sprawdz_wartosc_semafora(MIEJSCE_NA_MOSTKU, szlabany) != 0) && plyn && wczesniej_check) {
                if ((rand() % 2 == 1)) { //szansa na wczesniejszy start 50%
                    printf("\033[32mKapitan Portu zezwala na wcześniejszy start\033[0m\n");
                    break;
                }
                wczesniej_check=0; //zapewnienie ze test wykona się tylko raz, zeby nie pytac kapitana co sekundę (nie wykonywac dodatkowych operacji co chwilę)
            }
        } 
        wyslij_sygnal(pid_kapitana, SIGUSR1);
        if (sprawdz_wartosc_semafora(1, szlabany) == -1) kill(getpid(), SIGINT);
        sleep(czas2); //odczekaj az rejs sie skonczy - inaczej sygnały beda wysyłane do pustego portu
        wczesniej_check=1;
    }
    return NULL;
}

void* wyslij_sygnal_stop(void* arg) {
    while (plyn) {
        for(int i=0; i<czas; i++) { //raz na rejs wypatruj burzy
            sleep(1);
        }

        if (rand() % 100 < szansa_na_burze) { // szansa na burzę na podstawie wartości użytkownika
            printf("\n\033[31mNadchodzi sztorm! Kapitan portu nakazał zakończenie rejsów!\033[0m\n\n");
            wyslij_sygnal(pid_kapitana, SIGUSR2);
            plyn = 0;
            return NULL;
        }
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

    czas = atoi(argv[1]);
    czas2 = atoi(argv[2]);
    szansa_na_burze = atoi(argv[3]); // Pobranie szansy na burzę

    srand(time(NULL));
    signal(SIGINT, koniec_pracy);

    if (mkfifo("./fifo", 0666) == -1 && errno != EEXIST) {
        perror("Błąd tworzenia FIFO");
        exit(EXIT_FAILURE);
    }

    pid_kapitana = odbierz_pid("./fifo2");
    wyslij_pid(getpid(), "./fifo");

    if (unlink("./fifo") == -1) {
        perror("Błąd usuwania FIFO");
        exit(EXIT_FAILURE);
    }

    if (czas <= 0) {
        fprintf(stderr, "Podane wartosci muszą być większe niż 0\n");
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

    szlabany = utworz_semafor(100, 2);

    if (pthread_create(&sygnal_start, NULL, wyslij_sygnal_start, NULL) != 0) {
        perror("Błąd tworzenia wątku sygnału startu");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&sygnal_stop, NULL, wyslij_sygnal_stop, NULL) != 0) {
        perror("Błąd tworzenia wątku sygnału stop");
        exit(EXIT_FAILURE);
    }

    pthread_join(sygnal_start, NULL);
    pthread_join(sygnal_stop, NULL);

    return 0;
}
