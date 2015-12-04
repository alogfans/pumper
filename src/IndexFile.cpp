// IndexFile.cpp
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Container of all keys and values in one file *.INDEX
// Which could also support random lookup. 

#include "IndexFile.h"

namespace Pumper {
	IndexFile::IndexFile(PagedFile& paged_file) : paged_file(paged_file)
    {
        btree = NULL;
    }

	IndexFile::~IndexFile()
    {
        delete btree;
    }

	Status IndexFile::Create(const String& file)
    {
        RETHROW_ON_EXCEPTION(PagedFile::Create(file));
        RETURN_SUCCESS();
    }

	Status IndexFile::Unlink(const String& file)
    {
        RETHROW_ON_EXCEPTION(PagedFile::Unlink(file));
        RETURN_SUCCESS();
    }


    // Open or close one file.
    // In our implementation, a paged file object could open ONLY one file in disk,
    // and one file could be opened by one paged file object. Otherwise the file will
    // be corrupted and the system will shut down. We supply open `memory file` 
    // function, without modify hard disk, but most functions work well like disk.
    Status IndexFile::OpenFile(const String& file)
    {
        WARNING_ASSERT(!btree);
        RETHROW_ON_EXCEPTION(paged_file.OpenFile(file));
        btree = new BTree(paged_file);
        RETURN_SUCCESS();
    }

    Status IndexFile::Close()
    {
        WARNING_ASSERT(btree);
        RETHROW_ON_EXCEPTION(paged_file.Close());
        delete btree;
        btree = NULL;
        RETURN_SUCCESS();
    }

    Status IndexFile::Put(const String& key, int32_t data_pid)
    {
        btree->Insert(key, data_pid);
        RETURN_SUCCESS();
    }

    bool IndexFile::Exist(const String& key)
    {
        int32_t data_pid;
        if (!btree->Search(key, data_pid))
            return false;
        return true;
    }

    Status IndexFile::Get(const String& key, int32_t &data_pid)
    {
        btree->Search(key, data_pid);
        RETURN_SUCCESS();
    }

    Status IndexFile::Update(const String& key, int32_t data_pid)
    {
        btree->Update(key, data_pid);
        RETURN_SUCCESS();
    }

    Status IndexFile::Remove(const String& key)
    {
        btree->Remove(key);
        RETURN_SUCCESS();
    }


} // namespace Pumper
