// BTree.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// B+Tree Implementation. Will use a seperate index file for
// managing each key/value row's location and provide fast lookup
// property.

#include "BTree.h"

namespace Pumper
{

    BTree::BTree() : root(-1)
    {
        memset(image, 0, sizeof(image));
    }

    BTree::~BTree()
    {

    }

    void BTree::Insert(const String &key, int32_t page_id)
    {
        int32_t hash = get_hash(key);
        if (root < 0)
        {
            make_root_leaf(hash, page_id);
        }
        else
        {
            BTNode *leaf = find_leaf(hash);
            if (leaf->num_keys < N_ORDER - 1)
                insert_in_leaf(leaf, hash, page_id);
            else
                insert_in_leaf_splitted(leaf, hash, page_id);
        }
    }

    void BTree::Remove(const String &key)
    {
        int32_t hash = get_hash(key);
        BTNode * key_leaf = find_leaf(hash);;
        int left = 0;

        while (key_leaf->keys[left] != hash)
            left++;
        for (int i = left + 1; i < key_leaf->num_keys; i++)
            key_leaf->keys[i - 1] = key_leaf->keys[i];
        for (int i = left + 1; i < N_ORDER - 1; i++)
            key_leaf->pointers[i - 1] = key_leaf->pointers[i];
        
        key_leaf->num_keys--;
    }

    bool BTree::Search(const String &key, int32_t &page_id)
    {
        int32_t hash = get_hash(key);
        int32_t slot;

        if (root < 0)
            return false;

        BTNode *leaf = find_leaf(hash);
        
        // leaf should be leaf part now
        for (slot = 0; slot < leaf->num_keys; slot++)
        {
            if (hash == leaf->keys[slot])
            {
                page_id = leaf->pointers[slot];
                return true;
            }
        }

        return false;
    }

    void BTree::PrintDebugInfo()
    {
        for (int i = 0; i < 32; i++)
        {
            if (image[i].num_keys)
            {
                printf("*** PageNo = %d, IsLeaf = %d ***\n", i, image[i].is_leaf);
                for (int j = 0; j < image[i].num_keys; j++)
                {
                    printf("key = %d, pointer = %d\n", image[i].keys[j], image[i].pointers[j]);
                }
            }
        }
    }

    BTNode * BTree::find_leaf(int32_t hash)
    {
        BTNode *node = load_page(root);
        int32_t slot, next;

        while (node->is_leaf == 0)
        {
            // Still in internal nodes
            slot = 0;
            while (slot < node->num_keys && hash >= node->keys[slot])
                slot++;
            next = node->pointers[slot];
            node = load_page(next);;
        }

        return node;
    }

    // Using BKDR Hash
    int32_t BTree::get_hash(const String &key)
    {
        uint32_t seed = 131;
        uint32_t hash = 0;
        uint8_t *p = (uint8_t *) key.c_str();

        while (*p)
            hash = hash * seed + (*p++);
        return (int32_t) (hash % 1000000007);
    }

    void BTree::make_root_leaf(int hash, int page_id)
    {
        root = lease_page();
        BTNode *node = load_page(root);

        memset(node, 0, sizeof(BTNode));
        node->is_leaf = 1;
        node->id = root;
        node->parent = -1;
        node->prev = -1;
        node->next = -1;

        node->keys[0] = hash;
        node->pointers[0] = page_id;
        node->pointers[N_ORDER - 1] = -1;
        node->num_keys++;
    }

    void BTree::insert_in_leaf(BTNode * leaf, int hash, int page_id)
    {
        int insert_point = 0;
        while (insert_point < leaf->num_keys && leaf->keys[insert_point] < hash)
            insert_point++;

        for (int i = leaf->num_keys; i > insert_point; i--)
        {
            leaf->keys[i] = leaf->keys[i - 1];
            leaf->pointers[i] = leaf->pointers[i - 1];
        }

        leaf->keys[insert_point] = hash;
        leaf->pointers[insert_point] = page_id;
        leaf->num_keys++;
    }

