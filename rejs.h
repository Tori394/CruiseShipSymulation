#ifndef REJS_H
#define REJS_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/sem.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#define NA_STATEK 3 // Kierunek wiadomości w kolejce, wchodzenie na statek
#define ZE_STATKU 1 // Kierunek wiadomości w kolejce, wychodzenie ze statku
#define ROZMIAR_PASAZERA (sizeof(pid_t)) // Rozmiar struktury pasażera
#define MIEJSCE_NA_MOSTKU 0 // Numer semafora dla miejsca na mostku
#define SZLABAN 1 // Numer semafora dla szlabanu

// Struktura pasażera
struct pasazer {
    long type;  // Typ wiadomości
    pid_t pas_pid;  // PID pasażera
};

//============ FUNKCJE DLA KAPITANA =====================
int utworz_semafor(key_t klucz, int nr);
void ustaw_wartosc_semafora(int wartosc, int nr, int sem);
int sprawdz_wartosc_semafora(int nr, int s);
void zakoncz(int kolejka, int semafor, pid_t pid);
void wyslij_pid(pid_t pid, const char *fifo_path);
pid_t odbierz_pid(const char *fifo_path);

//========== FUNKCJE DLA KAPITANA PORTU =================
//void wyslij_pid(pid_t pid, const char *fifo_path);
//pid_t odbierz_pid(const char *fifo_path);
//int sprawdz_wartosc_semafora(int nr, int s);
//int utworz_semafor(key_t klucz, int nr);
void wyslij_sygnal(pid_t pid, int sygnal);

//============ FUNKCJE DLA PASAŻERA =====================
//int utworz_semafor(key_t klucz, int nr);
int polacz_kolejke(int s);

//=================== INNE ==============================
void otworz_fifo(const char *fifo_path, int *fd, int mode);

#endif // REJS_H
