// HashTable.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// buffer_chain requires a Hashtable to implement fast search. We use fd and page_id as key, 
// and trying to determine corresponding index of buffer_chain array. 

#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

#include "Types.h"
#include "Status.h"

namespace Pumper {
    class HashTable: public noncopyable {
    public:
        HashTable(uint32_t slots);
        ~HashTable();
        
        // Try to find the key-value set. If found, slot_id will be set to corresponding
        // slot index, otherwise remain unchanged and return false.
        bool TryFind(int32_t fd, int32_t page_id, uint32_t& slot_id);

        // Same as TryFind, but do not return slot id
        bool IsExisted(int32_t fd, int32_t page_id);
        
        // Insert new key-value set. NOT allowed to insert same key twice.
        Status Insert(int32_t fd, int32_t page_id, uint32_t slot_id);
        
        // Remove existed key-value set. 
        Status Remove(int32_t fd, int32_t page_id);

        // Print debugging information.
        Status PrintDebugInfo();

    private:
        uint32_t hash(int32_t fd, int32_t page_id)
        {
            return ((uint32_t) (fd + page_id)) % slots;
        }

        struct HashEntry {
            HashEntry *next, *prev;     // We use linear probing to maintain collasp.
            
            int32_t fd;
            int32_t page_id;

            uint32_t slot_id;
        };
        
        uint32_t slots;
        HashEntry **hash_table;
    }; // HashTable

} // namespace Pumper

#endif // __HASH_TABLE_H__

