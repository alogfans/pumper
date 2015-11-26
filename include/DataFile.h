// DataFile.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Container of all keys and values in one file *.DATA
// Which could also support sequence lookup. Faster
// query, transactions should be located in other comps
// of Pumper.


#ifndef __DATA_FILE_H__
#define __DATA_FILE_H__

#include "Types.h"
#include "Status.h"
#include "Lock.h"

#include <vector>

namespace Pumper {
	class PagedFile;
	
    class DataFile : public noncopyable {
    public:
    	DataFile(PagedFile& paged_file);
    	~DataFile();

    	static Status Create(const String& file);
    	static Status Unlink(const String& file);

        // Open or close one file.
        // In our implementation, a paged file object could open ONLY one file in disk,
        // and one file could be opened by one paged file object. Otherwise the file will
        // be corrupted and the system will shut down. We supply open `memory file` 
        // function, without modify hard disk, but most functions work well like disk.
        Status OpenFile(const String& file);
        Status Close();

        // Will be sequence looking-up, which is very inefficient
        Status Put(const String& key, const String& value);
        Status Get(const String& key, String& value);
        Status Remove(const String& key);
        bool Contains(const String& key);
        std::vector<String> ListKeys();

        // Fast lookup, which specified page_id
        Status Put(int32_t page_id, const String& key, const String& value);
        Status Get(int32_t page_id, const String& key, String& value);
        Status Remove(int32_t page_id, const String& key);
        bool Contains(int32_t page_id, const String& key);
        std::vector<String> ListKeys(int32_t page_id);

    private:
    	PagedFile& paged_file;
    };
} // namespace Pumper

#endif // __DATA_FILE_H__

