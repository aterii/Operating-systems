#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>

int flag1 = 0;
int flag2 = 0;

sem_t my_semaphore; 

void *proc1(void* array_ptr) {
    printf("Поток 1 начал работу\n");
    char* array = (char*) array_ptr;

    while (flag1 == 0) {
        sem_wait(&my_semaphore); 
        //критический участок
        for (size_t i = 0; i < strlen(array); i++) {
            printf("%c", array[i]);
            fflush(stdout); 
            sleep(1);
        }
        //вне критического участка

        sem_post(&my_semaphore); 
        sleep(1);
    }

    
    pthread_exit(NULL);
    return NULL;
    printf("Поток 1 закончил работу\n");
}

void *proc2(void* array_ptr) {
    printf("Поток 2 начал работу\n");
    char* array = (char*) array_ptr;


    while (flag2 == 0) {
        sem_wait(&my_semaphore);
        //критический участок
        for (size_t i = 0; i < strlen(array); i++) {
            printf("%c", array[i]);
            fflush(stdout);
            sleep(1);
        }
        //вне критического участка

        sem_post(&my_semaphore);
        sleep(1);
    }

    pthread_exit(NULL);
    return NULL;
    printf("Поток 2 закончил работу\n");
}

int main() {
    printf("Программа начала работу\n");

    pthread_t ID1, ID2;

    char array_1[] = {"11111\0"};
    char array_2[] = {"22222\0"};

    if (sem_init(&my_semaphore, 0, 1) != 0) {
        perror("Ошибка инициализации семафора");
        return 1;
    }

    if (pthread_create(&ID1, NULL, proc1, array_1) != 0) {
        perror("Ошибка создания потока 1");
        return 1;
    }
    if (pthread_create(&ID2, NULL, proc2, array_2) != 0) {
        perror("Ошибка создания потока 2");
        return 1;
    }

    printf("Программа ждет нажатия клавиши\n");

    getchar();

    printf("Клавиша нажата\n");

    flag1 = 1;
    flag2 = 1;

    if (pthread_join(ID1, NULL) != 0) {
        perror("Ошибка ожидания завершения потока 1");
    }
    if (pthread_join(ID2, NULL) != 0) {
        perror("Ошибка ожидания завершения потока 2");
    }

    sem_destroy(&my_semaphore);

    printf("Программа завершила работу\n");
    return 0;
}