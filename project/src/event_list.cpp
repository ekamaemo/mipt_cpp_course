#include<event_list.h>
namespace nano_edr {
    EventList::~EventList(){
        Clear();
    }

    void EventList::PushBack(const Event& event){
        if (capacity_ != 0 && size_ == capacity_){
            PopFront();
        }

        auto* node = new EventNode(event);
        if (size_ == 0){
            head_ = node;
        } else {
            tail_->next = node;
        }
        tail_ = node;

        ++size_;
    }

    void EventList::PopFront(){
        if (empty()){
            return;
        }
        EventNode *old_head = head_;
        EventNode *new_head = head_->next;
        head_ = new_head;
        
        if (head_ == nullptr){
            tail_ = nullptr;
        }
        --size_;

        delete old_head;
    }

    void EventList::Clear(){
        while (size_ > 0){
            PopFront();
        }
    }
}