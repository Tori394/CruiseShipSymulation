#include "rejs.h"

int plyn = 1;
int startuj = 0;
int mostek;  // kolejka komunikatow
int szlabany; // semafor
pid_t pid_kapitana;

// Obsługa sygnału SIGINT PANIC BUTTON
void odbierz_sygnal_stop(int sig) {
    ustaw_wartosc_semafora(0, SZLABAN, szlabany);
    plyn=0;
}

void koniec_pracy(int sig) {
    zakoncz(mostek, szlabany, pid_kapitana);
    exit(EXIT_FAILURE);
}

void odbierz_sygnal_start(int sig) {
    if (sig == SIGUSR1) {
        printf("\033[32mKapitan otrzymał sygnał do startu!\033[0m\n");
        ustaw_wartosc_semafora(0, SZLABAN, szlabany);
        startuj = 1;
    }
}

// Funkcja opuszczania semafora -1
void opusc_semafor(int nr) {
    struct sembuf op = {nr, -1, 0};
    while (semop(szlabany, &op, 1) == -1) {
        if (errno == EINTR) {
            // Operacja została przerwana przez sygnał, spróbuj ponownie i anuluj sygnał
            continue;
        } else {
            perror("Blad opuszczania semafora");
            exit(EXIT_FAILURE);
        }
    }
}

// Funkcja podnoszenia semafora +1
void podnies_semafor(int nr) {
    struct sembuf op = {nr, 1, 0};
    while (semop(szlabany, &op, 1) == -1) {
        if (errno == EINTR) {
            // Operacja została przerwana przez sygnał, spróbuj ponownie i anuluj sygnał
            continue;
        } else {
            perror("Blad podnoszenia semafora");
            exit(EXIT_FAILURE);
        }
    }
}



