// Bucket.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// In this Key/Value database system, both keys and values are
// Varient string type. It means that all strings will length 
// at max 2000 bytes (in theory). If the strings are short, there
// is less neccessarity for buckets. Otherwise more I/O may be
// required.
//
// Structure: first of all is a counter that identify how many 
// pairs in this bucket, and them follows the string pointer. All
// strings will append backward (from last bytes) and when they met
// in middle area, there is not possible to insert again. Currently
// NO defrag support!

#include "Bucket.h"

namespace Pumper {
    Bucket::Bucket()
    {
        payload = new int8_t[PAGE_SIZE];
        ERROR_ASSERT(payload);
        BucketHeader * hdr = (BucketHeader *) payload;

        hdr->count_entry_ptr = 0;
        hdr->count_slice = 0;
        hdr->first_free_slice = INVALID_PTR; 
    }

    Bucket::~Bucket()
    {
        if (payload)
            delete[] payload;
    }

    bool Bucket::TryPut(const String &key, const String &value)
    {
        int16_t count_entry_ptr = ((BucketHeader *) payload)->count_entry_ptr;

        int8_t rollback[PAGE_SIZE];
        memcpy(rollback, payload, PAGE_SIZE);

        for (int16_t i = 0; i < count_entry_ptr; i++) 
        {
            EntryPtr * entry = get_entry_ptr(i); 
            String ref_key = get_key(entry->key_slice);
            String ref_value = get_key(entry->value_slice);
            if (strcmp(key.c_str(), ref_key.c_str()) == 0)
            {
                remove_key(entry->value_slice);
                if (set_key(entry->value_slice, value) < 0)
                {
                    memcpy(payload, rollback, PAGE_SIZE);
                    return false;
                }
                
                return true;
            }
        }

        // Brand new item will be inserted.
        int32_t idx = insert_entry_ptr();
        if (idx < 0)
            return false;
        EntryPtr * entry = get_entry_ptr(idx);
        if (set_key(entry->key_slice, key) < 0)
        {
            remove_entry_ptr(idx);
            return false;
        }

        if (set_key(entry->value_slice, value) < 0)
        {
            remove_key(entry->key_slice);
            remove_entry_ptr(idx);
            return false;
        }
        return true;

    }

    Status Bucket::Remove(const String &key)
    {
        int16_t count_entry_ptr = ((BucketHeader *) payload)->count_entry_ptr;
        for (int16_t i = 0; i < count_entry_ptr; i++) 
        {
            EntryPtr * entry = get_entry_ptr(i); 
            String ref_key = get_key(entry->key_slice);
            if (strcmp(key.c_str(), ref_key.c_str()) == 0)
            {
                remove_key(entry->key_slice);
                remove_key(entry->value_slice);
                remove_entry_ptr(i);
            }
        }
        RETURN_SUCCESS();
    }

    bool Bucket::Exist(const String &key)
    {
        int16_t count_entry_ptr = ((BucketHeader *) payload)->count_entry_ptr;
        for (int16_t i = 0; i < count_entry_ptr; i++) 
        {
            EntryPtr * entry = get_entry_ptr(i); 
            String ref_key = get_key(entry->key_slice);
            if (strcmp(key.c_str(), ref_key.c_str()) == 0)
                return true;
        }
        return false;
    }

    bool Bucket::TryGet(const String &key, String &value)
    {
        int16_t count_entry_ptr = ((BucketHeader *) payload)->count_entry_ptr;
        for (int16_t i = 0; i < count_entry_ptr; i++) 
        {
            EntryPtr * entry = get_entry_ptr(i); 
            String ref_key = get_key(entry->key_slice);
            if (strcmp(key.c_str(), ref_key.c_str()) == 0)
            {
                value = get_key(entry->value_slice);
                return true;
            }
        }
        return false;
    }

    Status Bucket::Defrag()
    {
        RETURN_SUCCESS();
    }

    Status Bucket::PrintDebugInfo()
    {
        printf("Bucket Debug Info\n");
        BucketHeader * hdr = (BucketHeader *) payload;
        printf("count_entry_ptr = %d, count_slice = %d, first_free_slice = %d\n", 
            hdr->count_entry_ptr, 
            hdr->count_slice, 
            hdr->first_free_slice);

        printf("--- entry ptrs ---\n");
        for (int16_t i = 0; i < hdr->count_entry_ptr; i++)
        {
            EntryPtr * entry_ptr = (EntryPtr *) (payload + sizeof(BucketHeader) + sizeof(EntryPtr) * i);
            printf("%d: key_slice = %d, value_slice = %d\n", 
                i, 
                entry_ptr->key_slice, 
                entry_ptr->value_slice);            
        }
        
        printf("--- string slices ---\n");
        for (int16_t i = 0; i < hdr->count_slice; i++)
        {
            int16_t offset = PAGE_SIZE - sizeof(StringSlice) * (i + 1);
            StringSlice * slice = (StringSlice *) (payload + offset);

            int8_t buf[15] = { 0 };
            memcpy(buf, slice->str_buf, 14);

            printf("%d: data = [%s], next = %d\n", 
                offset, 
                buf, 
                slice->next);
        }

        RETURN_SUCCESS();
    }

