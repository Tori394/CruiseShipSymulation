#include "rejs.h"


int mostek;  //kolejka komunikatow
int szlabany; //semafor


// Obsługa sygnału SIGINT
void handler(int sig) {
    exit(EXIT_SUCCESS);
}


// Funkcja opuszczania semafora -1
void opusc_semafor(int nr) {
    struct sembuf op = {nr, -1, 0};
    if (semop(szlabany, &op, 1) == -1) {
        //printf("Nie udało się dostać na rejs, pasażer odszedł \n");
        exit(0);
    }
}

// Funkcja podnoszenia semafora +1
void podnies_semafor(int nr) {
    struct sembuf op = {nr, 1, 0};
    if (semop(szlabany, &op, 1) == -1) {
        //printf("Nie udało się wypłynąć, pasażer odszedł \n");
        exit(0);
    }
}

void usun_podproces_dynamicznie(int sig) {
    // Automatyczne odbieranie statusu zakończenia wszystkich dzieci
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    struct pasazer pass;
    int i=0;
    int ilosc_pasazerow=0;
    int czas_miedzy_pasazerami;

    // Obsługa sygnałów
    signal(SIGINT, handler);
    signal(SIGCHLD, usun_podproces_dynamicznie);

    srand(time(NULL));

    // Połączenie z kolejką komunikatów
    if ((mostek = msgget(123, 0666)) == -1) {
        i++;
        if(i==6) exit(1); //jesli program kapitana nie zostanie włączony w przeciagu 30s, zamknij program
        sleep(5);
    }

    szlabany=utworz_semafor(100,2);

    while(1) {

         if (semctl(szlabany, 0, GETVAL) == -1 && (errno == EINVAL || errno == EIDRM)) {
            printf("Pozostali chętni się rozchodzą...\n");
            break;
        }

        czas_miedzy_pasazerami = rand() % 10 + 5; 
        sleep(czas_miedzy_pasazerami);

        if(fork() == 0) {  // Proces dziecka (pasażer)
            pass.type = NA_STATEK;
            pass.pas_pid = getpid();
            //ilosc_pasazerow++;

            // Próba wejścia na statek
            printf("Do kolejki w rejs ustawił się pasażer %d!\n", pass.pas_pid);
            opusc_semafor(SZLABAN);
            opusc_semafor(MIEJSCE_NA_MOSTKU);
            if (msgsnd(mostek, &pass, ROZMIAR_PASAZERA, 0) == -1) {
                perror("Nie udalo sie wejsc na statek\n");
                exit(EXIT_FAILURE);
            }

            printf("Pasażer %d wszedł na mostek\n", pass.pas_pid);

            // Czekanie na zejście ze statku
            if (msgrcv(mostek, &pass, ROZMIAR_PASAZERA, ZE_STATKU, 0) == -1) {
                perror("Blad przy czekaniu na zejscie ze statku\n");
                exit(EXIT_FAILURE);
            }
            sleep(1);
            printf("Pasażer %d zszedł na ląd\n", pass.pas_pid);
            podnies_semafor(MIEJSCE_NA_MOSTKU);

            exit(0);  // Kończymy proces dziecka
        }
    }
    printf("Port jest pusty, wszyscy się rozeszli\n");

    return 0;
}
