// Engine.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Container of all keys and values in one file *.INDEX
// Which could also support random lookup. 


#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "Types.h"
#include "Status.h"
#include "Lock.h"
#include "DataFile.h"
#include "IndexFile.h"

#include <vector>

namespace Pumper {
    class Engine : public noncopyable {
    public:
    	Engine();
        ~Engine();

        static Status CreateDb(const String& file);
        static Status UnlinkDb(const String& file);

        Status OpenDb(const String& file);
        Status CloseDb();
        Status UpdateChanges();

        Status Put(const String& key, const String& value);
        Status Get(const String& key, String& value);
        Status Remove(const String& key);
        bool Contains(const String& key);
        std::vector<String> ListKeys();

        bool IsOpened();
        String OpenDbName();

    private:
        DataFile * data_file;
        IndexFile * index_file;

        PagedFile data_paged_file, index_paged_file;
        String db_name;
    };
} // namespace Pumper

#endif // __ENGINE_H__

