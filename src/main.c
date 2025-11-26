#include <stdio.h>
#include <stdbool.h>
#include "client.h"
#include "file.h"
#include "console.h"
#include "input.h"
#include "deal.h"

//набор именованных констант
typedef enum {
    MENU_CLIENTS = 1,
    MENU_DEALS,
    MENU_FILTERS,
    MENU_SAVE_ALL,
    MENU_LOAD_ALL,
    MENU_FLUSH_ALL,
    // should be always at the bottom of enum
    MENU_EXIT,
} RootMenu;

static void print_root_menu() {
    printf("\n===== CRM Light =====\n");
    printf("%d) Клиенты\n", MENU_CLIENTS);
    printf("%d) Сделки\n", MENU_DEALS);
    printf("%d) Фильтры\n", MENU_FILTERS);
    printf("%d) Сохранить данные\n", MENU_SAVE_ALL);
    printf("%d) Загрузить данные\n", MENU_LOAD_ALL);
    printf("%d) Удалить данные\n", MENU_FLUSH_ALL);
    printf("%d) Выход\n", MENU_EXIT);
}

static void filters_menu(const ClientList *clients, const DealList *deals) {
    while (true) {
        printf("\n== Фильтры ==\n");
        printf("1) Сделки клиента\n");
        printf("2) Сделки по статусу\n");
        printf("3) Назад\n");
        int ch; in_read_int("Выбор: ", 1, 3, &ch);      //хранение выбора

        if (ch == 1) {
            int cid;
            in_read_int("ID клиента: ", 1, 1000000000, &cid);

            client_index_by_id(clients, cid) < 0 ? printf("Клиент не найден.\n") : print_client_deals(deals, cid);
        } else if (ch == 2) {
            const DealStatus st = (DealStatus)read_status();

            print_deals_by_status(deals, st);
        } else {
            return;
        };
    }
}

int main(void) {
    ClientList clients;
    init_clients_list(&clients);

    DealList   deals;
    init_deals_list(&deals);

    while (true) {
        print_root_menu();
        int choice;
        in_read_int("Выбор: ", 1, MENU_EXIT, &choice);

        if (choice == MENU_CLIENTS) {
            print_clients_menu(&clients, &deals);
        } else if (choice == MENU_DEALS) {
            print_deals_menu(&clients, &deals);
        } else if (choice == MENU_FILTERS) {
            filters_menu(&clients, &deals);
        } else if (choice == MENU_EXIT) {
            if (in_ask_yes_no("Выйти и очистить ресурсы?")) break;
        } else if (choice == MENU_SAVE_ALL) {
            save_all_menu_option(&clients, &deals);
        } else if (choice == MENU_LOAD_ALL) {
            load_all_menu_option(&clients, &deals);
        } else if (choice == MENU_FLUSH_ALL) {
            flush_all_menu_option(&clients, &deals);
        }
    }

    save_all(&clients, &deals);
    free_deals_list(&deals);
    free_clients_list(&clients);
    printf("📁 Память очищена. Завершение программы.\n");
}
