#include "client.h"
#include "utils.h"
#include <stdlib.h>

int ensure_client_list_capacity(ClientList *list, const size_t needed) {
    if (list->capacity >= needed) return 1;

    size_t new_cap = list->capacity ? list->capacity * 2 : 4;
    if (new_cap < needed) new_cap = needed;

    Client *p = realloc(list->data, new_cap * sizeof(Client));
    if (!p) return 0;

    list->data = p;
    list->capacity = new_cap;

    return 1;
}

void free_client(const Client *c) {
    free(c->name); free(c->company); free(c->email); free(c->phone); free(c->status);
}

void free_clients_list(ClientList *list) {
    for (size_t i = 0; i < list->count; i++) {
        const Client *client = &list->data[i];
        free_client(client);
    }

    free(list->data);
    list->data = NULL;
    list->count = list->capacity = 0;
}

int client_add(ClientList *list, const char *name, const char *company, const char *email, const char *phone, const char *status, int *out_id) {
    if (!ensure_client_list_capacity(list, list->count + 1)) return 0;

    const Client c = {
        .id = list->next_id++,
        .name = copy_string(name),
        .company = copy_string(company),
        .email = copy_string(email),
        .phone = copy_string(phone),
        .status = copy_string(status)
    };

    if (!c.name || !c.company || !c.email || !c.phone || !c.status) {
        free_client(&c);
        return 0;
    }

    list->data[list->count++] = c;
    if (out_id) *out_id = c.id;

    return 1;
}

int client_index_by_id(const ClientList *list, const int id) {
    for (size_t i = 0; i < list->count; i++) {
        if (list->data[i].id == id) return (int)i;
    }

    return -1;
}

int client_remove_at(ClientList *list, const size_t index) {
    if (index >= list->count) return 0;

    const Client *client = &list->data[index];
    free_client(client);

    for (size_t i = index + 1; i < list->count; i++) {
        list->data[i - 1] = list->data[i];
    }

    list->count--;

    return 1;
}

int client_update_at(const ClientList *list, const size_t index, const char *new_name, const char *new_company, const char *new_email, const char *new_phone, const char *new_status) {
    if (index >= list->count) return 0;      // когда пользователь выбрал несуществующего клиента.

    Client *c = &list->data[index];

    if (new_name) {
        free(c->name);

        c->name = copy_string(new_name);
    }

    if (new_company) {
        free(c->company);

        c->company = copy_string(new_company);
    }

    if (new_email) {
        free(c->email);

        c->email = copy_string(new_email);
    }

    if (new_phone) {
        free(c->phone);

        c->phone = copy_string(new_phone);
    }

    if (new_status) {
        free(c->status);

        c->status = copy_string(new_status);
    }

    return 1;
}

Client *client_by_index(const ClientList *clients_list, const size_t index) {
    if (!clients_list || index >= clients_list->count) {
        return NULL;
    }

    return &clients_list->data[index];
}

size_t clients_count(const ClientList *clients_list) {
    return clients_list->count;
}