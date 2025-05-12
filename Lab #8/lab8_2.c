#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>

#define SOCKET_PATH "socket.s"
#define BUFFER_SIZE 1024

typedef struct {
    int sock;
    pthread_mutex_t mutex;
    int is_connected;
    int should_stop;
    pthread_cond_t cond;
} ConnectionState;

void safe_close_socket(ConnectionState *state) {
    pthread_mutex_lock(&state->mutex);
    if (state->sock != -1) {
        shutdown(state->sock, SHUT_RDWR);
        close(state->sock);
        state->sock = -1;
        state->is_connected = 0;
    }
    pthread_mutex_unlock(&state->mutex);
}

void* send_thread_func(void* arg) {
    ConnectionState *state = (ConnectionState*)arg;
    int request_num = 1;

    while (1) {
        pthread_mutex_lock(&state->mutex);
        while (!state->is_connected && !state->should_stop) {
            pthread_cond_wait(&state->cond, &state->mutex);
        }

        if (state->should_stop) {
            pthread_mutex_unlock(&state->mutex);
            break;
        }

        int sock = state->sock;
        pthread_mutex_unlock(&state->mutex);

        char request[BUFFER_SIZE];
        snprintf(request, sizeof(request), "%d", request_num);

        int sentcount = send(sock, request, strlen(request), 0);
        if (sentcount == -1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("send");
                safe_close_socket(state);
            }
        } else {
            printf("Отправленный запрос: %s\n", request);
            request_num++;
        }

        sleep(1);
    }

    return NULL;
}

void* recv_thread_func(void* arg) {
    ConnectionState *state = (ConnectionState*)arg;
    char buffer[BUFFER_SIZE];

    while (1) {
        pthread_mutex_lock(&state->mutex);
        while (!state->is_connected && !state->should_stop) {
            pthread_cond_wait(&state->cond, &state->mutex);
        }

        if (state->should_stop) {
            pthread_mutex_unlock(&state->mutex);
            break;
        }

        int sock = state->sock;
        pthread_mutex_unlock(&state->mutex);

        int reccount = recv(sock, buffer, sizeof(buffer) - 1, 0);

        if (reccount == -1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("recv");
                safe_close_socket(state);
            }
        } else if (reccount == 0) {
            printf("Соединение с сервером потеряно. Пытаемся переподключиться...\n");
            safe_close_socket(state);
        } else {
            buffer[reccount] = '\0';
            printf("Полученный ответ: %s\n", buffer);
        }

        usleep(100000);
    }

    return NULL;
}

void* connect_thread_func(void* arg) {
    ConnectionState *state = (ConnectionState*)arg;

    while (!state->should_stop) {
        pthread_mutex_lock(&state->mutex);
        if (state->sock == -1) {
            int sock = socket(AF_UNIX, SOCK_STREAM, 0);
            if (sock == -1) {
                perror("socket");
                pthread_mutex_unlock(&state->mutex);
                sleep(1);
                continue;
            }

            if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1) {
                perror("fcntl");
                close(sock);
                pthread_mutex_unlock(&state->mutex);
                sleep(1);
                continue;
            }

            state->sock = sock;
        }

        int sock = state->sock;
        pthread_mutex_unlock(&state->mutex);

        struct sockaddr_un server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sun_family = AF_UNIX;
        strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

        int result = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

        if (result == 0) {
            pthread_mutex_lock(&state->mutex);
            state->is_connected = 1;
            printf("Соединен с сервером через UNIX-сокет: %s\n", SOCKET_PATH);
            pthread_cond_broadcast(&state->cond);
            pthread_mutex_unlock(&state->mutex);
        } else if (errno == EINPROGRESS) {
            fd_set writefds;
            FD_ZERO(&writefds);
            FD_SET(sock, &writefds);

            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            if (select(sock + 1, NULL, &writefds, NULL, &timeout) > 0) {
                int error = 0;
                socklen_t len = sizeof(error);
                getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len);

                if (error == 0) {
                    pthread_mutex_lock(&state->mutex);
                    state->is_connected = 1;
                    printf("Соединен с сервером через UNIX-сокет: %s\n", SOCKET_PATH);
                    pthread_cond_broadcast(&state->cond);
                    pthread_mutex_unlock(&state->mutex);
                } else {
                    printf("Нет соединения. Ошибка: %s\n", strerror(error));
                    safe_close_socket(state);
                }
            }
        } else if (errno != EALREADY && errno != EISCONN) {
            perror("connect");
            safe_close_socket(state);
        }

        sleep(1);
    }

    return NULL;
}

int main() {
    printf("Программа 2 начала работу\n");
    ConnectionState state = {
        .sock = -1,
        .is_connected = 0,
        .should_stop = 0,
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .cond = PTHREAD_COND_INITIALIZER
    };

    pthread_t connect_thread, send_thread, recv_thread;
    pthread_create(&connect_thread, NULL, connect_thread_func, &state);
    pthread_create(&send_thread, NULL, send_thread_func, &state);
    pthread_create(&recv_thread, NULL, recv_thread_func, &state);

    printf("Клиент запущен.\n");
    printf("Ожидание нажатия клавиши\n");
    
    getchar();
    
    printf("Клавиша нажата\n");
    pthread_mutex_lock(&state.mutex);
    state.should_stop = 1;
    pthread_cond_broadcast(&state.cond);
    pthread_mutex_unlock(&state.mutex);
    pthread_join(connect_thread, NULL);
    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);
    safe_close_socket(&state);
    pthread_mutex_destroy(&state.mutex);
    pthread_cond_destroy(&state.cond);

    printf("Клиент завершил работу\n");
    printf("Программа 2 завершила работу\n");
    return 0;
}