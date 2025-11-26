#include "console.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "input.h"
#include "client.h"
#include "file.h"
#include "deal.h"
#include "utils.h"

void print_client(const ClientList *list, const size_t index) {
    if (index >= clients_count(list)) {
     return;
    }

    const Client *client = client_by_index(list, index);

    printf("\n📇 Клиент #%d\n", client->id);
    printf("Имя: %s\nКомпания: %s\nEmail: %s\nТелефон: %s\nСтатус: %s\n", client->name, client->company, client->email, client->phone, client->status);
}

void print_all_clients(const ClientList *list) {
    if (clients_count(list) == 0) {
        printf("\n[База клиентов пуста]\n");

        return;
    }

    printf("\n📁 Все клиенты:\n");
    for (size_t i = 0; i < clients_count(list); i++) {
        print_client(list, i);
    }
}

void print_clients_menu(ClientList *clients, DealList *deals) {
    while (true) {
        printf("\n== Клиенты ==\n");
        printf("1) Добавить\n");
        printf("2) Удалить\n");
        printf("3) Изменить\n");
        printf("4) Показать одного\n");
        printf("5) Показать всех\n");
        printf("6) Назад\n");
        int ch;
        in_read_int("Выбор: ", 1, 6, &ch);

        if (ch == 1) {
            char *name=NULL, *company=NULL, *email=NULL, *phone=NULL, *status=NULL;

            printf("Имя: ");
            in_read_line(&name);

            printf("Компания: ");
            in_read_line(&company);

            printf("Email: ");
            in_read_line(&email);

            printf("Телефон: ");
            in_read_line(&phone);

            printf("Статус: ");
            in_read_line(&status);

            int id;
            if (client_add(clients, name, company, email, phone, status, &id)) {
                printf("Клиент добавлен (ID: %d)\n", id);
            } else {
                printf("Ошибка добавления\n");
            }

            free(name); free(company); free(email); free(phone); free(status);
        } else if (ch == 2) {
            if (clients->count == 0) {
                printf("Список пуст.\n");

                continue;
            }

            int id;
            in_read_int("ID клиента: ", 1, 1000000000, &id);

            const int idx = client_index_by_id(clients, id);
            if (idx < 0) {
                printf("Клиент не найден.\n");

                continue;
            }

            printf("К удалению:\n");
            print_client(clients, (size_t)idx);

            if (!in_ask_yes_no("Удалить клиента и все его сделки?")) {
                printf("Отменено.\n");

                continue;
            }

            const size_t removed = dl_remove_by_client(deals, id);

            client_remove_at(clients, (size_t)idx);
            printf("Клиент удалён. Каскадом удалено сделок: %zu\n", removed);
        } else if (ch == 3) {
            if (clients->count == 0) {
                printf("Список пуст.\n");

                continue;
            }

            int id;
            in_read_int("ID клиента: ", 1, 1000000000, &id);

            const int idx = client_index_by_id(clients, id);
            if (idx < 0) {
                printf("Клиент не найден.\n");

                continue;
            }

            char *name=NULL, *company=NULL, *email=NULL, *phone=NULL, *status=NULL;
            if (in_ask_yes_no("Изменить имя?")) {
                printf("Новое имя: ");

                in_read_line(&name);
            }
            if (in_ask_yes_no("Изменить компанию?")) {
                printf("Новая компания: ");

                in_read_line(&company);
            }
            if (in_ask_yes_no("Изменить email?")) {
                printf("Новый email: ");

                in_read_line(&email);
            }
            if (in_ask_yes_no("Изменить телефон?")) {
                printf("Новый телефон: ");

                in_read_line(&phone);
            }
            if (in_ask_yes_no("Изменить статус?")) {
                printf("Новый статус: ");

                in_read_line(&status);
            }

            client_update_at(clients, (size_t)idx, name, company, email, phone, status);
            printf("Обновлено.\n");

            free(name); free(company); free(email); free(phone); free(status);
        } else if (ch == 4) {
            int id;
            in_read_int("ID клиента: ", 1, 1000000000, &id);

            const int idx = client_index_by_id(clients, id);

            idx < 0 ? printf("Клиент не найден.\n") : print_client(clients, (size_t)idx);
        } else if (ch == 5) {
            print_all_clients(clients);
        } else { // 6
            return;
        }
    }
}

void print_deal(const DealList *dl, const size_t index){
  const Deal *deal = deal_by_index(dl, index);
  if (!deal) {
    return;
  }

  printf("ID: %d | КлиентID: %d | \"%s\" | %.2f | Статус: %s\nОписание: %s\n",
         deal->id, deal->client_id, deal->title, deal->amount, st_name(deal->status), deal->description);
}

void print_all_deals(const DealList *dl){
  if(deals_count(dl) == 0) {
    printf("[сделок нет]\n");
    return;
  }

  for(size_t i = 0; i < deals_count(dl); ++i){
    print_deal(dl, i);
  }
}

void print_client_deals(const DealList *dl, const int client_id) {
  bool found = false;

  for(size_t i = 0; i < deals_count(dl); ++i) {
    const Deal *deal = deal_by_index(dl, i);
    if(deal->client_id != client_id) {
      continue;
    }

    print_deal(dl,i);

    found=true;
  }

  if(!found) printf("[для клиента %d сделок нет]\n", client_id);
}

void print_deals_by_status(const DealList *dl, const DealStatus st){
  bool found = false;

  for(size_t i=0; i<deals_count(dl); ++i) {
    const Deal *deal = deal_by_index(dl, i);
    if(deal->status != st) {
      continue;
    }

    print_deal(dl,i);

    found = true;
  }

  if(!found) {
    printf("[сделок со статусом нет]\n");
  }
}

