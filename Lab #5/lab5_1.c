#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <termios.h>

#define SEM_NAME "/sem_sync"
#define FILE_NAME "output.txt"

int key_pressed_custom() {
    struct termios oldt, newt;
    int oldf;
    char ch;
    int pressed = 0;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    oldf = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    if (read(STDIN_FILENO, &ch, 1) > 0) {
        pressed = 1;
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    return pressed;
}

int main() {
    printf("Программа 5_1 начала работу\n");

    sem_t *sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);
    if (sem == SEM_FAILED) {
        if (errno == EEXIST) {
            sem = sem_open(SEM_NAME, 0);
            if (sem == SEM_FAILED) {
                perror("Ошибка открытия существующего семафора");
                exit(EXIT_FAILURE);
            }
        } else {
            perror("Ошибка создания семафора");
            exit(EXIT_FAILURE);
        }
    }

    FILE *file = fopen(FILE_NAME, "a");
    if (!file) {
        perror("Ошибка открытия файла");
        sem_close(sem);
        sem_unlink(SEM_NAME);
        exit(EXIT_FAILURE);
    }

    printf("Программа 5_1 ожидает нажатия клавиши\n");

    while (!key_pressed_custom()) {
        if (sem_wait(sem) == -1) {
            perror("Ошибка sem_wait");
            break;
        }

        for (int i = 0; i < 10; i++) {
            fputc('1', file);  
            fflush(file);
            putchar('1'); 
            fflush(stdout);
            sleep(1);
        }
        putchar('\n');

        if (sem_post(sem) == -1) {
            perror("Ошибка sem_post");
            break;
        }

        sleep(1);
    }

    fclose(file);
    sem_close(sem);
    sem_unlink(SEM_NAME);

    printf("Программа 5_1 завершила работу\n");
    return 0;
}
