#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define NA_STATEK 3
#define ZE_STATKU 1
#define ROZMIAR_PASAZERA (sizeof(pid_t))
#define MIEJSCE_NA_MOSTKU 0
#define SZLABAN 1

int plyn=1;
int startuj=0;
int mostek;  //kolejka komunikatow
int szlabany; //semafor
pthread_t czas;

// Struktura pasażera
struct pasazer {
    long type;
    pid_t pas_pid;
};

// Obsługa sygnału SIGINT PANIC BUTTON
void panic_button(int sig) {
    printf("Sygnal SIGINT! Zakonczenie dzialania\n");
    if (msgctl(mostek, IPC_RMID, NULL) == -1) {
        perror("Blad usuniecia kolejki\n");
    }
    if ((semctl(szlabany,0,IPC_RMID))==-1)
    {
        printf("Blad usuwania semafora\n");
        exit(EXIT_FAILURE);
    }
    plyn=0;
    exit(1);
}

// Czyszczenie na koncu programu
void zakoncz(){
    printf("Zakonczenie symulacji\n");
    if (msgctl(mostek, IPC_RMID, NULL) == -1) {
        perror("Blad usuniecia kolejki\n");
    }
    if ((semctl(szlabany,0,IPC_RMID))==-1)
    {
        printf("Blad usuwania semafora\n");
        exit(EXIT_FAILURE);
    }
    exit(1);
}

// Funkcja tworząca semafor
void utworz_semafor(key_t klucz, int nr) {
    szlabany = semget(klucz, nr, 0600 | IPC_CREAT);
    if (szlabany == -1) {
        perror("Nie udalo sie utworzyc semafora");
        exit(EXIT_FAILURE);
    }
}

// Funkcja ustawiająca wartość semafora
void ustaw_wartosc_semafora(int wartosc, int nr) {
    if (semctl(szlabany, nr, SETVAL, wartosc) == -1) {
        perror("Blad ustawienia semafora");
        exit(EXIT_FAILURE);
    }
}

// Funkcja opuszczania semafora -1
void opusc_semafor(int nr) {
    struct sembuf op = {nr, -1, 0};
    if (semop(szlabany, &op, 1) == -1) {
        perror("Blad opuszczania semafora");
        exit(EXIT_FAILURE);
    }
}

// Funkcja podnoszenia semafora +1
void podnies_semafor(int nr) {
    struct sembuf op = {nr, 1, 0};
    if (semop(szlabany, &op, 1) == -1) {
        perror("Blad podnoszenia semafora");
        exit(EXIT_FAILURE);
    }
}

void odbierz_sygnal_start(int sig) {
    if (sig == SIGUSR1) {
        printf("Kapitan otrzymał sygnał do startu, zamykamy wejście!\n");
        ustaw_wartosc_semafora(0, SZLABAN);
        startuj = 1;
    }
}


void przekaz_pid(pid_t pid) {
    int fd;

    if (mkfifo("./fifo", 0666) == -1) {
        if (errno != EEXIST) {
        perror("mkfifo");
        exit(1);
        }
    }
    sleep(5);
    fd = open("./fifo", O_WRONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }
    if (write(fd, &pid, sizeof(pid_t)) == -1) {
        perror("write");
        exit(1);
    }

    close(fd);
}

int main() {
    int liczba_pasazerow=0;
    int pojemnosc_mostka=4;
    int pojemnosc_statku=9;
    int ilosc_rejsow_dzis=4;
    int czas_rejsu=10;
    int czas_miedzy_rejsami=30;
    pid_t pasazerowie[pojemnosc_statku]; //ZAŁOŻENIE DO TESTOW ZE POJEMNOSC STATKU TO 7
    struct pasazer pass;
    struct msqid_ds buf;
    pid_t moj_pid;

    moj_pid=getpid();
    printf("%d, %d", moj_pid, getpid());
 
    przekaz_pid(moj_pid);

        // Ustawienie obsługi sygnału PANIC BUTTON
        signal(SIGINT, panic_button);
        signal(SIGUSR1, odbierz_sygnal_start);

        // Tworzenie kolejki komunikatów
        if ((mostek = msgget(123, IPC_CREAT | 0666)) == -1) {
            perror("Blad tworzenia kolejki\n");
            exit(EXIT_FAILURE);
        }
        // Tworzenie semafora
        utworz_semafor(100,2);
        ustaw_wartosc_semafora(pojemnosc_mostka, MIEJSCE_NA_MOSTKU); //semafor 0 - kontrola wielkosci mostka

        while(ilosc_rejsow_dzis && plyn)
        {
        
            ustaw_wartosc_semafora(pojemnosc_statku, SZLABAN); //semafor 1 - kontrola ilosc ludzi wchodzacych na statek
            printf("Szlaban sie otwiera...\n");
            sleep(2);

            while (liczba_pasazerow < pojemnosc_statku) {
                if (msgrcv(mostek, &pass, ROZMIAR_PASAZERA, NA_STATEK, 0) == -1) {
                    if (errno == EINTR) continue; // Ignorowanie przerwań
                    perror("Blad pobrania pasazera na statek");
                    break;
                }
                pasazerowie[liczba_pasazerow++] = pass.pas_pid; // Zapisz PID pasażera
                podnies_semafor(MIEJSCE_NA_MOSTKU);
                printf("Kapitan wpuscil na statek pasażera %d\n", pass.pas_pid);
            }

                printf("Na statek weszło %d pasażerów, za niedługo wypłynie\n", liczba_pasazerow);
            
                while(!startuj && plyn) {sleep(1);}
                        
                //jesli sygnal nie byl sygnalem do przerwania rejsow
                if(plyn){
                    // Symulacja rejsu
                    if (startuj) {
                        printf("\n\nWydano sygnal do startu\n\n");
                        sleep(2);
                        if(liczba_pasazerow==0){
                            printf("Statek nie odplynie bez pasażerów\n");
                            continue;
                        }
                        else{
                            printf("\nRejs z %d pasażerami się rozpoczął\n", liczba_pasazerow);
                            sleep(czas_rejsu); // Symulacja rejsu
                            printf("Rejs zakończony\n\n");
                            ilosc_rejsow_dzis--;
                        }
                    }
                }
                else{
                    printf("Rejs się jednak nie odbędzie\n");
                }

                // Wypuszczenie pasażerow ze statku
                for (int i = 0; i < liczba_pasazerow; i++) {
                pass.type = ZE_STATKU;
                    pass.pas_pid = pasazerowie[i];
                    if (msgsnd(mostek, &pass, ROZMIAR_PASAZERA, 0) == -1) {
                        perror("Blad wyslania pasażera ze statku");
                    } else {
                        opusc_semafor(MIEJSCE_NA_MOSTKU);
                        //printf("Kapitan wypuścił ze statku pasażera %d\n", pass.pas_pid);
                    }
                }  

                liczba_pasazerow=0;

                while (buf.msg_qnum != 0) {
                if (msgctl(mostek, IPC_STAT, &buf) == -1) {
                    perror("Blad pobrania informacji o kolejce\n");
                    exit(EXIT_FAILURE);
                }
                sleep(1); // Odczekanie przed ponownym sprawdzeniem
                }   
                startuj=0;         
        }

        switch(ilosc_rejsow_dzis){
            case 0: 
                printf("Wykonano wszystkie zaplanowane rejsy na dziś\n");
                break;
            default:
                printf("Rejsy zostały wstrzymane\n");
                break;
        }
    wait(NULL);
    zakoncz();
    return 0;
}
