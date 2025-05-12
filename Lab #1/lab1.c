#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

void* proc_1(void* arg) {
    printf("Поток №1 начал работу\n");

    char thread_name[16];
    if (pthread_getname_np(pthread_self(), thread_name, sizeof(thread_name)) == 0) {
        printf("Имя потока №1: %s\n", thread_name);
    } else {
        perror("pthread_getname_np");
    }

    const char* new_thread_name = "ThreadOne";
    if (pthread_setname_np(pthread_self(), new_thread_name) == 0) {
        printf("Новое имя потока №1 установлено\n");
    } else {
        perror("pthread_setname_np");
    }


    if (pthread_getname_np(pthread_self(), thread_name, sizeof(thread_name)) == 0) {
        printf("Имя потока №1 после изменения: %s\n", thread_name);
    } else {
        perror("pthread_getname_np");
    }

    int* flag_1 = (int*) arg;

    while(*flag_1 == 0) {
        putchar('1');
        fflush(stdout);
        sleep(1);
    }
    printf("Поток №1 закончил работу\n");

    pthread_exit((void*)1);
}


void* proc_2(void* arg) {
    printf("Поток №2 начал работу\n");

    char thread_name[16];
    if (pthread_getname_np(pthread_self(), thread_name, sizeof(thread_name)) == 0) {
        printf("Имя потока №2: %s\n", thread_name);
    } else {
        perror("pthread_getname_np");
    }

    const char* new_thread_name = "ThreadTwo";
    if (pthread_setname_np(pthread_self(), new_thread_name) == 0) {
        printf("Новое имя потока №2 установлено\n");
    } else {
        perror("pthread_setname_np");
    }

    if (pthread_getname_np(pthread_self(), thread_name, sizeof(thread_name)) == 0) {
        printf("Имя потока №2 после изменения: %s\n", thread_name);
    } else {
        perror("pthread_getname_np");
    }


    int* flag_2 = (int*) arg;

    while (*flag_2 == 0) {
        putchar('2');
        fflush(stdout);
        sleep(1);
    }
    printf("Поток №2 закончил работу\n");

    pthread_exit((void*)2);
}

int main() {
    printf("Программа начала работу\n");

    int flag_1 = 0, flag_2 = 0;
    int exit_code_1, exit_code_2;

    pthread_t id_1, id_2;

    pthread_create(&id_1, NULL, proc_1, &flag_1);
    pthread_create(&id_2, NULL, proc_2, &flag_2);

    printf("Программа ждет нажатия клавиши\n");

    getchar();
    printf("Клавиша нажата\n");

    flag_1 = 1;
    flag_2 = 1;
    pthread_join(id_1, (void**)&exit_code_1);
    pthread_join(id_2, (void**)&exit_code_2);
    
    printf("Первый exitcode = %d\n", exit_code_1);
    printf("Второй exitcode = %d\n", exit_code_2);

    return 0;
}
