#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "deal.h"
#include "client.h"

#define CLIENTS_FILENAME "clients.dat"
#define DEALS_FILENAME   "deals.dat"

#define FILE_VERSION 1

typedef struct {
    char     magic[4];
    uint32_t version;
    uint32_t count;
    uint32_t next_id;
} FileHeader;

static bool write_string(FILE *f, const char *s) {
    uint32_t len = 0;
    if (s != NULL) {
        len = (uint32_t)strlen(s);
    }

    if (fwrite(&len, sizeof(len), 1, f) != 1) return false;
    if (len > 0 && fwrite(s, 1, len, f) != len) return false;

    return true;
}

static bool read_string(FILE *f, char **out) {
    uint32_t len = 0;
    if (fread(&len, sizeof(len), 1, f) != 1) return false;

    if (len == 0) {
        *out = NULL;
        return true;
    }

    char *buf = malloc(len + 1);
    if (!buf) return false;

    if (fread(buf, 1, len, f) != len) {
        free(buf);
        return false;
    }
    buf[len] = '\0';
    *out = buf;
    return true;
}

bool save_clients(const ClientList *list, const char *filename) {
    FILE *f = fopen(filename, "wb");
    if (!f) return false;

    const FileHeader hdr = {
        .magic = {'C','L','N','T'},
        .version = FILE_VERSION,
        .count = (uint32_t)list->count,
        .next_id = (uint32_t)list->next_id
    };

    if (fwrite(&hdr, sizeof(hdr), 1, f) != 1) {
        fclose(f);
        return false;
    }

    for (size_t i = 0; i < list->count; i++) {
        const Client *c = &list->data[i];

        int32_t id = c->id;
        if (fwrite(&id, sizeof(id), 1, f) != 1) goto error;

        if (!write_string(f, c->name))    goto error;
        if (!write_string(f, c->company)) goto error;
        if (!write_string(f, c->email))   goto error;
        if (!write_string(f, c->phone))   goto error;
        if (!write_string(f, c->status))  goto error;
    }

    fclose(f);
    return true;

    error:
        fclose(f);
    return false;
}

bool save_deals(const DealList *dl, const char *filename) {
    if (!dl) return false;

    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("fopen deals");
        return false;
    }

    const FileHeader hdr = {
        .magic   = {'D','E','A','L'},
        .version = FILE_VERSION,
        .count   = (uint32_t)dl->count,
        .next_id = (uint32_t)dl->next_id
    };

    if (fwrite(&hdr, sizeof(hdr), 1, f) != 1) {
        fclose(f);
        return false;
    }

    for (size_t i = 0; i < dl->count; ++i) {
        const Deal *d = &dl->data[i];

        int32_t id        = d->id;
        int32_t client_id = d->client_id;
        int32_t status    = (int32_t)d->status;
        double  amount    = d->amount;

        if (fwrite(&id,        sizeof(id),        1, f) != 1 ||
            fwrite(&client_id, sizeof(client_id), 1, f) != 1 ||
            fwrite(&amount,    sizeof(amount),    1, f) != 1 ||
            fwrite(&status,    sizeof(status),    1, f) != 1) {
            fclose(f);
            return false;
            }

        if (!write_string(f, d->title))       { fclose(f); return false; }
        if (!write_string(f, d->description)) { fclose(f); return false; }
    }

    fclose(f);
    return true;
}

bool load_clients(ClientList *list, const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return false;

    FileHeader hdr;
    if (fread(&hdr, sizeof(hdr), 1, f) != 1) {
        fclose(f);
        return false;
    }

    if (memcmp(hdr.magic, "CLNT", 4) != 0 || hdr.version != FILE_VERSION) {
        fclose(f);
        return false;
    }

    free_clients_list(list);

    list->count    = hdr.count;
    list->capacity = hdr.count;
    list->next_id  = 1;

    if (hdr.count == 0) {
        fclose(f);
        return true;
    }

    list->data = calloc(list->capacity, sizeof(Client));
    if (!list->data) {
        fclose(f);
        return false;
    }

    int max_id = 0;
    size_t filled = 0;
    for (; filled < list->count; ++filled) {
        Client *c = &list->data[filled];

        int32_t id;
        if (fread(&id, sizeof(id), 1, f) != 1) goto error;
        c->id = id;
        if (id > max_id) max_id = id;

        if (!read_string(f, &c->name))    goto error;
        if (!read_string(f, &c->company)) goto error;
        if (!read_string(f, &c->email))   goto error;
        if (!read_string(f, &c->phone))   goto error;
        if (!read_string(f, &c->status))  goto error;
    }

    list->next_id = (max_id > 0) ? (max_id + 1) : 1;

    fclose(f);
    return true;

    error:
        for (size_t j = 0; j < filled; ++j) {
            free_client(&list->data[j]);
        }
    free(list->data);

    list->data   = NULL;
    list->count  = 0;
    list->capacity = 0;

    fclose(f);
    return false;
}