int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Błąd: Niepoprawna liczba argumentów. Oczekiwane 4 argumenty.\n");
        exit(EXIT_FAILURE);
    }

    int pojemnosc_mostka = atoi(argv[1]);
    int pojemnosc_statku = atoi(argv[2]);
    int ilosc_rejsow_dzis = atoi(argv[3]);
    int czas_rejsu = atoi(argv[4]);

    pid_t pasazerowie[pojemnosc_statku]; // Tablica na PID-y pasażerów

    signal(SIGINT, koniec_pracy);
    signal(SIGUSR1, odbierz_sygnal_start);
    signal(SIGUSR2, odbierz_sygnal_stop);
    
    if (mkfifo("./fifo2", 0666) == -1 && errno != EEXIST) {
        perror("Błąd tworzenia FIFO2");
        exit(EXIT_FAILURE);
    }

    wyslij_pid(getpid(), "./fifo2"); 
    pid_kapitana = odbierz_pid("./fifo");  // Odbieranie PID od kapitana portu

    if (unlink("./fifo2") == -1) {
        perror("Błąd usuwania FIFO");
        exit(EXIT_FAILURE);
    }

    // Obsługa złych danych wejściowych

    if (ilosc_rejsow_dzis<=0 || czas_rejsu <=0 || pojemnosc_mostka<=0 || pojemnosc_statku <0) {
        fprintf(stderr, "Podane wartosci muszą być większe niż 0\n");
        zakoncz(mostek, szlabany, pid_kapitana);
        exit(EXIT_FAILURE);
    }
    
    if (pojemnosc_mostka > pojemnosc_statku) {
        fprintf(stderr, "Pojemność mostka powinna być mniejsza niż pojemność statku\n");
        zakoncz(mostek, szlabany, pid_kapitana);
        exit(EXIT_FAILURE);
    }

    if (pojemnosc_statku > 300) {
        fprintf(stderr, "Statek nie może być tak duży!\n");
        zakoncz(mostek, szlabany, pid_kapitana);
        exit(EXIT_FAILURE);
    }

    // Tworzenie kolejki komunikatów
    if ((mostek = msgget(123, IPC_CREAT | 0666)) == -1) {
        perror("Blad tworzenia kolejki\n");
        exit(EXIT_FAILURE);
    }

    int liczba_pasazerow = 0;
    struct pasazer pass;
    struct msqid_ds buf;
    int val = pojemnosc_statku;
    int val2 = 0;

    // Tworzenie semafora
    szlabany = utworz_semafor(100, 2);
    ustaw_wartosc_semafora(pojemnosc_mostka, MIEJSCE_NA_MOSTKU, szlabany); // semafor 0 - kontrola wielkości mostka


    while (ilosc_rejsow_dzis && plyn) {
        ustaw_wartosc_semafora(pojemnosc_statku, SZLABAN, szlabany); // semafor 1 - kontrola liczby ludzi wchodzących na statek
        val = pojemnosc_statku;
        val2 = pojemnosc_mostka;

        while (val2 < pojemnosc_mostka || val > 0) {
            if (msgrcv(mostek, &pass, ROZMIAR_PASAZERA, NA_STATEK, 0) == -1) {
                if (errno == EINTR) { // Ignorowanie przerwań
                    val = sprawdz_wartosc_semafora(SZLABAN, szlabany);
                    val2 = sprawdz_wartosc_semafora(MIEJSCE_NA_MOSTKU, szlabany);
                    continue;
                }
            }
            pasazerowie[liczba_pasazerow++] = pass.pas_pid; // Zapisz PID pasażera
            podnies_semafor(MIEJSCE_NA_MOSTKU);
            printf("\033[36mKapitan wpuscił na statek pasażera \033[0m%d\n", pass.pas_pid);
            val = sprawdz_wartosc_semafora(SZLABAN, szlabany);
            val2 = sprawdz_wartosc_semafora(MIEJSCE_NA_MOSTKU, szlabany);
        }
        if (buf.msg_qnum == 0) printf("\033[36mMostek jest pusty\033[0m\n");
        printf("\033[36mNa statek weszło \033[0m%d\033[36m pasażerów\033[0m\n", liczba_pasazerow);

        while (!startuj && plyn) {
            sleep(1);
        }

        // Rejs rozpoczyna się, o ile nie przerwano rejsów
        if (plyn) {
            if (startuj) {
                if (liczba_pasazerow == 0) {
                    printf("\033[31mStatek nie odpłynie bez pasażerów\033[0m\n");
                    continue;
                } else {
                    printf("\033[36mRejs z \033[0m%d\033[36m pasażerami się rozpoczął\033[0m\n", liczba_pasazerow);
                    sleep(czas_rejsu); // Symulacja rejsu
                    printf("\033[36mRejs zakończony\033[0m\n\033[36mNa dzisiaj zaplanowano jeszcze \033[31m%d\033[36m rejsów\033[0m\n", --ilosc_rejsow_dzis);
                }
            }
        } else {
            printf("\033[36mRejs się jednak nie odbędzie\033[0m\n");
        }

        // Wypuszczenie pasażerów ze statku
        for (int i = 0; i < liczba_pasazerow; i++) {
            pass.type = ZE_STATKU;
            pass.pas_pid = pasazerowie[i];
            while (msgsnd(mostek, &pass, ROZMIAR_PASAZERA, 0) == -1) {
                if (errno == EINTR) {
                    continue;
                }
            }
            printf("\033[33mPasażer \033[0m%d\033[33m zszedł na ląd\033[0m\n", pass.pas_pid);
            opusc_semafor(MIEJSCE_NA_MOSTKU);
        }

        liczba_pasazerow = 0;

        while (buf.msg_qnum != 0) {
            if (msgctl(mostek, IPC_STAT, &buf) == -1) {
                perror("Blad pobrania informacji o kolejce\n");
                exit(EXIT_FAILURE);
            }
            sleep(1);
        }
        printf("\033[36mMostek jest pusty, inni pasażerowie mogą znowu wejść\033[0m\n");
        startuj = 0;
    }

    switch (ilosc_rejsow_dzis) {
        case 0:
            printf("\033[36mWykonano wszystkie zaplanowane rejsy na dziś\033[0m\n");
            break;
        default:
            printf("\033[31mRejsy zostały wstrzymane\033[0m\n");
            break;
    }
    zakoncz(mostek, szlabany, pid_kapitana);
    return 0;
}
