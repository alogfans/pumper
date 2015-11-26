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
#include "Status.h"
#include "Lock.h"

namespace Pumper {
    // The file header, comsuming the first 32 bytes of file
    struct Header {
        int8_t  magic[8];           // Magic bit, should be `PUMPER\0\0`.
        int32_t alloc_pages;        // Current allocated pages.
        int32_t free_list_head;     // Point to first free page.
        int32_t first_page;         // First page in logical perspective.
        int8_t reserved[17];        // I don't know how to allocate them
        uint16_t checksum;          // For error detection (only for header part).
    };

    class Page;
    // PagedFile Object Definition
    class PagedFile : public noncopyable {        
    public:
        PagedFile();
        ~PagedFile();

        // Create or Unlink the physical file.
        static Status Create(const String& file);
        static Status Unlink(const String& file);

        // Open or close one file.
        // In our implementation, a paged file object could open ONLY one file in disk,
        // and one file could be opened by one paged file object. Otherwise the file will
        // be corrupted and the system will shut down. We supply open `memory file` 
        // function, without modify hard disk, but most functions work well like disk.
        Status OpenFile(const String& file);
        Status Close();
        
        // Allocation management of pages
        Status AllocatePage(int32_t &page_id);
        Status ReleasePage(int32_t page_id);
        
        // Fetch allocated page and do some operations by upper procedures.
        Status FetchPage(int32_t page_id, int8_t** page);
        Status ForcePage(int32_t page_id = ALL_PAGES);
        Status MarkDirty(int32_t page_id);
        Status UnpinPage(int32_t page_id);

        Status SetRootPage(int32_t page_id);
        Status GetRootPage(int32_t &page_id);

        bool IsFileOpened() const;
        int32_t GetTotalPages() const;
    private:
        // calculate the file header checksum
        static uint16_t calculate_checksum(Header *hdr);

        // file discriptor for manipulation.
        bool is_file_opened;
        int32_t fd;

        // memory mapping of header in specific file. All operations that modify this
        // structure should also set the dirty bit, so the destructor function or
        // flush() will write it back.
        Header header_content;
        bool is_header_dirty;

    }; // PagedFile

} // namespace Pumper

#endif // __PAGED_FILE_H__

