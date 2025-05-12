#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>

#define SHM_NAME "/my_shm"
#define SEM_WRITE_NAME "/my_write_sem"
#define SEM_READ_NAME "/my_read_sem"
#define MAX_MSG_SIZE 512

volatile int flag = 0;
pthread_t thread_id;
char *shm_ptr;
int shm_fd;
sem_t *sem_write;
sem_t *sem_read;

void *thread_function(void *arg) {
    for (int i = 0; i < 100; i++) {
        char hostname[MAX_MSG_SIZE];
        if (gethostname(hostname, sizeof(hostname)) != 0) {
            perror("Ошибка gethostname");
            strcpy(hostname, "неизвестный_хост");
        }

        char message[MAX_MSG_SIZE];
        hostname[200] = '\0';
        snprintf(message, sizeof(message), "Имя хоста: %.200s, сообщение #%d", hostname, i + 1);

        if (sem_wait(sem_write) == -1) {
            perror("sem_wait (write_sem)");
            break;
        }

        strncpy(shm_ptr, message, MAX_MSG_SIZE);
        printf("Отправитель: записал сообщение: %s\n", message);

        if (sem_post(sem_read) == -1) {
            perror("sem_post (read_sem)");
            break;
        }

        sleep(1);
    }

    pthread_exit(NULL);
}


void signal_handler(int signo) {
    printf("\nЗавершение работы...\n");
    flag = 1;
    pthread_join(thread_id, NULL);
    sem_post(sem_read);
    sem_close(sem_write);
    sem_close(sem_read);
    sem_unlink(SEM_WRITE_NAME);
    sem_unlink(SEM_READ_NAME);
    munmap(shm_ptr, MAX_MSG_SIZE);
    close(shm_fd);
    shm_unlink(SHM_NAME);
    exit(0);
}

int main() {
    printf("Программа начала работу\n");
    signal(SIGINT, signal_handler);

    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd, MAX_MSG_SIZE) == -1) {
        perror("ftruncate");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    shm_ptr = mmap(NULL, MAX_MSG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    sem_write = sem_open(SEM_WRITE_NAME, O_CREAT | O_RDWR, 0666, 1);
    if (sem_write == SEM_FAILED) {
        perror("sem_open write_sem");
        exit(EXIT_FAILURE);
    }

    sem_read = sem_open(SEM_READ_NAME, O_CREAT | O_RDWR, 0666, 0);
    if (sem_read == SEM_FAILED) {
        perror("sem_open read_sem");
        sem_close(sem_write);
        sem_unlink(SEM_WRITE_NAME);
        exit(EXIT_FAILURE);
    }

    printf("Семафоры и разделяемая память успешно созданы\n");

    if (pthread_create(&thread_id, NULL, thread_function, NULL) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    printf("Программа ждет нажатия клавиши\n");
    getchar();
    printf("Клавиша нажата\n");

    flag = 1;
    pthread_join(thread_id, NULL);

    sem_close(sem_write);
    sem_close(sem_read);
    sem_unlink(SEM_WRITE_NAME);
    sem_unlink(SEM_READ_NAME);
    munmap(shm_ptr, MAX_MSG_SIZE);
    close(shm_fd);
    shm_unlink(SHM_NAME);

    printf("Программа завершила работу\n");
    return 0;
}
