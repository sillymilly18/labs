#pragma once

#include "client.h"
#include "deal.h"
#include <stddef.h>

// CRUD операции клиента
int  client_add(ClientList *list, const char *name, const char *company, const char *email, const char *phone, const char *status, int *out_id);
int  client_index_by_id(const ClientList *list, int id);
int  client_remove_at(ClientList *list, size_t index);
int  client_update_at(ClientList *list, size_t index, const char *new_name, const char *new_company, const char *new_email, const char *new_phone, const char *new_status);
int  client_deal_add(DealList *dl, int client_id, const char *title,
            const char *description, double amount, DealStatus st, int *out_id);

// CRUD операции сделки клиента
int  deal_index_by_id(const DealList *dl, int id);
int  deal_remove_at(DealList *dl, size_t index);
int  deal_update_at(const DealList *dl, size_t index,
                  const int *new_client_id, const char *new_title,
                  const char *new_description, const double *new_amount,
                  const DealStatus *new_status);
// Функция для удаления сделок клиента. Возвращает, сколько удалено сделок у клиента
size_t dl_remove_by_client(DealList *dl, int client_id);

// Вывод информации
void print_client(const ClientList *list, size_t index);
void print_all_clients(const ClientList *list);

void print_clients_menu(ClientList *clients, DealList *deals);

void print_deal(const DealList *dl, size_t index);
void print_all_deals(const DealList *dl);
void print_client_deals(const DealList *dl, int client_id);
void print_deals_by_status(const DealList *dl, DealStatus st);
void print_deals_menu(const ClientList *clients, DealList *deals);

void flush_all_menu_option(ClientList *clients, DealList *deals);
void load_all_menu_option(ClientList *clients, DealList *deals);
void save_all_menu_option(const ClientList *clients, const DealList *deals);