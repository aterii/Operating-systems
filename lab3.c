#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

int flag1 = 0, flag2 = 0;
int fd[2];

void* proc1() {
    printf("Поток 1 начал работу\n");
    char buf[256];
    while(flag1 == 0) {
        int rv = gethostname(buf, sizeof(buf));
        printf("\n");
        if (rv < 0) {
            perror("Ошибка при получении имени хоста\n");
        }
        else {
            rv = write(fd[1], buf, strlen(buf) + 1);
            if (rv < 0) {
                perror("Ошибка при записи\n");
            }
        }
        sleep(1);
    }
    printf("Поток 1 закончил работу\n");
}

void* proc2() {
    printf("Поток 2 начал работу\n");
    char buf[256];
    int rv;
    while(flag2 == 0) {
        rv = read(fd[0], buf, sizeof(buf));
        if (rv == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("Нет данных для чтения.\n");
            } else {
                perror("Ошибка при чтении\n");
            }
            sleep(1);

        }
        else if (rv == 0) {
            printf("EOF\n");
        }
        else {
            printf("%s", buf);
            fflush(stdout);
        }
    }
    printf("Поток 2 закончил работу\n");
}

void sig_handler(int signo) {
    close(fd[0]);
    close(fd[1]);
    exit(0);
}

int main(int argc, char* argv[]) {
    signal(SIGINT, sig_handler);
    printf("Программа начала работу\n");
    pthread_t id1, id2;
    int pipe_flags = 0;
    // if (argc > 1) {
    //     if (!strcmp(argv[1], "pipe")) {
    //     }
    //     else if (!strcmp(argv[1], "pipe2")) {
    //         pipe_flags = O_NONBLOCK;
    //     }
    //     else if (!strcmp(argv[1], "fcntl")) {
    //         pipe_flags = O_NONBLOCK;
    //     }
    // }

    // if (pipe2(fd, pipe_flags) == -1 && pipe_flags != 0) {
    //         perror("pipe2");
    //         exit(EXIT_FAILURE);
    //         //pipe_flags = O_NONBLOCK;
    // } else if (pipe(fd) == -1 && pipe_flags == 0) {
    //     perror("pipe");
    //     exit(EXIT_FAILURE);
    // }

    // if (pipe_flags == O_NONBLOCK && !strcmp(argv[1], "fcntl")) {
    //   fcntl(fd[0], F_SETFL, O_NONBLOCK);
    //   fcntl(fd[1], F_SETFL, O_NONBLOCK);
    // }

    if (argv[1]) {
        if (!strcmp(argv[1], "pipe")) {
            pipe(fd);
        }
        else if (!strcmp(argv[1], "pipe2")) {
            pipe2(fd, O_NONBLOCK);
        }
        else if (!strcmp(argv[1], "fcntl")) {
            pipe(fd);
            fcntl(fd[0], F_SETFL, O_NONBLOCK);
            fcntl(fd[1], F_SETFL, O_NONBLOCK);
        }
    }
    else {
        pipe(fd);
    }

    pthread_create(&id1, NULL, proc1, NULL);
    pthread_create(&id2, NULL, proc2, NULL);

    printf("Программа ждет нажатия клавиши\n");

    getchar();

    printf("Клавиша нажата\n");

    flag1 = 1;
    flag2 = 1;

    pthread_join(id1, NULL);
    pthread_join(id2, NULL);

    close(fd[0]);
    close(fd[1]);
    
    printf("Программа завершила работу\n");

    return 0;

}
