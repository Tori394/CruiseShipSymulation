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

#define NA_STATEK 3
#define ZE_STATKU 1
#define ROZMIAR_PASAZERA (sizeof(pid_t))
#define MIEJSCE_NA_MOSTKU 0
#define SZLABAN 1


// Struktura pasażera
struct pasazer {
    long type;
    pid_t pas_pid;
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

//============ FUNKCJE DLA PASAŻERA =====================
//int utworz_semafor(key_t klucz, int nr);

//=================== INNE ==============================
void otworz_fifo(const char *fifo_path, int *fd, int mode);

#endif // REJS_H
