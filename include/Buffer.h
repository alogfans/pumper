// Buffer.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Maintaining the background buffer process. Note that there should be only one
// instance of Buffer class. We use Singleton technology to avoid creating more
// than one Buffer in one time.

#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "Types.h"

namespace Pumper {
    // Buffer Object Definition
    class Buffer: public noncopyable {
        // Forward declarations
        class Status;
        class Page;

    public:
        Buffer();
        ~Buffer();

        Buffer& GetBuffer();

        // Fetch a existed page in a opened file discriptor, it will return RAW page shadow
        // in memory. The last param allows user to fetch a page more than once. But be caseful
        // about locks!
        Status FetchPage(int32_t fd, uint32_t page_id, void** page, bool allow_multiple_refs = true);

        // Allocate a new physical page with page number page_id.
        Status AllocatePage(int32_t fd, uint32_t page_id);

        // Decrease the reference counter of this page. If ref counter reaches zero, will 
        // flush this page.
        Status DecreaseReference(int32_t fd, uint32_t page_id);

        // If the page are modified, it should be called for flush or forge.
        Status MarkDirty(int32_t fd, uint32_t page_id);

        // Update all items that marked dirty to disk, and delete these items from buffer.
        Status FlushPages(int32_t fd);

        // Clear all items, will flush all pages to disk.
        Status Clear();

        // Update all items that marked dirty to disk (or single page), but do not remove 
        // it from buffer.
        Status ForgePage(int32_t fd);
        Status ForgePage(int32_t fd, uint32_t page_id);

        // Print debugging information.
        Status PrintDebugInfo();

    private:
        struct BufferChain {
            int32_t previous, next;         // Previous and Next index of the chain
            int32_t ref_count;              // Reference counter

            int32_t fd;
            uint32_t page_id;
            bool is_dirty;
            void * mapping;                 // Mapping to memory area
        };

        const int32_t BUFFER_SIZE = 20;

        BufferChain buffer_chain[BUFFER_SIZE];
        HashTable hash_table(BUFFER_SIZE);
    }; // Buffer

} // namespace Pumper

#endif // __BUFFER_H__

