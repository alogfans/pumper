// BTree.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// B+Tree Implementation. Will use a seperate index file for
// managing each key/value row's location and provide fast lookup
// property.

#ifndef __BTREE_H__
#define __BTREE_H__

#include "Types.h"
#include "Status.h"
#include "Lock.h"

#define N_ORDER ((PAGE_SIZE - 24) / 8)

namespace Pumper
{
    struct BTNode
    {
        int32_t is_leaf;
        int32_t id;
        int32_t parent, prev, next;
        int32_t num_keys;
        int32_t keys[N_ORDER - 1];
        int32_t pointers[N_ORDER];
    };

    class BTree
    {
    public:
        BTree();
        ~BTree();

        void Insert(const String &key, int32_t page_id);
        void Remove(const String &key);
        bool Search(const String &key, int32_t &page_id);

        void PrintDebugInfo();

    private:
        int32_t get_hash(const String &key);
        BTNode * find_leaf(int32_t hash);

        void make_root_leaf(int hash, int page_id);
        void insert_in_leaf(BTNode * leaf, int hash, int page_id);
        void insert_in_leaf_splitted(BTNode * leaf, int hash, int page_id);
        void insert_into_parent(BTNode * left, BTNode * right, int hash);
        void insert_into_new_root(BTNode * left, BTNode * right, int32_t key);
        void insert_node(BTNode * parent, int left_index, int32_t key, BTNode * right);
        void insert_node_split(BTNode * old_node, int left_index, int32_t key, BTNode * right);

        // Test only
        BTNode * load_page(int32_t id)
        {
            if (id >= 0 && id < alloc)
                return &image[id];

            return NULL;
        }

        //void unload_page(BTNode * bt_node)
        //{
            // do nothing
        //}

        int32_t lease_page()
        {
            int32_t id = alloc;
            alloc++;
            return id;
        }

        int     alloc = 0;
        int     root;
        BTNode  image[32];
    };
} // namespace Pumper

#endif // __BTREE_H__

