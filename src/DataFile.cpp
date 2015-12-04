// DataFile.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Container of all keys and values in one file *.DATA
// Which could also support sequence lookup. Faster
// query, transactions should be located in other comps
// of Pumper.

#include "DataFile.h"
#include "PagedFile.h"
#include "PageHandle.h"
#include "Bucket.h"

namespace Pumper {

	DataFile::DataFile(PagedFile& paged_file) : paged_file(paged_file)
    {
        ERROR_ASSERT(!paged_file.IsFileOpened())
    }

	DataFile::~DataFile()
    {

    }

	Status DataFile::Create(const String& file)
    {
        RETHROW_ON_EXCEPTION(PagedFile::Create(file));
        RETURN_SUCCESS();
    }

	Status DataFile::Unlink(const String& file)
    {
        RETHROW_ON_EXCEPTION(PagedFile::Unlink(file));
        RETURN_SUCCESS();
    }

    // Open or close one file.
    // In our implementation, a paged file object could open ONLY one file in disk,
    // and one file could be opened by one paged file object. Otherwise the file will
    // be corrupted and the system will shut down. We supply open `memory file` 
    // function, without modify hard disk, but most functions work well like disk.
    Status DataFile::OpenFile(const String& file)
    {
        RETHROW_ON_EXCEPTION(paged_file.OpenFile(file));
        RETURN_SUCCESS();
    }

    Status DataFile::Close()
    {
        RETHROW_ON_EXCEPTION(paged_file.Close());
        RETURN_SUCCESS();
    }

    Status DataFile::Put(int32_t page_id, const String& key, const String& value)
    {
        PageHandle temp_ph;
        Bucket bucket;
        bool status;
        int8_t bucketblock[PAGE_SIZE] = { 0 };
        RETHROW_ON_EXCEPTION(temp_ph.OpenPage(paged_file, page_id));
        RETHROW_ON_EXCEPTION(temp_ph.Read(bucketblock, PAGE_SIZE));
        RETHROW_ON_EXCEPTION(bucket.Import(bucketblock));
        status = bucket.Put(key, value);
        RETHROW_ON_EXCEPTION(bucket.Export(bucketblock));
        RETHROW_ON_EXCEPTION(temp_ph.Write(bucketblock, PAGE_SIZE));
        RETHROW_ON_EXCEPTION(temp_ph.ClosePage());
        if (status) 
        {
            RETURN_SUCCESS();
        } 
        else
        {
            RETURN_INFORMATION("Out of space");
        }
    }

    Status DataFile::Get(int32_t page_id, const String& key, String& value)
    {
        PageHandle temp_ph;
        Bucket bucket;
        bool status;
        int8_t bucketblock[PAGE_SIZE] = { 0 };
        RETHROW_ON_EXCEPTION(temp_ph.OpenPage(paged_file, page_id));
        RETHROW_ON_EXCEPTION(temp_ph.Read(bucketblock, PAGE_SIZE));
        RETHROW_ON_EXCEPTION(bucket.Import(bucketblock));
        status = bucket.Get(key, value);
        RETHROW_ON_EXCEPTION(temp_ph.ClosePage());
        if (status) 
        {
            RETURN_SUCCESS();
        } 
        else
        {
            RETURN_INFORMATION("Item not found");
        }
    }

    Status DataFile::Remove(int32_t page_id, const String& key)
    {
        PageHandle temp_ph;
        Bucket bucket;
        int8_t bucketblock[PAGE_SIZE] = { 0 };
        RETHROW_ON_EXCEPTION(temp_ph.OpenPage(paged_file, page_id));
        RETHROW_ON_EXCEPTION(temp_ph.Read(bucketblock, PAGE_SIZE));
        RETHROW_ON_EXCEPTION(bucket.Import(bucketblock));
        RETHROW_ON_EXCEPTION(bucket.Remove(key));
        RETHROW_ON_EXCEPTION(bucket.Export(bucketblock));
        RETHROW_ON_EXCEPTION(temp_ph.Write(bucketblock, PAGE_SIZE));
        RETHROW_ON_EXCEPTION(temp_ph.ClosePage());
        RETURN_SUCCESS();
    }

