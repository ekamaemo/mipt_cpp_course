#include<event_list.h>
namespace nano_edr {
    void ListPushBack(EventList *list, const Event *event){
        if (list->capacity > 0 && list->size == list->capacity){
            ListPopFront(list);
        }

        EventNode *event_in_list = new EventNode;
        event_in_list -> event = *event;
        if (list->size == 0){
            list->head = event_in_list;
        } else {
            list->tail->next = event_in_list;
        }
        list->tail = event_in_list;

        ++list->size;
    }

    void ListPopFront(EventList* list){
        if (list->size >= 1){
            EventNode *old_head = list-> head;
            EventNode *new_head = list->head->next;
            list->head = new_head;
            --list->size;
            if (list->size == 0){
                list->tail = nullptr;
            }

            delete old_head;
        }
    }

    void ListClear(EventList* list){
        while (list->size >0){
            ListPopFront(list);
        }
    }

    EventList::~EventList() {
        ListClear(this);
    }
}