#include "rejs.h"

int mostek;  // Kolejka komunikatów do komunikacji z pasażerami
int szlabany; // Semafor kontrolujący dostęp do zasobów
int ilosc_pasazerow = 0; 
int limit=1000;
pid_t pid;
pid_t pid_pop;

// Obsługa sygnału SIGINT
void koniec_pracy(int sig) {
    while (waitpid(-1, NULL, 0) > 0);
    exit(EXIT_SUCCESS);
}

// Funkcja opuszczania semafora (zmniejsza wartość semafora o 1)
void opusc_semafor(int nr) {
   // printf("SZLABAN PRZED   %d\n", sprawdz_wartosc_semafora(SZLABAN, szlabany));
    struct sembuf op;
    op.sem_num = nr;  // Wskaż semafor, z którym operujemy (tylko jeden w tym przypadku)
    op.sem_op = -1;  // Operacja czekania (zmniejszamy semafor)
    op.sem_flg = 0;  // Flagi (0 oznacza blokowanie procesu)
    if (semop(szlabany, &op, 1) == -1) {
        // Pasażer odchodzi
        exit(0);
    }
   // printf("SZLABAN PO   %d\n", sprawdz_wartosc_semafora(SZLABAN, szlabany));
}

// Funkcja podnoszenia semafora (zwiększa wartość semafora o 1)
void podnies_semafor(int nr) {
    struct sembuf op = {nr, 1, 0}; 
    if (semop(szlabany, &op, 1) == -1) {
        // Pasażer odchodzi
        exit(0);
    }
}

// Obsługa sygnału SIGCHLD (automatyczne usuwanie procesów potomnych)
void usun_podproces_dynamicznie(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
    ilosc_pasazerow--;
}

int main() {
    struct pasazer pass; // Struktura przechowująca informacje o pasażerze
    int i = 0;
    int czas_miedzy_pasazerami;


    // Rejestracja obsługi sygnałów
    signal(SIGINT, koniec_pracy); 
    signal(SIGCHLD, usun_podproces_dynamicznie); 

    srand(time(NULL)); 

    szlabany = utworz_semafor(100, 2); // Tworzenie semafora
    /*while(msgget(123, 0666)==-1) {
        if (sprawdz_wartosc_semafora(1, szlabany) == -1 && (errno == EINVAL || errno == EIDRM)) {
            koniec_pracy(SIGINT);
        }
    }*/
   
   if ((mostek = msgget(123, IPC_CREAT | 0666)) == -1) {
        perror("Blad tworzenia kolejki\n");
        exit(EXIT_FAILURE);
    } // Łączenie się do kolejki komunikatów

    while (1) {

        // Sprawdzenie, czy semafor istnieje i jest dostępny
        if (sprawdz_wartosc_semafora(1, szlabany) == -1 && (errno == EINVAL || errno == EIDRM)) {
            break; // Zakończenie pętli w przypadku usunięcia semafora
        }

        // Dodawanie nowych pasażerów, jeśli ich liczba nie przekroczyła limitu
        if (ilosc_pasazerow < limit) {
        //   czas_miedzy_pasazerami = rand() % 5 + 5; // Losowy czas oczekiwania
         //   sleep(1);
            pid=fork();
            switch (pid)
            {
            case 0:
                pass.type = NA_STATEK; // Typ komunikatu: pasażer chce wejść na statek
                pass.pas_pid = getpid(); // PID procesu dziecka
        //        printf("\033[33mDo kolejki w rejs ustawił się pasażer \033[0m%d\033[33m!\033[0m\n", pass.pas_pid);

                // Próba wejścia na statek (opuszczenie semaforów)
                opusc_semafor(SZLABAN); // Sprawdzenie, czy można wejść na statek
                podnies_semafor(MIEJSCE_NA_MOSTKU);
                printf("zwiekszony  %d\n", sprawdz_wartosc_semafora(MIEJSCE_NA_MOSTKU, szlabany));
                if (msgsnd(mostek, &pass, ROZMIAR_PASAZERA, 0) == -1) {
                    //perror("Nie udalo sie wejsc na statek\n");
                    exit(0);
                }

                printf("\033[33mPasażer \033[0m%d\033[33m wszedł na mostek\033[0m\n", pass.pas_pid);
                opusc_semafor(MIEJSCE_NA_MOSTKU);

                // Oczekiwanie na zejście ze statku
                if (msgrcv(mostek, &pass, ROZMIAR_PASAZERA, ZE_STATKU, 0) == -1) {
                    perror("Blad przy czekaniu na zejscie ze statku\n");
                    exit(EXIT_FAILURE);
                }

                // Zwolnienie miejsca na mostku i zakończenie procesu pasażera
               // podnies_semafor(MIEJSCE_NA_MOSTKU);
                printf("\033[90mPasażer %d odchodzi\033[0m\n", pass.pas_pid);

                exit(0); // Zakończenie procesu dziecka
                break;
            case -1:
                limit=ilosc_pasazerow-ilosc_pasazerow/10;
                kill(pid_pop, SIGTERM);
                printf("\033[31mMolo jest przepełnione! Nie pojawi się więcej pasażerów dopóki się nie opróżni...\033[0m\n");
                while (waitpid(-1, NULL, 0) > 0);
                ilosc_pasazerow=0;
                break;
            default:
                ilosc_pasazerow++;
                pid_pop=pid;
                break;
            }
        }
    }

    // Zamykanie portu, oczekiwanie na zakończenie procesów potomnych
    printf("\033[31mChętni się rozchodzą...\033[0m\n");
    while (waitpid(-1, NULL, 0) > 0);
    printf("Port jest pusty, wszyscy się rozeszli\n");
    return 0; // Zakończenie programu
}