void deals_add(const ClientList *clients, DealList *deals) {
  if (clients_count(clients) == 0) {
    printf("Сначала добавьте клиента.\n");

    return;
  }

  int cid;
  in_read_int("ID клиента: ", 1, 1000000000, &cid);

  if (client_index_by_id(clients, cid) < 0) {
    printf("Клиент не найден.\n");

    return;
  }

  char *title=NULL, *desc=NULL;
  printf("Название сделки: ");
  in_read_line(&title);

  printf("Описание: ");
  in_read_line(&desc);

  printf("Сумма (можно 0): ");
  double amount = 0.0;
  if (scanf("%lf", &amount) != 1) amount = 0.0;

  flush_stdin_line();

  const DealStatus st = (DealStatus)read_status();
  int id;
  client_deal_add(deals, cid, title ? title : "", desc ? desc : "", amount, st, &id) ? printf("Сделка добавлена (ID=%d)\n", id) :  printf("Ошибка добавления сделки\n");

  free(title);
  free(desc);
}

void deals_change_status(const DealList *deals) {
  if (deals_count(deals) == 0) {
    printf("Сделок нет.\n");

    return;
  }

  int did;
  in_read_int("ID сделки: ", 1, 1000000000, &did);

  const int idx = deal_index_by_id(deals, did);
  if (idx < 0) {
    printf("Не найдена.\n");

    return;
  }

  const DealStatus st = (DealStatus)read_status();
  printf(deal_update_at(deals, (size_t)idx, NULL, NULL, NULL, NULL, &st) ?  "Статус обновлён\n" : "Ошибка\n");
}

void deals_edit(const DealList *deals, const ClientList *clients) {
  if (deals_count(deals) == 0) {
    printf("Сделок нет.\n");

    return;
  }

  int did;
  in_read_int("ID сделки: ", 1, 1000000000, &did);

  const int idx = deal_index_by_id(deals, did);
  if (idx < 0) {
    printf("Не найдена.\n");

    return;
  }

  int new_cid;
  const int *cid_ptr = NULL;
  char *title=NULL;
  char *desc=NULL;
  double amount;
  const double *amount_ptr = NULL;
  DealStatus st;
  const DealStatus *st_ptr = NULL;

    if (in_ask_yes_no("Сменить клиента?")) {
      in_read_int("Новый client_id: ", 1, 1000000000, &new_cid);
      if (client_index_by_id(clients, new_cid) < 0) {
        printf("Такого клиента нет.\n");
        return;
      }

      cid_ptr = &new_cid;
    }

  if (in_ask_yes_no("Изменить название?")) {
    printf("Новое название: ");
    in_read_line(&title);
  }

  if (in_ask_yes_no("Изменить описание?")) {
    printf("Новое описание: ");
    in_read_line(&desc);
  }

  if (in_ask_yes_no("Изменить сумму?")) {
    printf("Новая сумма: ");
    if (scanf("%lf",&amount)!=1) amount=0.0;

    flush_stdin_line();

    amount_ptr=&amount;
  }

  if (in_ask_yes_no("Изменить статус?")) {
    st=(DealStatus)read_status();

    st_ptr=&st;
  }

  printf(deal_update_at(deals, (size_t)idx, cid_ptr, title, desc, amount_ptr, st_ptr) ? "Сделка обновлена\n" : "Ошибка\n");

  free(title);
  free(desc);
}

void deals_delete(DealList *deals) {
  if (deals_count(deals) == 0) {
    printf("Сделок нет.\n");

    return;
  }

  int did; in_read_int("ID сделки: ", 1, 1000000000, &did);

  const int idx = deal_index_by_id(deals, did);

  if (idx < 0) {
    printf("Не найдена.\n");

    return;
  }

  if (in_ask_yes_no("Удалить сделку?")) {
    printf(deal_remove_at(deals, (size_t)idx) ? "Удалена\n" : "Ошибка\n");
  }
}

void print_deals_menu(const ClientList *clients, DealList *deals) {
    while (true) {
      printf("\n== Сделки ==\n");
      printf("1) Добавить\n");
      printf("2) Изменить статус\n");
      printf("3) Редактировать сделку\n");
      printf("4) Удалить\n");
      printf("5) Показать все\n");
      printf("6) Назад\n");
      int ch;
      in_read_int("Выбор: ", 1, 6, &ch);

      switch (ch) {
        case 1:
          deals_add(clients, deals);
          continue;
        case 2:
          deals_change_status(deals);
          continue;
        case 3:
          deals_edit(deals, clients);
          continue;
        case 4:
          deals_delete(deals);
          continue;
        case 5:
          print_all_deals(deals);
          continue;
        default:
          return;
      }
    }
}

void save_all_menu_option(const ClientList *clients, const DealList *deals) {
  if (!in_ask_yes_no("Вы уверены, что хотите сохранить данные?")) {
    return;
  }

  save_all(clients, deals);
  printf("Файл сохранен.");
}

void load_all_menu_option(ClientList *clients, DealList *deals) {
  if (!in_ask_yes_no("Вы уверены, что хотите перезаписать текущие данные?")) {
    return;
  }

  load_all(clients, deals);
  printf("Загрузили данные из файла.");
}

void flush_all_menu_option(ClientList *clients, DealList *deals) {
  if (!in_ask_yes_no("Вы уверены, что хотите удалить все данные?")) {
    return;
  }

  flush_all(clients, deals);
  printf("Данные удалены.");
}