    bool DataFile::Contains(int32_t page_id, const String& key)
    {
        PageHandle temp_ph;
        Bucket bucket;
        int8_t bucketblock[PAGE_SIZE] = { 0 };
        temp_ph.OpenPage(paged_file, page_id);
        temp_ph.Read(bucketblock, PAGE_SIZE);
        bucket.Import(bucketblock);
        return bucket.Exist(key);
    }

    std::vector<String> DataFile::ListKeys(int32_t page_id)
    {
        PageHandle temp_ph;
        Bucket bucket;
        int8_t bucketblock[PAGE_SIZE] = { 0 };
        temp_ph.OpenPage(paged_file, page_id);
        temp_ph.Read(bucketblock, PAGE_SIZE);
        bucket.Import(bucketblock);
        return bucket.ListKeys();
    }

    Status DataFile::Put(const String& key, const String& value)
    {
        int32_t page_id = 0;
        int32_t total_pages = paged_file.GetTotalPages();      
        for (; page_id < total_pages; page_id++)
        {
            if (Contains(page_id, key))
                break;
        }

        if (page_id != total_pages)
        {
            // Entry existed, try to rewrite it, otherwise, like
            // non-existed entry, append it.
            if (Put(page_id, key, value) == STATUS_SUCCESS)
            {
                RETURN_SUCCESS();
            }
            else
            {
                Remove(page_id, key);
            }
        }

        if (total_pages != 0 && Put(total_pages - 1, key, value) == STATUS_SUCCESS)
        {
            RETURN_SUCCESS();
        }

        // Page is not enough, need more page...
        RETHROW_ON_EXCEPTION(paged_file.AllocatePage(page_id));
        RETHROW_ON_EXCEPTION(Put(page_id, key, value));
        RETURN_SUCCESS();
    }

    Status DataFile::Put(const String& key, const String& value, int32_t &page_id, int32_t first_scan)
    {
        page_id = first_scan;
        int32_t total_pages = paged_file.GetTotalPages();      
        for (; page_id < total_pages; page_id++)
        {
            if (Contains(page_id, key))
                break;
        }

        if (page_id != total_pages)
        {
            // Entry existed, try to rewrite it, otherwise, like
            // non-existed entry, append it.
            if (Put(page_id, key, value) == STATUS_SUCCESS)
            {
                RETURN_SUCCESS();
            }
            else
            {
                Remove(page_id, key);
            }
        }

        if (total_pages != 0 && Put(total_pages - 1, key, value) == STATUS_SUCCESS)
        {
            RETURN_SUCCESS();
        }

        // Page is not enough, need more page...
        RETHROW_ON_EXCEPTION(paged_file.AllocatePage(page_id));
        RETHROW_ON_EXCEPTION(Put(page_id, key, value));
        RETURN_SUCCESS();
    }

    Status DataFile::Get(const String& key, String& value)
    {
        int32_t page_id = 0;
        int32_t total_pages = paged_file.GetTotalPages();      
        for (; page_id < total_pages; page_id++)
        {
            if (Contains(page_id, key))
                break;
        }

        if (page_id != total_pages)
        {
            RETHROW_ON_EXCEPTION(Get(page_id, key, value));
            RETURN_SUCCESS();
        }

        RETURN_INFORMATION("Item not found");
    }

    Status DataFile::Remove(const String& key)
    {
        int32_t page_id = 0;
        int32_t total_pages = paged_file.GetTotalPages();      
        for (; page_id < total_pages; page_id++)
        {
            if (Contains(page_id, key))
                break;
        }

        if (page_id != total_pages)
        {
            RETHROW_ON_EXCEPTION(Remove(page_id, key));
            RETURN_SUCCESS();
        }

        RETURN_INFORMATION("Item not found");
    }

    bool DataFile::Contains(const String& key)
    {
        int32_t page_id = 0;
        int32_t total_pages = paged_file.GetTotalPages();      
        for (; page_id < total_pages; page_id++)
        {
            if (Contains(page_id, key))
                return true;
        }
        return false;
    }

    std::vector<String> DataFile::ListKeys()
    {
        std::vector<String> response;
        int32_t page_id = 0;
        int32_t total_pages = paged_file.GetTotalPages();      
        for (; page_id < total_pages; page_id++)
        {
            std::vector<String> dummy;
            dummy = ListKeys(page_id);
            for (uint32_t i = 0; i < dummy.size(); i++)
                response.push_back(dummy[i]);
        }
        return response;
    }

} // namespace Pumper