    EntryPtr * Bucket::get_entry_ptr(int16_t entry_index)
    {
        return (EntryPtr *) (payload + sizeof(BucketHeader) + entry_index * sizeof(EntryPtr));
    }
    
    int16_t Bucket::insert_entry_ptr()
    {
        int16_t count_entry_ptr = ((BucketHeader *) payload)->count_entry_ptr;

        // We don't want to expand the entry ptr table, so scan first
        for (int16_t i = 0; i < count_entry_ptr; i++)
        {
            EntryPtr * entry = get_entry_ptr(i);
            if (entry->key_slice < 0 && entry->value_slice < 0)
            {
                // Ok, to insert here
                return i;
            }
        }

        // Expanding is required
        int16_t count_slice = ((BucketHeader *) payload)->count_slice;
        if (sizeof(BucketHeader) + (count_entry_ptr + 1) * sizeof(EntryPtr) + 
            (count_slice) * sizeof(StringSlice) > PAGE_SIZE)
        {
            // No space available
            return -1;
        }

        ((BucketHeader *) payload)->count_entry_ptr++;
        return count_entry_ptr;
    }

    void Bucket::remove_entry_ptr(int16_t entry_index)
    {
        int16_t count_entry_ptr = ((BucketHeader *) payload)->count_entry_ptr;
        EntryPtr * entry = get_entry_ptr(entry_index);
        entry->key_slice = -1;
        entry->value_slice = -1;

        // If it's the last entry, shrink is useful and safe
        if (entry_index + 1 == count_entry_ptr)
        {
            ((BucketHeader *) payload)->count_entry_ptr--;
        }
    }

    String Bucket::get_key(int16_t slice_offset)
    {
        String builder = "";

        while (slice_offset > 0) 
        {
            StringSlice * slice = (StringSlice *) (payload + slice_offset);
            builder = builder + String(slice->str_buf, SLICE_LENGTH);

            slice_offset = slice->next;
        }

        return builder;
    }

    int16_t Bucket::set_key(int16_t &slice_offset, const String &value)
    {
        int16_t n_slice = value.size() / (SLICE_LENGTH);
        int16_t *prev_link = NULL;

        int8_t rollback[PAGE_SIZE];
        memcpy(rollback, payload, PAGE_SIZE);

        if (value.size() % (SLICE_LENGTH))
            n_slice++;

        for (int16_t i = 0; i < n_slice; i++)
        {
            int8_t buffer[SLICE_LENGTH] = { 0 };

            for (int16_t j = 0; j < SLICE_LENGTH; j++)
                if (i * SLICE_LENGTH + j < (int16_t) value.size())
                    buffer[j] = value[i * SLICE_LENGTH + j];

            StringSlice * slice;
            int16_t slice_loc;

            if (((BucketHeader *) payload)->first_free_slice > 0)
            {
                slice_loc = ((BucketHeader *) payload)->first_free_slice;
                slice = (StringSlice *) (payload + slice_loc);
                ((BucketHeader *) payload)->first_free_slice = slice->next;
            }
            else
            {
                slice_loc = PAGE_SIZE - sizeof(StringSlice) - 
                    ((BucketHeader *) payload)->count_slice * sizeof(StringSlice);

                if (slice_loc < (int16_t) (sizeof(BucketHeader) + 
                    ((BucketHeader *) payload)->count_entry_ptr * sizeof(EntryPtr)))
                {
                    // Rollback. Not enough space
                    memcpy(payload, rollback, PAGE_SIZE);
                    return -1;
                }

                slice = (StringSlice *) (payload + slice_loc);
                ((BucketHeader *) payload)->count_slice++;
            }
            
            slice->next = -1;
            memcpy(slice->str_buf, buffer, SLICE_LENGTH);

            if (prev_link != NULL)
            {
                *prev_link = slice_loc;
            }
            else
            {
                slice_offset = slice_loc;
            }

            prev_link = &slice->next;
        }

        return 0;
    }

    void Bucket::remove_key(int16_t slice_offset)
    {
        int16_t end_slice_offset = slice_offset;

        while (1) 
        {
            StringSlice * slice = (StringSlice *) (payload + end_slice_offset);
            if (slice->next < 0)
            {
                // The last entry
                slice->next = ((BucketHeader *) payload)->first_free_slice;
                break;
            }
            end_slice_offset = slice->next;
        }

        ((BucketHeader *) payload)->first_free_slice = slice_offset;
    }

} // namespace Pumper

