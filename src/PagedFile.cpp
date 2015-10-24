// HashTable.cpp
// Part of PUMPER, copyright (C) 2015 Alogfans.

#include "PagedFile.h"
#include "Status.h"
#include "Buffer.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

namespace Pumper {
    PagedFile::PagedFile() : is_file_opened(false), fd(-2), is_header_dirty(false)
    {
        // static_assert(SIZEOF_HEADER == sizeof(Header));        
        memset(&header_content, 0, SIZEOF_HEADER);
    }

    PagedFile::~PagedFile()
    {
        if (is_file_opened)
            Close();
    }

    Status PagedFile::Create(const String& file)
    {
        int32_t new_fd;
        Header new_header;

        WARNING_ASSERT(!access(file.c_str(), F_OK));
        new_fd = open(file.c_str(), O_CREAT | O_WRONLY, 0664);
        ERROR_ASSERT(new_fd >= 0);
        
        memset(&new_header, 0, SIZEOF_HEADER);
        strncpy(new_header.magic, "PUMPER", 8);

        new_header.free_list_head = INVALID_PAGE_ID;
        new_header.first_page = INVALID_PAGE_ID;

        int32_t header_length;

        // set_checksum(header);
        lseek(new_fd, 0, SEEK_SET);
        header_length = write(new_fd, &new_header, SIZEOF_HEADER);
        ERROR_ASSERT(header_length == SIZEOF_HEADER);

        close(new_fd);
        RETURN_SUCCESS();
    }

    Status PagedFile::Unlink(const String& file)
    {
        WARNING_ASSERT(!unlink(file.c_str()));
        RETURN_SUCCESS();
    }

    Status PagedFile::OpenFile(const String& file)
    {
        ssize_t header_length;

        // Ensure that currently no file is opened.
        WARNING_ASSERT(!is_file_opened);

        // Open the file.
        fd = open(file.c_str(), O_RDWR);
        ERROR_ASSERT(fd >= 0);

        // Update elements
        lseek(fd, 0, SEEK_SET);
        header_length = read(fd, &header_content, SIZEOF_HEADER);
        ERROR_ASSERT(header_length == SIZEOF_HEADER);
        ERROR_ASSERT(!strncmp(header_content.magic, "PUMPER", 8));

        is_file_opened = true;
        is_header_dirty = false;
        RETURN_SUCCESS();
    }

    Status PagedFile::OpenMemory()
    {
        WARNING_ASSERT(!is_file_opened);
        fd = MEMORY_FD;

        RETURN_ERROR("OpenMemory: Not Implemented");

        is_file_opened = true;
        is_header_dirty = false;
        RETURN_SUCCESS();        
    }

    Status PagedFile::Close()
    {
        WARNING_ASSERT(is_file_opened);
        Buffer::GetBuffer()->FlushPages(fd);

        if (is_header_dirty)
        {
            int32_t header_length;
            lseek(fd, 0, SEEK_SET);
            header_length = write(fd, &header_content, SIZEOF_HEADER);
            ERROR_ASSERT(header_length == SIZEOF_HEADER);

            is_header_dirty = false;
        }

        close(fd);
        fd = INVALID_FD;
        is_file_opened = false;
        RETURN_SUCCESS();
    }
    
    Status PagedFile::AllocatePage(int32_t &page_id)
    {
        void * raw_page;

        // if there is a free page, reuse it.
        if (header_content.free_list_head == INVALID_PAGE_ID)
        {
            Buffer::GetBuffer()->AllocatePage(fd, header_content.alloc_pages);
            page_id = header_content.alloc_pages;
            header_content.alloc_pages++;
        }
        else
        {
            page_id = header_content.free_list_head;
            Buffer::GetBuffer()->FetchPage(fd, page_id, &raw_page);
            // For those pages that have been freed, the first 4 bytes will be reserved
            // to the next of free page chain. If it's used, it will become useless.
            header_content.free_list_head = *(int32_t *) raw_page;
            Buffer::GetBuffer()->DecreaseReference(fd, page_id);
        }

        RETURN_SUCCESS();
    }

    Status PagedFile::ReleasePage(int32_t page_id)
    {
        void * raw_page;

        Buffer::GetBuffer()->FetchPage(fd, page_id, &raw_page);
         *(int32_t *) raw_page = header_content.free_list_head;
        Buffer::GetBuffer()->DecreaseReference(fd, page_id);
        header_content.free_list_head = page_id;

        RETURN_SUCCESS();
    }
    
    Status PagedFile::FetchPage(int32_t page_id, Page &page)
    {
        void * raw_page;
        Buffer::GetBuffer()->FetchPage(fd, page_id, &raw_page);
        page.OpenPage(page_id, raw_page);
        RETURN_SUCCESS();
    }

    Status PagedFile::ForgePage(int32_t page_id)
    {
        return Buffer::GetBuffer()->ForgePage(fd, page_id);
    }

} // namespace Pumper

