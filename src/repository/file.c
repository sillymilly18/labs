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
    uint32_t checksum;
} FileHeader;


static uint32_t checksum_sum(const uint8_t *data, size_t len) {
    uint32_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}

static bool write_string(FILE *f, const char *s) {
    uint32_t len = s ? (uint32_t)strlen(s) : 0;
    if (fwrite(&len, sizeof(len), 1, f) != 1) return false;
    if (len > 0 && fwrite(s, 1, len, f) != len) return false;
    return true;
}

bool save_clients(const ClientList *list, const char *filename) {
    FILE *f = fopen(filename, "wb");
    if (!f) return false;


    uint8_t *buffer = NULL;
    size_t bufsize = 0;

    FILE *memf = open_memstream((char **)&buffer, &bufsize);
    if (!memf) { fclose(f); return false; }

    for (size_t i = 0; i < list->count; i++) {
        const Client *c = &list->data[i];

        fwrite(&c->id, sizeof(int32_t), 1, memf);
        write_string(memf, c->name);
        write_string(memf, c->company);
        write_string(memf, c->email);
        write_string(memf, c->phone);
        write_string(memf, c->status);
    }

    fclose(memf);

    // Считаем контр сумму
    uint32_t checksum = checksum_sum(buffer, bufsize);


    FileHeader hdr = {
        .magic = {'C','L','N','T'},
        .version = FILE_VERSION,
        .count = (uint32_t)list->count,
        .next_id = (uint32_t)list->next_id,
        .checksum = checksum
    };

    fwrite(&hdr, sizeof(hdr), 1, f);


    fwrite(buffer, 1, bufsize, f);

    free(buffer);
    fclose(f);
    return true;
}


bool save_deals(const DealList *dl, const char *filename) {
    FILE *f = fopen(filename, "wb");
    if (!f) return false;

    uint8_t *buffer = NULL;
    size_t bufsize = 0;

    FILE *memf = open_memstream((char **)&buffer, &bufsize);
    if (!memf) { fclose(f); return false; }

    for (size_t i = 0; i < dl->count; i++) {
        const Deal *d = &dl->data[i];

        fwrite(&d->id, sizeof(int32_t), 1, memf);
        fwrite(&d->client_id, sizeof(int32_t), 1, memf);
        fwrite(&d->amount, sizeof(double), 1, memf);
        int32_t st = d->status;
        fwrite(&st, sizeof(int32_t), 1, memf);

        write_string(memf, d->title);
        write_string(memf, d->description);
    }

    fclose(memf);

    uint32_t checksum = checksum_sum(buffer, bufsize);

    FileHeader hdr = {
        .magic = {'D','E','A','L'},
        .version = FILE_VERSION,
        .count = (uint32_t)dl->count,
        .next_id = (uint32_t)dl->next_id,
        .checksum = checksum
    };

    fwrite(&hdr, sizeof(hdr), 1, f);
    fwrite(buffer, 1, bufsize, f);

    free(buffer);
    fclose(f);
    return true;
}

bool load_clients(ClientList *list, const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {

        return false;
    }

    FileHeader hdr;
    if (fread(&hdr, sizeof(hdr), 1, f) != 1) {
        printf("Ошибка: файл клиентов повреждён (не удалось прочитать заголовок)\n");
        fclose(f);
        return false;
    }

    if (memcmp(hdr.magic, "CLNT", 4) != 0 || hdr.version != FILE_VERSION) {
        printf("Ошибка: файл клиентов несовместимой версии или испорчен\n");
        fclose(f);
        return false;
    }

    fseek(f, 0, SEEK_END);
    long body_size = ftell(f) - sizeof(FileHeader);
    fseek(f, sizeof(FileHeader), SEEK_SET);

    if (body_size < 0) {
        printf("Ошибка: размер файла клиентов некорректен\n");
        fclose(f);
        return false;
    }

    uint8_t *buffer = malloc(body_size);
    if (!buffer) {
        printf("Ошибка: не хватает памяти для загрузки клиентов\n");
        fclose(f);
        return false;
    }

    if (fread(buffer, 1, body_size, f) != (size_t)body_size) {
        printf("Ошибка: файл клиентов повреждён (не удалось прочитать содержимое)\n");
        free(buffer);
        fclose(f);
        return false;
    }

    fclose(f);

    // --- проверяем контрольную сумму ---
    uint32_t real_sum = checksum_sum(buffer, body_size);
    if (real_sum != hdr.checksum) {
        printf("Ошибка: файл клиентов повреждён (контрольная сумма не совпадает)\n");
        free(buffer);
        return false;
    }


    free_clients_list(list);

    list->count    = hdr.count;
    list->capacity = hdr.count;
    list->next_id  = 1;

    if (hdr.count == 0) {
        free(buffer);
        return true;
    }


    list->data = calloc(hdr.count, sizeof(Client));
    if (!list->data) {
        printf("Ошибка: не удалось выделить память для клиентов\n");
        free(buffer);
        return false;
    }


    size_t pos = 0;
    int max_id = 0;

    for (size_t i = 0; i < hdr.count; i++) {
        Client *c = &list->data[i];

        if (pos + sizeof(int32_t) > (size_t)body_size) {
            printf("Ошибка: файл клиентов повреждён (нехватка данных)\n");
            free(buffer);
            return false;
        }

        memcpy(&c->id, buffer + pos, sizeof(int32_t));
        pos += sizeof(int32_t);

        if (c->id > max_id)
            max_id = c->id;


        uint32_t len;

        memcpy(&len, buffer + pos, sizeof(len));
        pos += sizeof(len);
        if (len > 0) {
            c->name = malloc(len + 1);
            memcpy(c->name, buffer + pos, len);
            c->name[len] = '\0';
            pos += len;
        } else c->name = NULL;


        memcpy(&len, buffer + pos, sizeof(len));
        pos += sizeof(len);
        if (len > 0) {
            c->company = malloc(len + 1);
            memcpy(c->company, buffer + pos, len);
            c->company[len] = '\0';
            pos += len;
        } else c->company = NULL;


        memcpy(&len, buffer + pos, sizeof(len));
        pos += sizeof(len);
        if (len > 0) {
            c->email = malloc(len + 1);
            memcpy(c->email, buffer + pos, len);
            c->email[len] = '\0';
            pos += len;
        } else c->email = NULL;


        memcpy(&len, buffer + pos, sizeof(len));
        pos += sizeof(len);
        if (len > 0) {
            c->phone = malloc(len + 1);
            memcpy(c->phone, buffer + pos, len);
            c->phone[len] = '\0';
            pos += len;
        } else c->phone = NULL;


        memcpy(&len, buffer + pos, sizeof(len));
        pos += sizeof(len);
        if (len > 0) {
            c->status = malloc(len + 1);
            memcpy(c->status, buffer + pos, len);
            c->status[len] = '\0';
            pos += len;
        } else c->status = NULL;
    }

    list->next_id = max_id + 1;

    free(buffer);
    return true;
}


