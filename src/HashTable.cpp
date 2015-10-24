// HashTable.cpp
// Part of PUMPER, copyright (C) 2015 Alogfans.

#include <stdio.h>
#include <string.h>

#include "HashTable.h"
#include "Status.h"

namespace Pumper {
    HashTable::HashTable(uint32_t slots)
    {
        this->slots = slots;
        ERROR_ASSERT(slots > 0);
        hash_table = new HashEntry* [slots];
        for (uint32_t i = 0; i < slots; i++)
            hash_table[i] = NULL;
    }

    HashTable::~HashTable()
    {
        for (uint32_t i = 0; i < slots; i++)
        {
            HashEntry *entry = hash_table[i];
            while (entry != NULL)
            {
                HashEntry *next = entry->next;
                delete entry;
                entry = next;
            }
        }
        delete [] hash_table;
    }
    
    bool HashTable::TryFind(int32_t fd, int32_t page_id, uint32_t& slot_id)
    {
        uint32_t key = hash(fd, page_id);
        HashEntry *entry = hash_table[key];

        while (entry != NULL)
        {
            HashEntry *next = entry->next;

            if (entry->fd == fd && entry->page_id == page_id)
            {
                slot_id = entry->slot_id;
                return true;
            }

            entry = next;
        }

        return false;
    }
    
    bool HashTable::IsExisted(int32_t fd, int32_t page_id)
    {
        uint32_t slot_id;
        return TryFind(fd, page_id, slot_id);
    }

    Status HashTable::Insert(int32_t fd, int32_t page_id, uint32_t slot_id)
    {
        WARNING_ASSERT(!IsExisted(fd, page_id));
        uint32_t key = hash(fd, page_id);
        HashEntry *new_entry = new HashEntry();
        WARNING_ASSERT(new_entry);

        new_entry->prev = NULL;
        new_entry->next = hash_table[key];
        new_entry->fd = fd;
        new_entry->page_id = page_id;
        new_entry->slot_id = slot_id;

        if (hash_table[key])
            hash_table[key]->prev = new_entry;

        hash_table[key] = new_entry;

        RETURN_SUCCESS();
    }
    
    Status HashTable::Remove(int32_t fd, int32_t page_id)
    {
        uint32_t key = hash(fd, page_id);
        HashEntry *entry = hash_table[key];

        while (entry != NULL)
        {
            HashEntry *next = entry->next;

            if (entry->fd == fd && entry->page_id == page_id)
                break;
            entry = next;
        }

        WARNING_ASSERT(entry);
        if (hash_table[key] == entry)
            hash_table[key] = entry->next;
        if (entry->prev != NULL)
            entry->prev->next = entry->next;
        if (entry->next != NULL)
            entry->next->prev = entry->prev;
        delete entry;

        RETURN_SUCCESS();
    }

    Status HashTable::PrintDebugInfo()
    {
        printf("HashTable Debug Info (fd, page_id, slot_id)\n");
        for (uint32_t i = 0; i < slots; ++i)
        {
            printf("#%d: ", i);

            HashEntry *entry = hash_table[i];
            while (entry != NULL)
            {
                printf("[%d, %d, %d]-> ", 
                    entry->fd, entry->page_id, entry->slot_id);
                HashEntry *next = entry->next;
                entry = next;
            }
            printf("(NULL)\n");
        }
        RETURN_SUCCESS();
    }

} // namespace Pumper

