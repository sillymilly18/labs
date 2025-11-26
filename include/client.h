#pragma once

#include <stddef.h>

struct DealList;

typedef struct Client {
    int id;
    char *name;
    char *company;
    char *email;
    char *phone;
    char *status;      // Статус: потенциальный / в работе / закрыт
} Client;

typedef struct ClientList {
    Client *data;      // Динамический массив клиентов
    size_t count;       // Текущее количество клиентов
    size_t capacity;   // Ёмкость массива
    int next_id;       // Счётчик ID
} ClientList;

// Высвобождение памяти
void free_clients_list(ClientList *list);
void free_client(const Client *c);

// Функция для выделения памяти под массив клиентов
int ensure_client_list_capacity(ClientList *list, size_t needed);

Client *client_by_index(const ClientList *clients_list, size_t index);
size_t clients_count(const ClientList *clients_list);