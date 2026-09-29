#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUFFER_SIZE 512

typedef struct Node {
    char *str;
    struct Node *next;
} Node;

int main(void) {
    char buffer[BUFFER_SIZE];
    Node *head = NULL;
    Node *tail = NULL;
    printf("Введите строки (для завершения начните строку с '.'):\n");

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        // условия остановкии
        if (buffer[0] == '.') {
            break;
        }

        //выделяем памть под узел списка
        Node *new_node = (Node *)malloc(sizeof(Node));
        if (new_node == NULL) {
            perror("malloc for node failed");
            exit(1);
        }

        // ровно под длину строки + '\0'
        size_t len = strlen(buffer);
        new_node->str = (char *)malloc(len + 1);
        if (new_node->str == NULL) {
            perror("malloc for string failed");
            free(new_node);
            exit(1);
        }

        //копируем строку и инициализируем указатель
        strcpy(new_node->str, buffer);
        new_node->next = NULL;

        // вставка в хвст списка
        if (head == NULL) {
            head = new_node;
            tail = new_node;
        } else {
            tail->next = new_node;
            tail = new_node;
        }
    }

    //вывод
    printf("\n--- Содержимое списка ---\n");
    Node *curr = head;
    while (curr != NULL) {
        printf("%s", curr->str);
        curr = curr->next;
    }

    //освбождение памяти
    curr = head;
    while (curr != NULL) {
        Node *temp = curr;
        curr = curr->next;
        free(temp->str); // освобожаем строку
        free(temp);      // потом узел
    }

    return 0;
}