    void BTree::insert_in_leaf_splitted(BTNode * leaf, int hash, int page_id)
    {
        int32_t new_leaf_id = lease_page();
        BTNode *new_leaf = load_page(new_leaf_id);

        memset(new_leaf, 0, sizeof(BTNode));
        new_leaf->is_leaf = 1;
        new_leaf->id = new_leaf_id;
        new_leaf->parent = -1;
        new_leaf->prev = -1;
        new_leaf->next = -1;

        int32_t temp_keys[N_ORDER];
        int32_t temp_pointers[N_ORDER];

        int insert_point = 0;
        while (insert_point < leaf->num_keys && leaf->keys[insert_point] < hash)
            insert_point++;

        int i, j;
        // j is used to skip the insert_point element.
        for (i = 0, j = 0; i < leaf->num_keys; i++, j++) 
        {
            if (j == insert_point) 
                j++;
            temp_keys[j] = leaf->keys[i];
            temp_pointers[j] = leaf->pointers[i];
        }

        temp_keys[insert_point] = hash;
        temp_pointers[insert_point] = page_id;

        leaf->num_keys = 0;

        int split = (N_ORDER + 1) / 2;

        for (i = 0; i < split; i++) 
        {
            leaf->pointers[i] = temp_pointers[i];
            leaf->keys[i] = temp_keys[i];
            leaf->num_keys++;
        }

        for (i = split, j = 0; i < N_ORDER; i++, j++) 
        {
            new_leaf->pointers[j] = temp_pointers[i];
            new_leaf->keys[j] = temp_keys[i];
            new_leaf->num_keys++;
        }
        
        new_leaf->pointers[N_ORDER - 1] = leaf->pointers[N_ORDER - 1];
        leaf->pointers[N_ORDER - 1] = new_leaf->id;
        new_leaf->parent = leaf->parent;

        int32_t new_key = new_leaf->keys[0];
        insert_into_parent(leaf, new_leaf, new_key);
    }

    void BTree::insert_into_parent(BTNode * left, BTNode * right, int hash)
    {    
        if (left->parent == -1)
            insert_into_new_root(left, right, hash);
        else
        {        
            BTNode * parent = load_page(left->parent);

            int left_index = 0;
            while (left_index <= parent->num_keys && parent->pointers[left_index] != left->id)
                left_index++;

            if (parent->num_keys < N_ORDER - 1)
                insert_node(parent, left_index, hash, right);
            else
                insert_node_split(parent, left_index, hash, right); 
        }
    }

    void BTree::insert_into_new_root(BTNode * left, BTNode * right, int32_t key)
    {
        root = lease_page();
        BTNode *node = load_page(root);

        memset(node, 0, sizeof(BTNode));
        node->id = root;
        node->parent = -1;
        node->prev = -1;
        node->next = -1;

        node->keys[0] = key;
        node->pointers[0] = left->id;
        node->pointers[1] = right->id;

        node->num_keys++;
        left->parent = root;
        right->parent = root;
    }

    void BTree::insert_node(BTNode * parent, int left_index, int32_t key, BTNode * right)
    {
        for (int i = parent->num_keys; i > left_index; i--)
        {
            parent->pointers[i + 1] = parent->pointers[i];
            parent->keys[i] = parent->keys[i - 1];
        }

        parent->pointers[left_index + 1] = right->id;
        parent->keys[left_index] = key;
        parent->num_keys++;
    }

    void BTree::insert_node_split(BTNode * old_node, int left_index, int32_t key, BTNode * right)
    {
        int32_t new_node_id = lease_page();
        BTNode *new_node = load_page(new_node_id);

        memset(new_node, 0, sizeof(BTNode));
        new_node->id = new_node_id;
        new_node->parent = -1;
        new_node->prev = -1;
        new_node->next = -1;

        int i, j;

        int32_t temp_keys[N_ORDER];
        int32_t temp_pointers[N_ORDER];

        for (i = 0, j = 0; i < old_node->num_keys + 1; i++, j++) 
        {
            if (j == left_index + 1) 
                j++;
            temp_pointers[j] = old_node->pointers[i];
        }

        for (i = 0, j = 0; i < old_node->num_keys; i++, j++) 
        {
            if (j == left_index) 
                j++;
            temp_keys[j] = old_node->keys[i];
        }

        temp_pointers[left_index + 1] = right->id;
        temp_keys[left_index] = key;
        
        int split = (N_ORDER + 1) / 2;
        old_node->num_keys = 0;

        for (i = 0; i < split - 1; i++) 
        {
            old_node->pointers[i] = temp_pointers[i];
            old_node->keys[i] = temp_keys[i];
            old_node->num_keys++;
        }

        old_node->pointers[i] = temp_pointers[i];

        int32_t new_key = temp_keys[split - 1];

        for (++i, j = 0; i < N_ORDER; i++, j++) 
        {
            new_node->pointers[j] = temp_pointers[i];
            new_node->keys[j] = temp_keys[i];
            new_node->num_keys++;
        }

        new_node->pointers[j] = temp_pointers[i];
        new_node->parent = old_node->parent;

        for (i = 0; i <= new_node->num_keys; i++) 
        {
            BTNode * child = load_page(new_node->pointers[i]);
            child->parent = new_node->id;
        }

        insert_into_parent(old_node, new_node, new_key);
    }

} // namespace Pumper
