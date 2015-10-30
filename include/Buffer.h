// Buffer.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Maintaining the background buffer process. Note that there should be only one
// instance of Buffer class. We use Singleton technology to avoid creating more
// than one Buffer in one time.

#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "Types.h"
#include "Status.h"
#include "HashTable.h"

namespace Pumper {
    class Buffer: public noncopyable {
        // Forward declarations
    public:
        Buffer();
        ~Buffer();

        // Singleton interface. Will drop an instance, and constructor is forbidden outside it
        // static Buffer* GetBuffer();

        // Fetch a existed page in a opened file discriptor, it will return RAW page shadow
        // in memory. The last param allows user to fetch a page more than once. But be caseful
        // about locks!
        // It could also allocate a new physical page with page number, but read_physical_page
        // should be set FALSE.
        Status FetchPage(int32_t fd, int32_t page_id, int8_t** page, bool read_physical_page = true,
            bool allow_multiple_pins = true);

        // Unpin a page so that it can be discarded from the buffer.
        Status UnpinPage(int32_t fd, int32_t page_id);

        // If the page are modified, it should be called for flush or forge.
        Status MarkDirty(int32_t fd, int32_t page_id);

        // Update all items that marked dirty to disk, and delete these items from buffer.
        Status FlushPages(int32_t fd);

        // Clear all items, will flush all pages to disk. By default pinned page will not cleared, 
        // except when deconstructor called it
        Status Clear(bool force = false);

        // Update all items that marked dirty to disk (or single page), but do not remove 
        // it from buffer.
        Status ForcePage(int32_t fd, int32_t page_id = ALL_PAGES);

        // Print debugging information.
        Status PrintDebugInfo();

    private:       
        Status unlink_slot(int32_t slot_id);
        Status enqueue_slot(int32_t slot_id);
        Status enqueue_free(int32_t slot_id);
        Status allocate_slot(int32_t& slot_id);
        Status read_page(int32_t fd, int32_t page_id, int8_t* mapping);
        Status write_page(int32_t fd, int32_t page_id, int8_t* mapping);
        
        // static Buffer *instance;

        struct BufferChain {
            int32_t prev, next;             // Previous and Next index of the chain
            int32_t pin_count;              // Pin counter

            int32_t fd;
            int32_t page_id;
            bool is_dirty;
            int8_t * mapping;              // Mapping to memory area
        };

        BufferChain buffer_chain[BUFFER_SIZE];
        HashTable hash_table;

        int32_t free_list_head;             // The first index of free buffer space
        int32_t first, last;                // First and last element in LRU queue
    }; // Buffer

} // namespace Pumper

#endif // __BUFFER_H__

