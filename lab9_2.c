#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Использование: %s <PID процесса lab9_1>\n", argv[0]);
        return 1;
    }

    pid_t pid = atoi(argv[1]);

    printf("lab9_2: UID=%d, EUID=%d\n", getuid(), geteuid());
    printf("lab9_2: Попытка отправить SIGUSR1 процессу с PID=%d\n", pid);

    if (kill(pid, SIGUSR1) == -1) {
        if (errno == EPERM) {
            printf("Ошибка: недостаточно прав для отправки сигнала.\n");
        } else if (errno == ESRCH) {
            printf("Ошибка: процесс с PID %d не найден.\n", pid);
        } else {
            printf("Ошибка при вызове kill(): %s\n", strerror(errno));
        }
        return 1;
    }

    printf("lab9_2: Сигнал SIGUSR1 успешно отправлен.\n");
    return 0;
}
