#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h> 
#include <signal.h> 

sem_t my_semaphore;

typedef struct
{
    int flag;
    char sym;
} targs;

pthread_t id1, id2;

void *proc1(void *arg) {
    targs *args = (targs*) arg;
    printf("Поток 1 начал работу\n");
    sleep(1);
    while (args->flag == 0) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts); 
        ts.tv_sec += 1;  
        //критический участок
        int sem_result = sem_timedwait(&my_semaphore, &ts);

        if (sem_result == 0) {
            for (int i = 0; i < 5; i++) {
                putchar(args->sym);
                fflush(stdout);
                sleep(1);
            }
            sem_post(&my_semaphore);
            //вне критического участка
            sleep(1);
        } else {
            if (errno == ETIMEDOUT) {
                
                if (args->flag == 1) break; 
                usleep(100000);
            } else {
                perror("Поток 1: ошибка sem_timedwait");
                break;
            }
        }
        if (args->flag == 1) break;
    }
    printf("Поток 1 закончил работу\n");
    pthread_exit((void*)9);
    return NULL;
}

void *proc2(void *arg) {
    targs *args = (targs*) arg;
    printf("Поток 2 начал работу\n");
    sleep(1);
    while (args->flag == 0) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts); 
        ts.tv_sec += 1; 
        //критический участок
        int sem_result = sem_timedwait(&my_semaphore, &ts);

        if (sem_result == 0) {
            for (int i = 0; i < 5; i++) {
                putchar(args->sym);
                fflush(stdout);
                sleep(1);
            }
            sem_post(&my_semaphore);
            //вне критического участка
            sleep(1);
        } else {
            if (errno == ETIMEDOUT) {
                if (args->flag == 1) break;
                usleep(100000);
            } else {
                perror("Поток 2: ошибка sem_timedwait");
                break;
            }
        }
        if (args->flag == 1) break;
    }
    printf("Поток 2 закончил работу\n");
    pthread_exit((void*)12);
    return NULL;
}


void sig_handler(int signo) {
    printf("Получен сигнал, завершаем программу...\n");
    exit(0);
}

int main() {
    printf("Программа начала работу\n");
    targs arg1;
    targs arg2;

    if (sem_init(&my_semaphore, 0, 1) != 0) {
        perror("Ошибка инициализации семафора");
        return 1;
    }

    arg1.flag = 0;
    arg1.sym = '1';
    arg2.flag = 0;
    arg2.sym = '2';


    signal(SIGINT, sig_handler);

    if (pthread_create(&id1, NULL, proc1, &arg1) != 0) {
        perror("Ошибка создания потока 1");
        sem_destroy(&my_semaphore);
        return 1;
    }

    if (pthread_create(&id2, NULL, proc2, &arg2) != 0) {
        perror("Ошибка создания потока 2");
        sem_destroy(&my_semaphore);
        pthread_join(id1, NULL);
        return 1;
    }

    printf("Программа ожидает нажатия клавиши\n");
    getchar();
    printf("Клавиша нажата\n");

    arg1.flag = 1;
    arg2.flag = 1;

    void *exitcode1, *exitcode2;
    pthread_join(id1, &exitcode1);
    pthread_join(id2, &exitcode2);

    sem_destroy(&my_semaphore);
    printf("Программа завершила работу\n");

    return 0;
}