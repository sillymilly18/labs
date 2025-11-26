#pragma once

#include "client.h"
#include "deal.h"

bool flush_all(ClientList *clients, DealList *deals);

bool save_all(const ClientList *clients, const DealList *deals);
bool load_all(ClientList *clients, DealList *deals);

void init_clients_list(ClientList *list);

void init_deals_list(DealList *dl);