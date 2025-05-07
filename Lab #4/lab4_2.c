#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {

    printf("Родительский процесс начал работу\n");
    printf("Идентификатор процесса-родителя: %d\n", getppid());
    printf("Идентификатор текущего процесса: %d\n", getpid());

    pid_t pid = fork();
    int status;

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    else if (pid == 0) {

        char *args[6];
        args[0] = "./lab4_1";
        for (int i = 1; i < argc; i++) {
            args[i] = argv[i];
        }
        args[argc] = NULL;

        char *env[] = {"MY_ENV_VAR=HelloFromParent", NULL};

        if (execve("./lab4_1", args, env) == -1) {
            perror("Ошибка execve");
            exit(EXIT_FAILURE); 
        }

        fprintf(stderr, "Ошибка: execve не вернул управление.\n");
        exit(EXIT_FAILURE);

    } 
    
    else if (pid > 0) {
        
        while (waitpid(pid, &status, WNOHANG)) {
            printf("Ожидание\n");
            usleep(500000);
        }

        printf("Идентификатор дочернего процесса: %d\n", pid);

    }

    printf("Родительский процесс завершил работу\n");
    return 0;
    
}