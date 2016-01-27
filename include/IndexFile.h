// IndexFile.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Container of all keys and values in one file *.INDEX
// Which could also support random lookup. 


#ifndef __INDEX_FILE_H__
#define __INDEX_FILE_H__

#include "Types.h"
#include "Status.h"
#include "Lock.h"
#include "BTree.h"

namespace Pumper {
    class IndexFile : public noncopyable {
    public:
    	IndexFile(PagedFile& paged_file);
    	~IndexFile();

    	static Status Create(const String& file);
    	static Status Unlink(const String& file);

        // Open or close one file.
        // In our implementation, a paged file object could open ONLY one file in disk,
        // and one file could be opened by one paged file object. Otherwise the file will
        // be corrupted and the system will shut down. We supply open `memory file` 
        // function, without modify hard disk, but most functions work well like disk.
        Status OpenFile(const String& file);
        Status Close();
        Status UpdateChanges();

        Status Put(const String& key, int32_t data_pid);
        Status Get(const String& key, int32_t &data_pid);
        Status Update(const String &key, int32_t new_page_id);
        bool Exist(const String& key);
        Status Remove(const String& key);

    private:
    	PagedFile& paged_file;
        BTree * btree;
    };
} // namespace Pumper

#endif // __INDEX_FILE_H__

