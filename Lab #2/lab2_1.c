#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int flag1 = 0;
int flag2 = 0;

void proc1 (void* array_ptr) {

    printf("Поток 1 начал работу\n");
    char* array = (char*)array_ptr;

    while(flag1 == 0) {
        //критический участок
        for (int i = 0; i < strlen(array); i++) {
            printf("%c", array[i]);
            fflush(stdout);
            sleep(1);
        }
        //вне критического участка
        
        sleep(1);
    }

    printf("Поток 1 закончил работу\n");

}

void proc2 (void* array_ptr) {

    printf("Поток 2 начал работу\n");
    char* array = (char*)array_ptr;

    while(flag2 == 0) {
        //критический участок
        for (int i = 0; i < strlen(array); i++) {
            printf("%c", array[i]);
            fflush(stdout);
            sleep(1);
        }
        //вне критического участка
        
        sleep(1);
    }

    printf("Поток 2 закончил работу\n");

}

int main() {
    printf("Программа начала работу\n");

    pthread_t ID1, ID2; 
    char array_1[] = {"11111\0"};
    char array_2[] = {"22222\0"};
    
    pthread_create(&ID1, NULL, proc1, &array_1);
    pthread_create(&ID2, NULL, proc1, &array_2);
    
    printf("Программа ждет нажатия клавиши\n");
    
    getchar();
    
    printf("Клавиша нажата\n");
    
    flag1 = 1;
    flag2 = 1;
    
    pthread_join(ID1, NULL);
    pthread_join(ID2, NULL);
    
    printf("Программа завершила работу\n");
}