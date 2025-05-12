#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#define MSG_KEY 1234
#define MSG_SIZE 2048

struct my_msgbuf {
    long mtype;
    char mtext[MSG_SIZE];
};

int msqid;
int fl1 = 0;
pthread_t thid;

void* thr_fn2(void* arg) {
    struct my_msgbuf msg;

    while (!fl1) {
        ssize_t len = msgrcv(msqid, &msg, MSG_SIZE, 0, IPC_NOWAIT);
        if (len == -1) {
            if (errno == ENOMSG) {
                printf("Очередь пуста, ожидаем...\n");
            } else {
                perror("Ошибка msgrcv");
            }
        } else {
            printf("Принято сообщение: %s\n", msg.mtext);
        }

        sleep(1);
    }

    pthread_exit(NULL);
}

void sig_hand(int sig) {
    fl1 = 1;
    pthread_cancel(thid);
    pthread_join(thid, NULL);
    msgctl(msqid, IPC_RMID, NULL);
    printf("\nПрограмма завершена (получатель)\n");
    exit(0);
}

int main() {
    printf("Программа 2 (получатель) начала работу.\n");
    signal(SIGINT, sig_hand);

    msqid = msgget(MSG_KEY, 0666);
    if (msqid == -1) {
        perror("Ошибка msgget");
        return 1;
    }

    if (pthread_create(&thid, NULL, thr_fn2, NULL) != 0) {
        perror("Ошибка создания потока");
        return 1;
    }

    printf("Ожидание нажатия клавиши.\n");
    getchar();
    printf("Клавиша нажата\n");

    fl1 = 1;
    pthread_join(thid, NULL);

    msgctl(msqid, IPC_RMID, NULL);
    printf("Очередь сообщений удалена\n");
    printf("Программа 2 (получатель) закончила работу.\n");
    return 0;
}
