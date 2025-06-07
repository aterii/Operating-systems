#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define WATCH_DIR "/home/user/audit_test"
#define WATCH_FILE WATCH_DIR "/testfile.txt"
#define MKDIR_PATH WATCH_DIR "/created_dir"
#define KEY_FILE "watch_testfile"
#define KEY_SYSCALL "mkdir_test"

void run_command(const char *desc, const char *cmd) {
    printf("\n%s:\n%s\n", desc, cmd);
    fflush(stdout);
    int ret = system(cmd);
    if (ret != 0) {
        printf("Ошибка при выполнении команды\n");
    }
}

int main() {
    system("sudo truncate -s 0 /var/log/audit/audit.log");
    printf("Аудит действий с каталогом и системными вызовами (auditd)\n");

    if (getuid() != 0) {
        printf("Требуются права root. Запустите программу с sudo.\n");
        return 1;
    }

    mkdir(WATCH_DIR, 0755);

    run_command("Удаление предыдущих правил", "auditctl -D");

    char cmd_file[512];
    snprintf(cmd_file, sizeof(cmd_file), "auditctl -w %s -p rwa -k %s", WATCH_FILE, KEY_FILE);
    run_command("Добавление правила для файла", cmd_file);

    char cmd_syscall[512];
    snprintf(cmd_syscall, sizeof(cmd_syscall),
        "auditctl -a exit,always -F arch=b64 -S mkdir -F auid=%d -k %s",
        getuid(), KEY_SYSCALL);
    run_command("Добавление правила для mkdir", cmd_syscall);

    int fd = open(WATCH_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Ошибка при создании файла");
    } else {
        write(fd, "audit test\n", 11);
        close(fd);
    }
    mkdir(MKDIR_PATH, 0755);

    sleep(1);

    run_command("Журнал для файла", "ausearch -k watch_testfile");
    run_command("Журнал для mkdir", "ausearch -k mkdir_test");
    run_command("Удаление правил", "auditctl -D");

    printf("\nПояснения по полям журнала\n");
    printf("type=SYSCALL  - информация о системном вызове\n");
    printf("syscall=83    - номер вызова (mkdir для x86_64)\n");
    printf("success=yes   - успешное выполнение\n");
    printf("auid=1000     - ID аутентификации пользователя\n");
    printf("exe=...       - путь к исполняемому файлу\n");
    printf("name=...      - путь к затронутому объекту\n");

    printf("\nРабота завершена.\n");
    return 0;
}