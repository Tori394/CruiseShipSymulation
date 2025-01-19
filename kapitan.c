#include "rejs.h"

int plyn = 1;
int startuj = 0;
int mostek;  // kolejka komunikatow
int szlabany; // semafor
pid_t pid_kapitana;

// Obsługa sygnału SIGINT PANIC BUTTON
void odbierz_sygnal_stop(int sig) {
    plyn = 0;
}

void odbierz_sygnal_start(int sig) {
    if (sig == SIGUSR1) {
        printf("Kapitan otrzymał sygnał do startu!\n");
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



int main() {
    int liczba_pasazerow = 0;
    int pojemnosc_mostka, pojemnosc_statku, ilosc_rejsow_dzis, czas_rejsu;

    scanf("%d", &pojemnosc_mostka);
    scanf("%d", &pojemnosc_statku); //dodac obdluge bledow!
    scanf("%d", &ilosc_rejsow_dzis);
    scanf("%d", &czas_rejsu);

    pid_t pasazerowie[pojemnosc_statku]; // Tablica na PID-y pasażerów

    struct pasazer pass;
    struct msqid_ds buf;
    int val = pojemnosc_statku;
    int val2 = 0;


    if (mkfifo("./fifo2", 0666) == -1 && errno != EEXIST) {
        perror("Błąd tworzenia FIFO2");
        exit(EXIT_FAILURE);
    }
    // Tworzenie kolejki komunikatów
    if ((mostek = msgget(123, IPC_CREAT | 0666)) == -1) {
        perror("Blad tworzenia kolejki\n");
        exit(EXIT_FAILURE);
    }

    wyslij_pid(getpid(), "./fifo2"); 
    pid_kapitana = odbierz_pid("./fifo");  // Odbieranie PID od kapitana portu

    if (unlink("./fifo2") == -1) {
        perror("Błąd usuwania FIFO");
        exit(EXIT_FAILURE);
    }

    // Wysłanie PID do portu
    
    signal(SIGINT, odbierz_sygnal_stop);
    signal(SIGUSR1, odbierz_sygnal_start);

    // Tworzenie semafora
    szlabany = utworz_semafor(100, 2);
    ustaw_wartosc_semafora(pojemnosc_mostka, MIEJSCE_NA_MOSTKU, szlabany); // semafor 0 - kontrola wielkości mostka

    while (ilosc_rejsow_dzis && plyn) {
        ustaw_wartosc_semafora(pojemnosc_statku, SZLABAN, szlabany); // semafor 1 - kontrola liczby ludzi wchodzących na statek
        val = pojemnosc_statku;
        val2 = pojemnosc_mostka;
        printf("Szlaban się otwiera...\n");

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
            printf("Kapitan wpuscil na statek pasażera %d\n", pass.pas_pid);
            val = sprawdz_wartosc_semafora(SZLABAN, szlabany);
            val2 = sprawdz_wartosc_semafora(MIEJSCE_NA_MOSTKU, szlabany);
        }

        printf("Na statek weszło %d pasażerów\n", liczba_pasazerow);

        while (!startuj && plyn) {
            sleep(1);
        }

        // Rejs rozpoczyna się, o ile nie przerwano rejsów
        if (plyn) {
            if (startuj) {
                if (liczba_pasazerow == 0) {
                    printf("Statek nie odplynie bez pasażerów\n");
                    continue;
                } else {
                    printf("Rejs z %d pasażerami się rozpoczął\n", liczba_pasazerow);
                    sleep(czas_rejsu); // Symulacja rejsu
                    printf("Rejs zakończony\nNa dzisiaj zaplanowano jeszcze %d rejsów\n", --ilosc_rejsow_dzis);
                }
            }
        } else {
            printf("Rejs się jednak nie odbędzie\n");
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
        printf("Mostek jest pusty, inni pasażerownie mogą znowu wejść\n");
        startuj = 0;
    }

    switch (ilosc_rejsow_dzis) {
        case 0:
            printf("Wykonano wszystkie zaplanowane rejsy na dziś\n");
            break;
        default:
            printf("Rejsy zostały wstrzymane\n");
            break;
    }
    zakoncz(mostek, szlabany);
    kill(pid_kapitana, SIGINT);
    return 0;
}
