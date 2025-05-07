#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    printf("Программа lab4_1 начала работу\n");
    printf("Дочерний процесс начал работу\n");
    printf("Идентификатор процесса-родителя: %d\n", getppid());
    printf("Идентификатор текущего процесса: %d\n", getpid());

    char *env_var = getenv("MY_ENV_VAR");
    if (env_var) {
        printf("Получена переменная окружения MY_ENV_VAR: %s\n", env_var);
    } else {
        printf("Переменная окружения MY_ENV_VAR не найдена\n");
    }
    printf("Дочерний процесс завершил работу\n");
    printf("Программа lab4_1 завершила работу\n");
    return 0;
}