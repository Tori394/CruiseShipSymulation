#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
pid_t pid_kapitana;

void pobierz_pid() {
    int fd;

    while ((fd = open("./fifo", O_RDONLY)) == -1) {
        sleep(1);
    }
    printf("pipe sie otworzylo");
    if (read(fd, &pid_kapitana, sizeof(pid_t)) == -1) {
        perror("read");
        exit(1);
    }
    else
    printf("pipe pobralo pid");

    unlink("./fifo");
    close(fd);
}

int main() {
    
    pobierz_pid();
    printf("Otrzymano PID Kapitana %d\n", pid_kapitana);

    // Pętla wysyłająca sygnały co 30 sekund
    while (1) {
        sleep(30);
        if (kill(pid_kapitana, SIGUSR1) == -1) {
            perror("Błąd wysyłania sygnału");
            exit(EXIT_FAILURE);
        }
        printf("Wysłano sygnał SIGUSR1 do procesu o PID %d\n", pid_kapitana);
    }

    return 0;
}