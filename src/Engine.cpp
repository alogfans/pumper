// Engine.cpp
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Container of all keys and values in one file *.INDEX
// Which could also support random lookup. 

#include "Engine.h"

namespace Pumper {

    Engine::Engine() : data_file(NULL), index_file(NULL)
    {

    }

    Engine::~Engine()
    {
        if (data_file)
            delete data_file;
        if (index_file)
            delete index_file;
    }

    Status Engine::CreateDb(const String& file)
    {
        RETHROW_ON_EXCEPTION(PagedFile::Create(file + ".DATA"));
        RETHROW_ON_EXCEPTION(PagedFile::Create(file + ".INDEX"));
        RETURN_SUCCESS();
    }

    Status Engine::UnlinkDb(const String& file)
    {
        RETHROW_ON_EXCEPTION(PagedFile::Unlink(file + ".DATA"));
        RETHROW_ON_EXCEPTION(PagedFile::Unlink(file + ".INDEX"));
        RETURN_SUCCESS();
    }

    Status Engine::OpenDb(const String& file)
    {
        WARNING_ASSERT(!data_file && !index_file);

        data_file = new DataFile(data_paged_file);
        index_file = new IndexFile(index_paged_file);

        RETHROW_ON_EXCEPTION(data_file->OpenFile(file + ".DATA"));
        RETHROW_ON_EXCEPTION(index_file->OpenFile(file + ".INDEX"));
        RETURN_SUCCESS();
    }

    Status Engine::CloseDb()
    {
        WARNING_ASSERT(data_file && index_file);
        RETHROW_ON_EXCEPTION(data_file->Close());
        RETHROW_ON_EXCEPTION(index_file->Close());
        delete data_file;
        delete index_file;
        data_file = NULL;
        index_file = NULL;
        RETURN_SUCCESS();
    }

    Status Engine::Put(const String& key, const String& value)
    {
        WARNING_ASSERT(data_file && index_file);
        if (index_file->Exist(key))
        {
            // If I just want to update it, or conflict in same page, behave
            // normally is ok.
            int page_id;
            RETHROW_ON_EXCEPTION(index_file->Get(key, page_id));
            if (data_file->Contains(page_id, key))
            {
                if (!(data_file->Put(page_id, key, value) == STATUS_SUCCESS))
                {
                    RETHROW_ON_EXCEPTION(data_file->Remove(page_id, key));
                    RETHROW_ON_EXCEPTION(data_file->Put(key, value, page_id));
                    RETHROW_ON_EXCEPTION(index_file->Update(key, page_id));
                }
            }
            else
            {
                // Otherwise it will fallback.                
                RETHROW_ON_EXCEPTION(data_file->Put(key, value));
                RETHROW_ON_EXCEPTION(index_file->Update(key, page_id | 0x80000000));
            }
        }
        else
        {
            // New entry here. Just insert it normally (very slow)
            int page_id;
            // XXX: To faster insert cost
            static int scan = 0;
            RETHROW_ON_EXCEPTION(data_file->Put(key, value, page_id, scan));
            scan = page_id;
            RETHROW_ON_EXCEPTION(index_file->Put(key, page_id));
        }

        RETURN_SUCCESS();
    }

    Status Engine::Get(const String& key, String& value)
    {
        WARNING_ASSERT(data_file && index_file);
        WARNING_ASSERT(index_file->Exist(key));

        int page_id;
        RETHROW_ON_EXCEPTION(index_file->Get(key, page_id));

        // The highest bit: determine whether there is a conflict
        if (page_id & 0x80000000)
        {            
            if (data_file->Contains(page_id & 0x7fffffff, key)) 
            {
                // Try to fetch major items here
                RETHROW_ON_EXCEPTION(data_file->Get(page_id & 0x7fffffff, key, value));
            }
            else
            {
                // Otherwise, use the slow method as fallback
                RETHROW_ON_EXCEPTION(data_file->Get(key, value));
            }
        }
        else 
        {
            RETHROW_ON_EXCEPTION(data_file->Get(page_id, key, value));
        }

        RETURN_SUCCESS();
    }

    Status Engine::Remove(const String& key)
    {
        WARNING_ASSERT(data_file && index_file);
        WARNING_ASSERT(index_file->Exist(key));

        int page_id;
        RETHROW_ON_EXCEPTION(index_file->Get(key, page_id));

        // The highest bit: determine whether there is a conflict
        if (page_id & 0x80000000)
        {
            if (data_file->Contains(page_id & 0x7fffffff, key)) 
            {
                // Try to fetch major items here
                RETHROW_ON_EXCEPTION(data_file->Remove(page_id & 0x7fffffff, key));
            }
            else
            {
                // Otherwise, use the slow method as fallback
                RETHROW_ON_EXCEPTION(data_file->Remove(key));
            }
        }
        else 
        {
            RETHROW_ON_EXCEPTION(index_file->Remove(key));
            RETHROW_ON_EXCEPTION(data_file->Remove(page_id, key));
        }

        RETURN_SUCCESS();
    }

    bool Engine::Contains(const String& key)
    {
        //WARNING_ASSERT(data_file && index_file);
        // If same hash key is not found, it cannot be existed.
        if (!index_file->Exist(key))
            return false;

        int page_id;
        index_file->Get(key, page_id);

        // The highest bit: determine whether there is a conflict
        if (page_id & 0x80000000)
        {
            // Use the slow method as fallback
            return data_file->Contains(page_id & 0x7fffffff, key) || data_file->Contains(key);
        }
        else 
        {
            return true;
        }
    }

    std::vector<String> Engine::ListKeys()
    {
        //WARNING_ASSERT(data_file && index_file);
        return data_file->ListKeys();
    }


} // namespace Pumper
