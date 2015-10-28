// PageHandle.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// PageHandle support a safer and easy-to-use interface on a raw page (4096 byte)
// And it supports all copy and assignment operators and safe read/write operation
// with different options.

#ifndef __PAGED_HANDLE_H__
#define __PAGED_HANDLE_H__

#include "Types.h"
#include "Status.h"

#include <memory>

namespace Pumper {
    // Forward declaration
    class PagedFile;

    class PageHandle {        
    public:
        PageHandle();
        ~PageHandle();

        Status OpenPage(PagedFile *paged_file, int32_t page_id);
        Status ClosePage();

        // Copy assignments and comparator
        PageHandle(const PageHandle &rhs);
        bool operator==(const PageHandle &rhs) const;
        PageHandle& operator=(const PageHandle &rhs);

        // Replace low-level memory operations so that user could easily access the page
        // without worrying about out-of-region, etc. And they do not care about if it becomes
        // dirty or not.
        // Copying particular data to user. Will NOT modify the page content
        Status Read(int8_t *data, int32_t length, int32_t offset = 0);
        // Copying particular user data to page area. Will modify the page content.
        Status Write(int8_t *data, int32_t length, int32_t offset = 0, bool need_force = false);

        // Misc.
        Status ForceUpdate();

    private:
        bool is_file_opened;

        PagedFile *paged_file;
        int32_t page_id;

        // Buffer image of current page. Will managed by all EQUALLED instances
        int8_t *buffer_image;
    }; // PageHandle

} // namespace Pumper

#endif // __PAGED_HANDLE_H__

