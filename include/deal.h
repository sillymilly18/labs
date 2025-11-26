#pragma once
#include <stddef.h>

struct ClientList;

/* Статус сделки */
typedef enum {
    DS_TODO = 0,        // запланирована
    DS_IN_PROGRESS = 1, // в работе
    DS_DONE = 2,        // завершена (успешно)
    DS_CANCELED = 3     // отменена / неуспех
} DealStatus;

/* Одна сделка/таск, всегда привязана к клиенту */
typedef struct Deal {
    int   id;          // уникальный ID сделки
    int   client_id;   // ID клиента-владельца (ссылка на Client.id)
    char *title;       // краткое название
    char *description; // описание/заметки
    double amount;     // сумма (можно 0, если не нужно)
    DealStatus status; // статус сделки
} Deal;

typedef struct DealList {
    Deal   *data;
    size_t  count;
    size_t  capacity;
    int     next_id;
} DealList;

// Высвобождение памяти
void free_deal(const Deal *d);
void free_deals_list(DealList *dl);

// Функция для выделения памяти под массив сделок
int ensure_deal_list_capacity(DealList *list, size_t needed);

// Функция для конвертации enum статуса сделки в строку
const char* st_name(DealStatus s);
// Чтение статуса из консоли
int read_status(void);

Deal *deal_by_index(const DealList *dl, size_t index);
size_t deals_count(const DealList *dl);
