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

void* thr_fn1(void* arg) {
    struct my_msgbuf msg;
    msg.mtype = 1;
    char hostname[MSG_SIZE];

    while (!fl1) {
        if (gethostname(hostname, sizeof(hostname)) == -1) {
            snprintf(msg.mtext, MSG_SIZE, "Ошибка gethostname: %s", strerror(errno));
        } else {

            if (snprintf(msg.mtext, MSG_SIZE, "Имя хоста: %s", hostname) >= MSG_SIZE) {
                snprintf(msg.mtext, MSG_SIZE, "Ошибка: имя хоста слишком длинное");
            }
        }

        if (msgsnd(msqid, &msg, strlen(msg.mtext) + 1, IPC_NOWAIT) == -1) {
            if (errno == EAGAIN) {
                printf("Очередь сообщений заполнена\n");
            } else {
                perror("Ошибка msgsnd");
            }
        } else {
            printf("Отправлено сообщение: %s\n", msg.mtext);
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
    printf("\nПрограмма завершена (отправитель)\n");
    exit(0);
}

int main() {
    printf("Программа 1 (отправитель) начала работу.\n");
    signal(SIGINT, sig_hand);

    msqid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msqid == -1) {
        perror("Ошибка msgget");
        return 1;
    }

    if (pthread_create(&thid, NULL, thr_fn1, NULL) != 0) {
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
    printf("Программа 1 (отправитель) закончила работу.\n");

    return 0;
}
