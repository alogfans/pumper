// BTree.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// B+Tree Implementation. Will use a seperate index file for
// managing each key/value row's location and provide fast lookup
// property.

#include "BTree.h"
#include "PageHandle.h"

namespace Pumper
{

    BTree::BTree(PagedFile &pf) : pf(pf)
    {
        ERROR_ASSERT(pf.IsFileOpened());         
        pf.GetRootPage(root);
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
            unload_page(leaf);
        }
    }

    void BTree::Remove(const String &key)
    {
        int32_t hash = get_hash(key);
        BTNode * key_leaf = find_leaf(hash);
        delete_entry(key_leaf, hash);
        unload_page(key_leaf);
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
                unload_page(leaf);
                return true;
            }
        }
        unload_page(leaf);
        return false;
    }

    void BTree::PrintDebugInfo()
    {
        /*
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
        */
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
            unload_page(node);
            node = load_page(next);
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
        pf.SetRootPage(root);
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

        unload_page(node);
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

        unload_page(new_leaf);
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

            unload_page(parent);
        }
    }

    void BTree::insert_into_new_root(BTNode * left, BTNode * right, int32_t key)
    {
        root = lease_page();
        pf.SetRootPage(root);
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

        unload_page(node);
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
            unload_page(child);
        }

        insert_into_parent(old_node, new_node, new_key);
        unload_page(new_node);
    }

    void BTree::delete_entry(BTNode * node, int hash)
    {
        int left = 0;
        int num_pointers = node->is_leaf ? node->num_keys : node->num_keys + 1;

        while (node->keys[left] != hash)
            left++;
        for (int i = left + 1; i < node->num_keys; i++)
            node->keys[i - 1] = node->keys[i];
        for (int i = left + 1; i < num_pointers; i++)
            node->pointers[i - 1] = node->pointers[i];
        
        node->num_keys--;

        /*
        if (node->id == root && node->num_keys == 0) 
            adjust_root();
        else
        {
            int min_keys = node->is_leaf ? (N_ORDER - 1) / 2 : N_ORDER / 2 - 1;
            int capacity = node->is_leaf ? N_ORDER : N_ORDER - 1;

            if (node->num_keys >= min_keys)
                return;

            BTNode *parent_node = load_page(node->parent);

            int neighbor_index;
            for (int i = 0; i <= parent_node->num_keys; i++)
            {
                if (parent_node->pointers[i] == node->id)
                {
                    neighbor_index = i - 1;
                    break;
                }
            }

            int k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
            int k_prime = parent_node->keys[k_prime_index];
            BTNode *neighbor = load_page(neighbor_index == -1 ? 
                parent_node->pointers[1] : parent_node->pointers[neighbor_index]);

            if (neighbor->num_keys + node->num_keys < capacity) {
                int node_id = node->id;
                coalesce_nodes(node, neighbor, neighbor_index, k_prime);
                recycle_page(node_id);
            }
            else
                redistribute_nodes(node, neighbor, neighbor_index, k_prime_index, k_prime);

            unload_page(parent_node);
            unload_page(neighbor);
        }
        */
    }

    void BTree::adjust_root() 
    {
        int32_t orig_root = root;
        BTNode *root_node = load_page(root);

        if (!root_node->is_leaf) {
            root = root_node->pointers[0];
            pf.SetRootPage(root);
            unload_page(root_node);
            root_node = load_page(root);
            root_node->parent = -1;            
        }
        else
        {
            root = -1;
            pf.SetRootPage(root);
        }
        unload_page(root_node);
        recycle_page(orig_root);
    }

    void BTree::coalesce_nodes(BTNode * node, BTNode * neighbor, int neighbor_index, int k_prime) 
    {
        int i, j, neighbor_insertion_index, node_end;

        if (neighbor_index == -1) 
        {
            // swap node and neighbor
            // swap_btnode(node, neighbor);
            BTNode dummy;
            memcpy(&dummy, node, sizeof(BTNode));
            memcpy(node, neighbor, sizeof(BTNode));
            memcpy(neighbor, node, sizeof(BTNode));
        }

        neighbor_insertion_index = neighbor->num_keys;

        if (!node->is_leaf) 
        {
            neighbor->keys[neighbor_insertion_index] = k_prime;
            neighbor->num_keys++;

            node_end = node->num_keys;

            for (i = neighbor_insertion_index + 1, j = 0; j < node_end; i++, j++) 
            {
                neighbor->keys[i] = node->keys[j];
                neighbor->pointers[i] = node->pointers[j];
                neighbor->num_keys++;
                node->num_keys--;
            }

            neighbor->pointers[i] = node->pointers[j];

            for (i = 0; i < neighbor->num_keys + 1; i++) 
            {
                BTNode * tmp = load_page(neighbor->pointers[i]);
                tmp->parent = neighbor->id;
                unload_page(tmp);
            }
        }
        else 
        {
            for (i = neighbor_insertion_index, j = 0; j < node->num_keys; i++, j++) 
            {
                neighbor->keys[i] = node->keys[j];
                neighbor->pointers[i] = node->pointers[j];
                neighbor->num_keys++;
            }
            neighbor->pointers[N_ORDER - 1] = node->pointers[N_ORDER - 1];
        }

        BTNode * parent_node = load_page(node->parent);
        delete_entry(parent_node, k_prime);
        unload_page(parent_node);
    }

    void BTree::redistribute_nodes(BTNode * node, BTNode * neighbor, int neighbor_index, 
        int k_prime_index, int k_prime) 
    {
        int i;
        BTNode * parent_node;

        if (neighbor_index != -1) {
            if (!node->is_leaf)
                node->pointers[node->num_keys + 1] = node->pointers[node->num_keys];

            for (i = node->num_keys; i > 0; i--) 
            {
                node->keys[i] = node->keys[i - 1];
                node->pointers[i] = node->pointers[i - 1];
            }

            if (!node->is_leaf) 
            {
                node->pointers[0] = neighbor->pointers[neighbor->num_keys];

                BTNode * tmp = load_page(neighbor->pointers[0]);
                tmp->parent = node->id;
                unload_page(tmp);

                neighbor->pointers[neighbor->num_keys] = -1;
                node->keys[0] = k_prime;

                parent_node = load_page(node->parent);
                parent_node->keys[k_prime_index] = neighbor->keys[neighbor->num_keys - 1];
                unload_page(parent_node);
            }
            else 
            {
                node->pointers[0] = neighbor->pointers[neighbor->num_keys - 1];
                neighbor->pointers[neighbor->num_keys - 1] = -1;
                node->keys[0] = neighbor->keys[neighbor->num_keys - 1];

                parent_node = load_page(node->parent);
                parent_node->keys[k_prime_index] = node->keys[0];
                unload_page(parent_node);
            }
        }
        else 
        {  
            if (node->is_leaf) 
            {
                node->keys[node->num_keys] = neighbor->keys[0];
                node->pointers[node->num_keys] = neighbor->pointers[0];

                parent_node = load_page(node->parent);
                parent_node->keys[k_prime_index] = neighbor->keys[1];
                unload_page(parent_node);
            }
            else 
            {
                node->keys[node->num_keys] = k_prime;
                node->pointers[node->num_keys + 1] = neighbor->pointers[0];

                BTNode * tmp = load_page(node->pointers[node->num_keys + 1]);
                tmp->parent = node->id;
                unload_page(tmp);

                parent_node = load_page(node->parent);
                parent_node->keys[k_prime_index] = neighbor->keys[0];
                unload_page(parent_node);
            }

            for (i = 0; i < neighbor->num_keys - 1; i++) 
            {
                neighbor->keys[i] = neighbor->keys[i + 1];
                neighbor->pointers[i] = neighbor->pointers[i + 1];
            }

            if (!node->is_leaf)
                neighbor->pointers[i] = neighbor->pointers[i + 1];
        }

        node->num_keys++;
        neighbor->num_keys--;

    }

    BTNode * BTree::load_page(int32_t id)
    {
        BTNode * bt_node = new BTNode();
        PageHandle ph;
        ph.OpenPage(pf, id);
        ph.Read((char *) bt_node, sizeof(BTNode));
        ph.ClosePage();
        return bt_node;
    }

    void BTree::unload_page(BTNode * bt_node)
    {
        PageHandle ph;
        ph.OpenPage(pf, bt_node->id);
        ph.Write((char *) bt_node, sizeof(BTNode));
        ph.ClosePage();
        delete bt_node;
    }

    int32_t BTree::lease_page()
    {
        int32_t id;
        pf.AllocatePage(id);
        return id;
    }

    void BTree::recycle_page(int32_t id)
    {
        pf.ReleasePage(id);
    }

} // namespace Pumper