bool load_deals(DealList *dl, const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return false;

    FileHeader hdr;
    if (fread(&hdr, sizeof(hdr), 1, f) != 1) { fclose(f); return false; }

    if (memcmp(hdr.magic, "DEAL", 4) != 0 || hdr.version != FILE_VERSION) {
        fclose(f); return false;
    }

    fseek(f, 0, SEEK_END);
    long filesize = ftell(f) - sizeof(FileHeader);
    fseek(f, sizeof(FileHeader), SEEK_SET);

    uint8_t *buffer = malloc(filesize);
    if (!buffer) { fclose(f); return false; }

    fread(buffer, 1, filesize, f);
    fclose(f);

    uint32_t real = checksum_sum(buffer, filesize);
    if (real != hdr.checksum) {
        printf("Ошибка: файл сделок повреждён (контрольная сумма).\n");
        free(buffer);
        return false;
    }

    free(buffer);
    return true;
}


bool save_all(const ClientList *clients, const DealList *deals) {
    return save_clients(clients, CLIENTS_FILENAME) &&
           save_deals(deals, DEALS_FILENAME);
}

bool load_all(ClientList *clients, DealList *deals) {
    return load_clients(clients, CLIENTS_FILENAME) &&
           load_deals(deals, DEALS_FILENAME);
}


void init_clients_list(ClientList *list) {
    list->data     = NULL;
    list->count    = 0;
    list->capacity = 0;
    list->next_id  = 1;

    if (!load_clients(list, CLIENTS_FILENAME)) {
        printf("Клиенты.данные не найдены или повреждены, создаю пустой список.\n");
    }
}

void init_deals_list(DealList *dl) {
    dl->data     = NULL;
    dl->count    = 0;
    dl->capacity = 0;
    dl->next_id  = 1;

    if (!load_deals(dl, DEALS_FILENAME)) {
        printf("Сделки.данные не найдены или повреждены, создаю пустой список.\n");
    }
}

bool flush_all(ClientList *clients, DealList *deals) {
    if (!clients || !deals) return false;

    FILE *f;

    // Перезаписываем файл клиентов
    f = fopen(CLIENTS_FILENAME, "wb");
    if (!f) return false;
    FileHeader hdr_c = {
        .magic   = {'C','L','N','T'},
        .version = FILE_VERSION,
        .count   = 0,
        .next_id = 1,
        .checksum = 0
    };
    fwrite(&hdr_c, sizeof(hdr_c), 1, f);
    fclose(f);

    // Перезаписываем файл сделок
    f = fopen(DEALS_FILENAME, "wb");
    if (!f) return false;
    FileHeader hdr_d = {
        .magic   = {'D','E','A','L'},
        .version = FILE_VERSION,
        .count   = 0,
        .next_id = 1,
        .checksum = 0
    };
    fwrite(&hdr_d, sizeof(hdr_d), 1, f);
    fclose(f);

    // очищаем списки в оперативной памяти
    free_clients_list(clients);
    free_deals_list(deals);

    return true;
}
