#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <sys/queue.h>
#include <fcntl.h>
#include <errno.h>

#define SOCKET_PATH "socket.s"
#define BUFFER_SIZE 1024
#define MAX_HOSTNAME_LEN 256

struct request_entry {
    int request_number;
    char request_data[BUFFER_SIZE];
    STAILQ_ENTRY(request_entry) entries;
};

STAILQ_HEAD(request_head, request_entry);

int listen_sock = -1;
int client_sock = -1;
struct request_head request_queue;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
int accept_thread_flag = 0;
int recv_thread_flag = 0;
int send_thread_flag = 0;

void* recv_thread_func(void* arg) {
    char buffer[BUFFER_SIZE];

    while (!recv_thread_flag) {
        int reccount = recv(client_sock, buffer, sizeof(buffer) - 1, 0);

        if (reccount == -1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("recv");
                sleep(1);
            }
        } else if (reccount == 0) {
            printf("Клиент отключен\n");
            close(client_sock);
            client_sock = -1;
            break;
        } else {
            buffer[reccount] = '\0';
            printf("Полученный запрос: %s\n", buffer);

            int request_num;
            sscanf(buffer, "%d", &request_num);

            struct request_entry *new_entry = malloc(sizeof(struct request_entry));
            new_entry->request_number = request_num;
            strncpy(new_entry->request_data, buffer, BUFFER_SIZE);

            pthread_mutex_lock(&queue_mutex);
            STAILQ_INSERT_TAIL(&request_queue, new_entry, entries);
            pthread_mutex_unlock(&queue_mutex);
        }
    }

    return NULL;
}

void* send_thread_func(void* arg) {
    while (!send_thread_flag) {
        pthread_mutex_lock(&queue_mutex);

        if (!STAILQ_EMPTY(&request_queue)) {
            struct request_entry *first = STAILQ_FIRST(&request_queue);
            int request_num = first->request_number;
            STAILQ_REMOVE_HEAD(&request_queue, entries);
            pthread_mutex_unlock(&queue_mutex);

            char hostname[MAX_HOSTNAME_LEN];
            if (gethostname(hostname, sizeof(hostname)) == -1) {
                perror("gethostname");
                strncpy(hostname, "неизвестно", sizeof(hostname));
            }

            char response[BUFFER_SIZE];
            int written = snprintf(response, sizeof(response),
                     "Запрос %d: Имя хоста: %s",
                     request_num, hostname);
            
            if (written >= sizeof(response)) {
                response[sizeof(response) - 1] = '\0';
                printf("Предупреждение: ответ был усечен\n");
            }

            if (client_sock != -1) {
                int sentcount = send(client_sock, response, strlen(response), 0);
                if (sentcount == -1) {
                    perror("send");
                } else {
                    printf("Отправленный ответ: %s\n", response);
                }
            }

            free(first);
        } else {
            pthread_mutex_unlock(&queue_mutex);
            usleep(100000);
        }
    }

    return NULL;
}

void* accept_thread_func(void* arg) {
    while (!accept_thread_flag) {
        if (client_sock == -1) {
            struct sockaddr_un client_addr;
            socklen_t client_len = sizeof(client_addr);

            client_sock = accept(listen_sock, (struct sockaddr*)&client_addr, &client_len);

            if (client_sock == -1) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("accept");
                }
                usleep(100000);
            } else {
                printf("Клиент подключён через UNIX-сокет: %s\n", client_addr.sun_path);

                pthread_t recv_thread, send_thread;
                recv_thread_flag = 0;
                send_thread_flag = 0;

                pthread_create(&recv_thread, NULL, recv_thread_func, NULL);
                pthread_create(&send_thread, NULL, send_thread_func, NULL);
            }
        } else {
            usleep(100000);
        }
    }

    return NULL;
}

int main() {
    printf("Программа 1 начала работу\n");
    pthread_t accept_thread;

    STAILQ_INIT(&request_queue);

    unlink(SOCKET_PATH);

    listen_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_sock == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (fcntl(listen_sock, F_SETFL, O_NONBLOCK) == -1) {
        perror("fcntl");
        close(listen_sock);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (bind(listen_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(listen_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(listen_sock, 5) == -1) {
        perror("listen");
        close(listen_sock);
        exit(EXIT_FAILURE);
    }

    printf("Сервер запущен (UNIX-сокет: %s).\n", SOCKET_PATH);

    accept_thread_flag = 0;
    pthread_create(&accept_thread, NULL, accept_thread_func, NULL);

    printf("Ожидание нажатия клавиши\n");
    getchar();
    printf("Клавиша нажата\n");

    accept_thread_flag = 1;
    recv_thread_flag = 1;
    send_thread_flag = 1;

    pthread_join(accept_thread, NULL);

    if (client_sock != -1) {
        shutdown(client_sock, SHUT_RDWR);
        close(client_sock);
    }

    close(listen_sock);
    unlink(SOCKET_PATH);

    struct request_entry *entry;
    while (!STAILQ_EMPTY(&request_queue)) {
        entry = STAILQ_FIRST(&request_queue);
        STAILQ_REMOVE_HEAD(&request_queue, entries);
        free(entry);
    }

    pthread_mutex_destroy(&queue_mutex);
f
    printf("Сервер завершил работу\n");
    printf("Программа 1 завершила работу\n");
    return 0;
}