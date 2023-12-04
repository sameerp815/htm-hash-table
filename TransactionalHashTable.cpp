#include <cstdlib>
#include <iostream>
#include <vector>
#include <immintrin.h>
#include <pthread.h>
#include <getopt.h>
#include <type_traits>

#define DEFAULT_SIZE 50

template <typename Value>
class TransactionalHashTable
{
    typedef struct HashItem
    {
        int key;
        Value value;
        struct HashItem *next;
    } HashItem;

public:
    TransactionalHashTable(size_t originalCapacity) : capacity(originalCapacity), size(0)
    {
        items = (HashItem **)calloc(capacity, sizeof(HashItem *));
    }
    TransactionalHashTable() : TransactionalHashTable(DEFAULT_SIZE)
    {
    }

    size_t getSize() __transaction_atomic
    {
        return size;
    }

    void insert(int key, Value value) __transaction_atomic
    {

        // Check load factor
        float loadFactor = (float)size / capacity;
        if (loadFactor > 0.7)
        {
            // Resize the table, e.g., double the size
            reSize(capacity * 2);
        }

        if (insertItem(items, key, value, capacity))
        {
            size++;
        }
    }

    bool containsKey(int key) __transaction_relaxed
    {
        int index = hashFunction(key, capacity);
        HashItem *item = items[index];

        while (item != NULL)
        {
            if (item->key == key)
            {
                return true;
            }
            item = item->next;
        }
        return false;
    }

    Value get(int key) __transaction_relaxed
    {
        int index = hashFunction(key, capacity);
        HashItem *item = items[index];

        while (item != NULL)
        {
            if (item->key == key)
            {
                return item->value;
            }
            item = item->next;
        }
        return item->value;
    }

    bool deleteItem(int key) __transaction_atomic
    {
        HashItem *old = nullptr;
        int index = hashFunction(key, capacity);
        HashItem *item = items[index];
        HashItem *prev = NULL;

        while (item != NULL)
        {
            if (item->key == key)
            {
                old = item;
                item = item->next;
                if (prev == NULL)
                {
                    items[index] = item;
                }
                else
                {
                    prev->next = item;
                }
                size--;
                break;
            }
            prev = item;
            item = item->next;
        }

        if (old == nullptr)
        {
            return false;
        }
        free(old);
        return true;
    }

private:
    size_t hashFunction(int key, size_t arraySize) __transaction_atomic
    {
        return key % arraySize;
    }

    void searchAll(int key, std::vector<int> &values) __transaction_atomic
    {
        int index = hashFunction(key, size);
        HashItem *item = items[index];
        values.clear();

        while (item != NULL)
        {
            if (item->key == key)
            {
                values.push_back(item->value);
            }
            item = item->next;
        }
    }

    [[transaction_safe]] bool insertItem(HashItem **tableItems, int key, Value value, size_t tableSize)
    {
        // not - > using chaining to handle collisions
        int index = hashFunction(key, tableSize);
        HashItem *item = (HashItem *)malloc(sizeof(HashItem));
        item->key = key;
        item->next = NULL;

        bool completedInsert = false;
        __transaction_atomic
        {
            if (tableItems == nullptr)
            {
                __transaction_cancel;
            }
            if (tableItems[index] == NULL)
            {
                completedInsert = true;
                tableItems[index] = item;
            }
            else
            {
                HashItem *curr = tableItems[index];
                while (curr != NULL)
                {
                    if (curr->key == item->key)
                    {
                        __transaction_cancel;
                    }
                    curr = curr->next;
                }
                item->next = tableItems[index];
                item->value = value;
                tableItems[index] = item;
                completedInsert = true;
            }
        }

        if (!completedInsert)
        {
            free(item);
        }

        return completedInsert;
    }

    void reSize(int newCapacity) __transaction_atomic
    {

        HashItem **resizedItems = (HashItem **)calloc(newCapacity, sizeof(HashItem *));
        // Rehash the items from the old table to the new table
        for (int i = 0; i < capacity; i++)
        {
            HashItem *item = items[i];
            while (item != NULL)
            {
                // Insert into the new table
                insertItem(resizedItems, item->key, item->value, newCapacity);
                item = item->next;
            }
        }

        // Free the old table's items
        for (int i = 0; i < capacity; i++)
        {
            HashItem *item = items[i];
            while (item != NULL)
            {
                HashItem *temp = item;
                item = item->next;
                free(temp);
            }
        }

        // Free the old table's array and update the table's properties
        free(items);
        // TODO: Need transaction?
        items = resizedItems;
    }

    size_t capacity;
    size_t size;
    HashItem **items;
};