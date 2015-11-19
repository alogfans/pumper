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

#ifndef __BUCKET_H__
#define __BUCKET_H__

#include "Types.h"
#include "Status.h"
#include "Lock.h"

#include <vector>

#define SLICE_LENGTH    14
#define INVALID_PTR     -1
// #define BUCKET_HDR(stream, param)  ((BucketHeader *) ##stream)->##param;

namespace Pumper {

    struct BucketHeader {
        int16_t count_entry_ptr;
        int16_t count_slice;
        int16_t first_free_slice;       
    };

    struct StringSlice {
        int8_t str_buf[SLICE_LENGTH];
        int16_t next;
    };

    struct EntryPtr {
        int16_t key_slice;
        int16_t value_slice;
    };
    
    class Bucket {
    public:
        Bucket();
        ~Bucket();

        // Allow import or export payload. Note that the pointer argument
        // can be NULL, which means that scratch from memory
        Status Import(int8_t * buffer);
        Status Export(int8_t * buffer);

        bool Put(const String &key, const String &value);
        Status Remove(const String &key);
        bool Exist(const String &key);
        bool Get(const String &key, String &value);
        std::vector<String> ListKeys();
        
        Status PrintDebugInfo();
        Status Defrag();
    private:
        // 4096-byte content, which is identical to disk.
        int8_t *payload;

        EntryPtr * get_entry_ptr(int16_t entry_index);
        int16_t insert_entry_ptr();
        void remove_entry_ptr(int16_t entry_index);

        String get_key(int16_t slice_offset);
        int16_t set_key(int16_t &slice_offset, const String &value);
        void remove_key(int16_t slice_offset);

    };
} // namespace Pumper

#endif // __BUCKET_H__