bool load_deals(DealList *dl, const char *filename) {
    if (!dl) {
        return false;
    }

    FILE *f = fopen(filename, "rb");
    if (!f) {
        return false;
    }

    FileHeader hdr;
    if (fread(&hdr, sizeof(hdr), 1, f) != 1) {
        fclose(f);
        return false;
    }

    if (memcmp(hdr.magic, "DEAL", 4) != 0 || hdr.version != FILE_VERSION) {
        fclose(f);
        return false;
    }

    free_deals_list(dl);

    dl->count    = hdr.count;
    dl->capacity = hdr.count;
    dl->next_id  = 1;

    if (hdr.count == 0) {
        dl->data = NULL;
        fclose(f);
        return true;
    }

    dl->data = calloc(hdr.count, sizeof(Deal));
    if (!dl->data) {
        fclose(f);
        dl->count = dl->capacity = 0;
        return false;
    }

    int max_id = 0;
    size_t filled = 0;
    for (; filled < dl->count; ++filled) {
        Deal *d = &dl->data[filled];

        int32_t id        = 0;
        int32_t client_id = 0;
        int32_t status    = 0;
        double  amount    = 0.0;

        if (fread(&id,        sizeof(id),        1, f) != 1 ||
            fread(&client_id, sizeof(client_id), 1, f) != 1 ||
            fread(&amount,    sizeof(amount),    1, f) != 1 ||
            fread(&status,    sizeof(status),    1, f) != 1) {
            goto error;
            }

        d->id        = id;
        d->client_id = client_id;
        d->amount    = amount;
        d->status    = (DealStatus)status;

        if (id > max_id) max_id = id;

        if (!read_string(f, &d->title) ||
            !read_string(f, &d->description)) {
            goto error;
            }
    }

    dl->next_id = (max_id > 0) ? (max_id + 1) : 1;

    fclose(f);
    return true;

    error:
        for (size_t j = 0; j < filled; ++j) {
            free_deal(&dl->data[j]);
        }
    free(dl->data);

    dl->data     = NULL;
    dl->count    = 0;
    dl->capacity = 0;
    dl->next_id  = 1;

    fclose(f);
    return false;
}

void init_clients_list(ClientList *list) {
    list->data     = NULL;
    list->count    = 0;
    list->capacity = 0;
    list->next_id  = 1;

    if (!load_clients(list, CLIENTS_FILENAME)) {
        printf(" clients.dat not found or invalid, starting with empty list\n");
    }
}

void init_deals_list(DealList *dl) {
    dl->data     = NULL;
    dl->count    = 0;
    dl->capacity = 0;
    dl->next_id  = 1;

    if (!load_deals(dl, DEALS_FILENAME)) {
        printf(" deals.dat not found or invalid, starting with empty list\n");
    }
}

bool save_all(const ClientList *clients, const DealList *deals) {
    if (!save_clients(clients, CLIENTS_FILENAME)) return false;
    if (!save_deals(deals,   DEALS_FILENAME))   return false;
    return true;
}

bool load_all(ClientList *clients, DealList *deals) {
    const bool ok1 = load_clients(clients, CLIENTS_FILENAME);
    const bool ok2 = load_deals(deals,   DEALS_FILENAME);

    return ok1 && ok2;
}

bool flush_all_clients(ClientList *clients) {
    if (!clients) return false;

    // Очистка памяти
    free_clients_list(clients);

    // Перезаписываем пустой файл
    FILE *f = fopen(CLIENTS_FILENAME, "wb");
    if (!f) return false;

    const FileHeader hdr = {
        .magic   = {'C','L','N','T'},
        .version = FILE_VERSION,
        .count   = 0,
        .next_id = 1
    };

    if (fwrite(&hdr, sizeof(hdr), 1, f) != 1) {
        fclose(f);
        return false;
    }

    fclose(f);
    return true;
}

bool flush_all_deals(DealList *deals) {
    if (!deals) return false;

    free_deals_list(deals);

    FILE *f = fopen(DEALS_FILENAME, "wb");
    if (!f) return false;

    const FileHeader hdr = {
        .magic   = {'D','E','A','L'},
        .version = FILE_VERSION,
        .count   = 0,
        .next_id = 1
    };

    if (fwrite(&hdr, sizeof(hdr), 1, f) != 1) {
        fclose(f);
        return false;
    }

    fclose(f);
    return true;
}

bool flush_all(ClientList *clients, DealList *deals) {
    if (!clients || !deals) return false;

    if (!flush_all_clients(clients)) return false;
    if (!flush_all_deals(deals))   return false;

    return true;
}