#!/bin/bash

if [[ $EUID -ne 0 ]]; then
    exec sudo "$0" "$@"
fi

USER_NAME=$(logname)


echo "Компиляция программ lab9_1.c и lab9_2.c"
gcc lab9_1.c -o lab9_1
gcc lab9_2.c -o lab9_2

if [ $? -ne 0 ]; then
    echo "Ошибка компиляции!"
    exit 1
fi


echo "Запуск lab9_1 от имени root в фоновом режиме"
./lab9_1 &
PID=$!
sleep 1

echo "PID процесса lab9_1 (запущен от root): $PID"


echo -e "\nПопытка отправки сигнала от пользователя $USER_NAME без CAP_KILL (ожидается ошибка)"
runuser -u "$USER_NAME" -- ./lab9_2 $PID


echo -e "\nУстановка CAP_KILL для lab9_2"
setcap cap_kill=eip ./lab9_2


echo -e "\nПроверка установленных возможностей lab9_2"
getcap ./lab9_2


echo -e "\nПовторная отправка сигнала от пользователя $USER_NAME с CAP_KILL"
runuser -u "$USER_NAME" -- ./lab9_2 $PID


echo -e "\nСброс возможностей lab9_2"
setcap -r ./lab9_2


echo -e "\nПроверка после сброса"
getcap ./lab9_2


echo -e "\nЗавершение lab9_1"
kill $PID