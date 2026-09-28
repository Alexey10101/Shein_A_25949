#!/bin/bash

echo "=== Сборка программы ==="
make

echo -e "\n=== 1. Проверка без аргументов ==="
./options

echo -e "\n=== 2. Проверка недопустимой опции (-x) ==="
./options -x

echo -e "\n=== 3. Опции, разделенные знаком минус (-i -p -s) ==="
./options -i -p -s

echo -e "\n=== 4. Слепленные опции (-ips) ==="
./options -ips

echo -e "\n=== 5. Неудачное значение для -U (-U-5) ==="
./options -U-5

echo -e "\n=== 6. Проверка изменения ulimit (-U100 -u) ==="
./options -U100 -u

echo -e "\n=== Очистка ==="
make clean