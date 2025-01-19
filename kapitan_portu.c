#include "rejs.h"

pid_t pid_kapitana; 
int czas; // Czas rejsu
int plyn = 1;  // Flaga działania programu
int szlabany;

void koniec_pracy(int sig) {
    plyn = 0;
    semctl(szlabany, 0, IPC_RMID) == -1;
    printf("Koniec pracy\n");
    exit(0);
}

void wyslij_sygnal(pid_t pid, int sygnal) {
    if (kill(pid, sygnal) == -1) {
        perror("Błąd wysyłania sygnału");
        exit(EXIT_FAILURE);
    }
}

void* wyslij_sygnal_start(void* arg) {
    while (plyn) {
        for(int i = 0; i < czas; i++) {
            sleep(1);
            if (sprawdz_wartosc_semafora(1, szlabany) == 0 && sprawdz_wartosc_semafora(0, szlabany) != 0 && rand() % 2 == 1 && plyn) {
                printf("Kapitan Portu zezwolił na wcześnejsze wypłynięcie\n");
                break;
            }
        } 
        wyslij_sygnal(pid_kapitana, SIGUSR1);
        if(sprawdz_wartosc_semafora(1, szlabany) == -1) kill(getpid(),SIGINT);
    }
    return NULL;
}

void* wyslij_sygnal_stop(void* arg) {
    while (plyn) {
        int czas_oczekiwania = rand() % (czas * 2) + (czas / 2);
        sleep(czas_oczekiwania);

        if (rand() % 20 == 1) {
            printf("Nadchodzi sztorm! Kapitan portu nakazał zakończenie rejsów!\n");
            wyslij_sygnal(pid_kapitana, SIGUSR2);
            plyn = 0;
            return NULL;
        }
        if(sprawdz_wartosc_semafora(1, szlabany) == -1) kill(getpid(),SIGINT);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Błąd: Niepoprawna liczba argumentów. Oczekiwane 2 argumenty.\n");
        exit(EXIT_FAILURE);
    }

    pthread_t sygnal_start, sygnal_stop;

    czas = atoi(argv[1]);
    int czas2 = atoi(argv[2]);

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
