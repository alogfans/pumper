// Buffer.cpp
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Maintaining the background buffer process. Note that there should be only one
// instance of Buffer class. We use Singleton technology to avoid creating more
// than one Buffer in one time.

#include "Types.h"
#include "Status.h"
#include "Buffer.h"
#include "HashTable.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

namespace Pumper {    

    Buffer::Buffer()
    {
        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            buffer_chain[i].prev = (i == 0 ? INVALID_PAGE_ID : i - 1);
            buffer_chain[i].next = (i == BUFFER_SIZE - 1 ? INVALID_PAGE_ID : i + 1);

            buffer_chain[i].mapping = new int8_t[PAGE_SIZE];
            ERROR_ASSERT(buffer_chain[i].mapping);
            memset(buffer_chain[i].mapping, 0, PAGE_SIZE);
            buffer_chain[i].fd = INVALID_FD;
            buffer_chain[i].page_id = INVALID_PAGE_ID;
            buffer_chain[i].pin_count = 0;
        }

        free_list_head = 0;
        first = INVALID_SLOT_ID;
        last = INVALID_SLOT_ID;
    }

    Buffer::~Buffer()
    {
        Clear(true);
        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            if (buffer_chain[i].mapping)
                delete[] buffer_chain[i].mapping;
        }
    }

    /*   
    // Singleton object
    Buffer* Buffer::instance = NULL;
    Buffer* Buffer::GetBuffer()
    {
        if (instance == NULL)
            instance = new Buffer();

        ERROR_ASSERT(instance);
        return instance;
    }
    */
    
    Status Buffer::FetchPage(int32_t fd, int32_t page_id, int8_t** page, bool read_physical_page, 
        bool allow_multiple_pins)
    {
        int32_t slot_id = 0;
        if (hash_table.TryFind(fd, page_id, slot_id))
        {
            // The slot exists, we should check if it's able to pin. Then pin it in the buffer
            WARNING_ASSERT(slot_id >= 0 && slot_id < BUFFER_SIZE);
            WARNING_ASSERT(!(allow_multiple_pins == false && buffer_chain[slot_id].pin_count > 0));
            buffer_chain[slot_id].pin_count++;

            // Putting in front of LRU queue
            RETHROW_ON_EXCEPTION(unlink_slot(slot_id));
            RETHROW_ON_EXCEPTION(enqueue_slot(slot_id));
        }
        else
        {
            // Should allocate new slot. Someone may be victimed
            RETHROW_ON_EXCEPTION(allocate_slot(slot_id));

            Status status = STATUS_SUCCESS;
            if (read_physical_page)            
                status = read_page(fd, page_id, buffer_chain[slot_id].mapping);

            if (status == STATUS_SUCCESS)
                status = hash_table.Insert(fd, page_id, slot_id);

            if (!(status == STATUS_SUCCESS))
            {
                RETHROW_ON_EXCEPTION(unlink_slot(slot_id));
                RETHROW_ON_EXCEPTION(enqueue_free(slot_id));
                return status;
            }

            // RETHROW_ON_EXCEPTION(enqueue_slot(slot_id));
            // Initialize slot entry
            buffer_chain[slot_id].fd = fd;
            buffer_chain[slot_id].page_id = page_id;
            buffer_chain[slot_id].is_dirty = false;
            buffer_chain[slot_id].pin_count = 1;
        }

        *page = buffer_chain[slot_id].mapping;
        // PrintDebugInfo();
        RETURN_SUCCESS();
    }

    Status Buffer::UnpinPage(int32_t fd, int32_t page_id)
    {
        int32_t slot_id = 0;
        WARNING_ASSERT(hash_table.TryFind(fd, page_id, slot_id));
        WARNING_ASSERT(buffer_chain[slot_id].pin_count > 0);
        if ((--buffer_chain[slot_id].pin_count) == 0)
        {
            RETHROW_ON_EXCEPTION(unlink_slot(slot_id));
            RETHROW_ON_EXCEPTION(enqueue_slot(slot_id));
        }
        // PrintDebugInfo();
        RETURN_SUCCESS();
    }

    Status Buffer::MarkDirty(int32_t fd, int32_t page_id)
    {
        int32_t slot_id = 0;
        WARNING_ASSERT(hash_table.TryFind(fd, page_id, slot_id));
        WARNING_ASSERT(buffer_chain[slot_id].pin_count);
        buffer_chain[slot_id].is_dirty = true;
        RETHROW_ON_EXCEPTION(unlink_slot(slot_id));
        RETHROW_ON_EXCEPTION(enqueue_slot(slot_id));
        // PrintDebugInfo();
        RETURN_SUCCESS();
    }

    Status Buffer::FlushPages(int32_t fd)
    {
        int32_t slot_id = first;
        while (slot_id != INVALID_SLOT_ID)
        {
            int32_t next = buffer_chain[slot_id].next;
            if (buffer_chain[slot_id].fd == fd )
            {
                if (buffer_chain[slot_id].pin_count)
                {
                    // If there is a page pinned, it's NOT expected, but we don't
                    // want to interrupt the process.
                    // Will use logger to replace direct screen output.
                    printf("Unexpected pinned page that unable to flush: fd=%d, page=%d\n", 
                        fd, buffer_chain[slot_id].page_id);
                }
                else
                {
                    if (buffer_chain[slot_id].is_dirty)
                    {
                        RETHROW_ON_EXCEPTION(write_page(fd, buffer_chain[slot_id].page_id, 
                            buffer_chain[slot_id].mapping));
                        buffer_chain[slot_id].is_dirty = false;
                    }

                    RETHROW_ON_EXCEPTION(hash_table.Remove(buffer_chain[slot_id].fd,
                        buffer_chain[slot_id].page_id));
                    RETHROW_ON_EXCEPTION(unlink_slot(slot_id));
                    RETHROW_ON_EXCEPTION(enqueue_free(slot_id));      
                }
            }

            slot_id = next;
        }
        // PrintDebugInfo();
        RETURN_SUCCESS();
    }

    Status Buffer::Clear(bool force)
    {
        int32_t slot_id = first;
        while (slot_id != INVALID_SLOT_ID)
        {
            int32_t next = buffer_chain[slot_id].next;
            if (force || !buffer_chain[slot_id].pin_count)
            {
                RETHROW_ON_EXCEPTION(hash_table.Remove(buffer_chain[slot_id].fd, 
                    buffer_chain[slot_id].page_id));
                RETHROW_ON_EXCEPTION(unlink_slot(slot_id));
                RETHROW_ON_EXCEPTION(enqueue_free(slot_id));                
            }

            slot_id = next;
        }
        // PrintDebugInfo();
        RETURN_SUCCESS();
    }

    Status Buffer::ForcePage(int32_t fd, int32_t page_id)
    {
        int32_t slot_id = first;
        while (slot_id != INVALID_SLOT_ID)
        {
            int32_t next = buffer_chain[slot_id].next;
            if (buffer_chain[slot_id].fd == fd && (page_id == ALL_PAGES || 
                buffer_chain[slot_id].page_id == page_id))
            {
                if (buffer_chain[slot_id].is_dirty)
                {
                    RETHROW_ON_EXCEPTION(write_page(fd, buffer_chain[slot_id].page_id, 
                        buffer_chain[slot_id].mapping));
                    buffer_chain[slot_id].is_dirty = false;
                }
            }

            slot_id = next;
        }
        RETURN_SUCCESS();
    }

    Status Buffer::PrintDebugInfo()
    {
        printf("Buffer DebugInfo\n");
        printf("slot_id fd page_id pin_count is_dirty\n");

        int32_t slot_id = first;
        while (slot_id != INVALID_SLOT_ID)
        {
            printf("%7d %2d %7d %9d %8d\n", 
                slot_id,
                buffer_chain[slot_id].fd,
                buffer_chain[slot_id].page_id,
                buffer_chain[slot_id].pin_count,
                buffer_chain[slot_id].is_dirty);

            slot_id = buffer_chain[slot_id].next;
        }
        RETURN_SUCCESS();
    }

    Status Buffer::unlink_slot(int32_t slot_id)
    {
        WARNING_ASSERT(slot_id >= 0 && slot_id < BUFFER_SIZE);

        if (first == slot_id)
            first = buffer_chain[slot_id].next;
        if (last == slot_id)
            last = buffer_chain[slot_id].prev;

        if (buffer_chain[slot_id].next != INVALID_SLOT_ID)
            buffer_chain[ buffer_chain[slot_id].next ].prev = buffer_chain[slot_id].prev;
        if (buffer_chain[slot_id].prev != INVALID_SLOT_ID)
            buffer_chain[ buffer_chain[slot_id].prev ].next = buffer_chain[slot_id].next;

        buffer_chain[slot_id].prev = buffer_chain[slot_id].next = INVALID_SLOT_ID;

        RETURN_SUCCESS();
    }

    Status Buffer::enqueue_slot(int32_t slot_id)
    {
        WARNING_ASSERT(slot_id >= 0 && slot_id < BUFFER_SIZE);

        buffer_chain[slot_id].next = first;
        buffer_chain[slot_id].prev = INVALID_SLOT_ID;

        if (first != INVALID_SLOT_ID)
            buffer_chain[first].prev = slot_id;
        first = slot_id;
        if (last == INVALID_SLOT_ID)
            last = first;

        RETURN_SUCCESS();
    }

    Status Buffer::enqueue_free(int32_t slot_id)
    {
        WARNING_ASSERT(slot_id >= 0 && slot_id < BUFFER_SIZE);

        buffer_chain[slot_id].next = free_list_head;
        free_list_head = slot_id;

        RETURN_SUCCESS();
    }

    Status Buffer::allocate_slot(int32_t& slot_id)
    {
        // If there is element in free list, reuse it
        if (free_list_head != INVALID_SLOT_ID)
        {
            slot_id = free_list_head;
            free_list_head = buffer_chain[free_list_head].next;
        }
        else
        {
            for (slot_id = last; slot_id != INVALID_SLOT_ID; slot_id = buffer_chain[slot_id].prev)
                if (!buffer_chain[slot_id].pin_count)
                    break;

            WARNING_ASSERT(slot_id != INVALID_SLOT_ID);

            if (buffer_chain[slot_id].is_dirty)
                RETHROW_ON_EXCEPTION(ForcePage(buffer_chain[slot_id].fd, buffer_chain[slot_id].page_id));
            RETHROW_ON_EXCEPTION(hash_table.Remove(buffer_chain[slot_id].fd, buffer_chain[slot_id].page_id));
            RETHROW_ON_EXCEPTION(unlink_slot(slot_id));
        }

        RETHROW_ON_EXCEPTION(enqueue_slot(slot_id));
        RETURN_SUCCESS();
    }

    Status Buffer::read_page(int32_t fd, int32_t page_id, int8_t* mapping)
    {
        // printf("Read operation: fd=%d, page=%d\n", fd, page_id);
        int32_t offset = PAGE_ZERO_OFFSET + PAGE_SIZE * page_id;
        WARNING_ASSERT(lseek(fd, offset, SEEK_SET));
        WARNING_ASSERT(read(fd, mapping, PAGE_SIZE) == PAGE_SIZE);
        RETURN_SUCCESS();
    }

    Status Buffer::write_page(int32_t fd, int32_t page_id, int8_t* mapping)
    {
        // printf("Write operation: fd=%d, page=%d\n", fd, page_id);
        int32_t offset = PAGE_ZERO_OFFSET + PAGE_SIZE * page_id;
        WARNING_ASSERT(lseek(fd, offset, SEEK_SET));
        WARNING_ASSERT(write(fd, mapping, PAGE_SIZE) == PAGE_SIZE);
        RETURN_SUCCESS();
    }

} // namespace Pumper

