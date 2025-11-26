#include "deal.h"
#include <stdlib.h>

#include "input.h"
#include "utils.h"

int ensure_deal_list_capacity(DealList *list, const size_t needed){
  if (list->capacity >= needed) return 1;

  size_t new_cap = list->capacity ? list->capacity * 2 : 4;
  if (new_cap < needed) new_cap = needed;

  Deal *p = realloc(list->data, new_cap * sizeof(Deal));
  if (!p) return 0;

  list->data = p;
  list->capacity = new_cap;

  return 1;
}

void free_deal(const Deal *d) {
  if(!d) return;

  free(d->title); free(d->description);
}

void free_deals_list(DealList *dl) {
  if (!dl) {
    return;
  }

  for (size_t i = 0; i < dl->count; ++i) {
    free_deal(&dl->data[i]);
  }

  free(dl->data);

  dl->data = NULL;
  dl->count = 0;
  dl->capacity = 0;
  dl->next_id = 1;
}

size_t deals_count(const DealList *dl) {
  return dl->count;
}

const char* st_name(const DealStatus s){
  switch(s){
    case DS_TODO: return "запланирована";
    case DS_IN_PROGRESS: return "в работе";
    case DS_DONE: return "завершена";
    case DS_CANCELED: return "отменена";
    default: return "?";
  }
}

int read_status(void) {
  int s;
  in_read_int("Статус (0=TODO,1=IN_PROGRESS,2=DONE,3=CANCELED): ", 0, 3, &s);

  return s;
}

int client_deal_add(DealList *dl, const int client_id, const char *title,
           const char *description, const double amount, const DealStatus st, int *out_id)
{
  if(!ensure_deal_list_capacity(dl, dl->count+1)) return 0;

  const Deal d = {
    .id = dl->next_id++,
    .client_id = client_id,
    .title = copy_string(title),
    .description = copy_string(description),
    .amount = amount,
    .status = st
  };

  if(!d.title || !d.description) {
    free_deal(&d);

    return 0;
  }

  dl->data[dl->count++]=d;
  if(out_id) *out_id=d.id;

  return 1;
}

int deal_index_by_id(const DealList *dl, const int id){
  for(size_t i=0;i<dl->count;++i) {
    if(dl->data[i].id==id) {
      return (int)i;
    };
  };

  return -1;
}

Deal *deal_by_index(const DealList *dl, const size_t index) {
  if (!dl || index >= dl->count) {
    return NULL;
  }

  return &dl->data[index];
}

int deal_remove_at(DealList *dl, const size_t index) {
  if(index>=dl->count) {
    return 0;
  }

  const Deal deal = dl->data[index];
  free_deal(&deal);

  for(size_t i=index+1;i<dl->count;++i) {
    dl->data[i-1]=dl->data[i];
  };

  dl->count--;

  return 1;
}

int deal_update_at(const DealList *dl, const size_t index,
                 const int *new_client_id, const char *new_title,
                 const char *new_description, const double *new_amount,
                 const DealStatus *new_status) {
  if(index>=dl->count) return 0;

  Deal *d=&dl->data[index];

  if(new_client_id) {
    d->client_id = *new_client_id;
  }

  if(new_title) {
    char *t=copy_string(new_title);
    if(!t) return 0;

    free(d->title);

    d->title=t;
  }

  if(new_description) {
    char *t=copy_string(new_description);
    if(!t) return 0;

    free(d->description);

    d->description=t;
  }

  if(new_amount) {
    d->amount = *new_amount;
  }

  if(new_status) {
    d->status = *new_status;
  }

  return 1;
}

size_t dl_remove_by_client(DealList *dl, const int client_id){
  size_t removed=0;
  for(size_t i = 0; i<dl->count;){
    if(dl->data[i].client_id == client_id) {
      deal_remove_at(dl,i);
      ++removed;
    } else {
      ++i;
    };
  }

  return removed;
}