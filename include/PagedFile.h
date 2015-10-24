// PagedFile.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// There are three types of data file in our system: DATA, INDEX and LOG. Each file support
// different features of PUMPER. These files share the same physical module, like virtual
// memory management in operating systems. The file will expanded when new data came in, and
// we split them to pages. A page with 4096 bytes is the unit that allocate space in this
// level. Remember, do not expect to allocate physically consequent pages, but something like
// page tables will help you.

#ifndef __PAGED_FILE_H__
#define __PAGED_FILE_H__

#include "Types.h"

namespace Pumper {

    // PagedFile Object Definition
    class PagedFile {

        // Forward declarations
        class Status;
        class Page;
    public:
        PagedFile();
        ~PagedFile();

        // Open or close one file.
        // In our implementation, a paged file object could open ONLY one file in disk,
        // and one file could be opened by one paged file object. Otherwise the file will
        // be corrupted and the system will shut down. We supply open `memory file` 
        // function, without modify hard disk, but most functions work well like disk.
        Status OpenFile(const String& file);
        Status OpenMemory();
        Status Close();
        
        // Allocation management of pages
        Status AllocatePage(uint16_t &page_id);
        Status ReleasePage(uint16_t page_id);
        
        // Fetch allocated page and do some operations by upper procedures.
        Status FetchPage(uint16_t page_id, Page &page);

    private:
        // The file header, comsuming the first 32 bytes of file
        struct Header {
            uint8_t magic[8];           // Magic bit, should be `PUMPER\0\0`.
            uint32_t alloc_pages;       // Current allocated pages.
            uint32_t free_list_head;    // Point to first free page.
            uint32_t first_page;        // First page in logical perspective.
            uint8_t reserved[17];       // I don't know how to allocate them
            uint16_t checksum;          // For error detection (only for header part).
        };
        
        // file discriptor for manipulation.
        bool is_file_opened;
        int32_t fd;

        // memory mapping of header in specific file. All operations that modify this
        // structure should also set the dirty bit, so the destructor function or
        // flush() will write it back.
        Header header_content;
        bool is_header_dirty;

        // Future: locking in this layer
        
    }; // PagedFile

} // namespace Pumper

#endif // __PAGED_FILE_H__

