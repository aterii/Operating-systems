#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void handler(int signum) {
    printf("lab9_1: получен сигнал %d\n", signum);
}

int main() {
    printf("lab9_1: PID = %d. Ожидание сигнала SIGUSR1...\n", getpid());
    signal(SIGUSR1, handler);
    while (1) {
        pause();
    }
    return 0;
}
