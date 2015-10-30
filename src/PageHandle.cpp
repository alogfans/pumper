// PageHandle.cpp
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// PageHandle support a safer and easy-to-use interface on a raw page (4096 byte)
// And it supports all copy and assignment operators and safe read/write operation
// with different options.

#include "Types.h"
#include "Status.h"
#include "PageHandle.h"
#include "PagedFile.h"

#include <stdio.h>
#include <string.h>

namespace Pumper {
    PageHandle::PageHandle() : is_file_opened(false)
    {
    }

    PageHandle::~PageHandle()
    {
        if (is_file_opened)
            ClosePage();
    }

    PageHandle::PageHandle(const PageHandle &rhs) : is_file_opened(rhs.is_file_opened),
        paged_file(rhs.paged_file), page_id(rhs.page_id), buffer_image(rhs.buffer_image)
    {
    }

    bool PageHandle::operator==(const PageHandle &rhs) const
    {
        if (is_file_opened != rhs.is_file_opened)
            return false;
        if (paged_file != rhs.paged_file)
            return false;
        if (page_id != rhs.page_id)
            return false;
        if (buffer_image != rhs.buffer_image)
            return false;
        return true;
    }

    PageHandle& PageHandle::operator=(const PageHandle &rhs)
    {
        if (*this == rhs)
            return *this;

        // If current a page is opened, first close it to avoid damage
        if (is_file_opened)
            ClosePage();

        // Assignments
        is_file_opened = rhs.is_file_opened;
        paged_file = rhs.paged_file;
        page_id = rhs.page_id;
        buffer_image = rhs.buffer_image;

        return *this;
    }

    Status PageHandle::OpenPage(PagedFile &paged_file, int32_t page_id)
    {
        WARNING_ASSERT(!is_file_opened);
        this->paged_file = &paged_file;
        RETHROW_ON_EXCEPTION(this->paged_file->FetchPage(page_id, &buffer_image));
        WARNING_ASSERT(buffer_image);
        this->is_file_opened = true;
        this->page_id = page_id;
        RETURN_SUCCESS();
    }

    Status PageHandle::ClosePage()
    {
        WARNING_ASSERT(is_file_opened);        
        RETHROW_ON_EXCEPTION(paged_file->UnpinPage(page_id));
        is_file_opened = false;
        RETURN_SUCCESS();
    }

    Status PageHandle::Read(int8_t *data, int32_t length, int32_t offset)
    {
        WARNING_ASSERT(is_file_opened);
        WARNING_ASSERT(data);
        WARNING_ASSERT(length >= 0 && offset >= 0 && length + offset <= PAGE_SIZE);

        // TODO: Need locking when multiple PageHandles
        {
            // RegionLock region_lock;
            memcpy(data, buffer_image + offset, length);
        }

        RETURN_SUCCESS();
    }

    Status PageHandle::Write(const int8_t *data, int32_t length, int32_t offset, bool need_force)
    {
        WARNING_ASSERT(is_file_opened);
        WARNING_ASSERT(data);
        WARNING_ASSERT(length >= 0 && offset >= 0 && length + offset <= PAGE_SIZE);

        // TODO: Need locking when multiple PageHandles
        {
            // RegionLock region_lock;
            memcpy(buffer_image + offset, data, length);
        }

        if (need_force) 
        {
            RETHROW_ON_EXCEPTION(paged_file->ForcePage(page_id));
        }
        else 
        {
            RETHROW_ON_EXCEPTION(paged_file->MarkDirty(page_id));
        }
        RETURN_SUCCESS();
    }

    Status PageHandle::ForceUpdate()
    {
        WARNING_ASSERT(is_file_opened);
        RETHROW_ON_EXCEPTION(paged_file->ForcePage(page_id));
        RETURN_SUCCESS();
    }

} // namespace Pumper
