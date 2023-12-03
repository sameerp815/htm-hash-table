#include <cstdlib>
#include <iostream>
#include <vector>
#include <immintrin.h>
#include <pthread.h>
#include <getopt.h>
#include <type_traits>

#define DEFAULT_SIZE 50

template <typename Key, typename Value>
class HashTable
{
    typedef struct HashItem
    {
        Key key;
        Value value;
        struct HashItem *next;
    } HashItem;

public:
    HashTable(size_t originalCapacity) : capacity(originalCapacity), size(0)
    {
        items = (HashItem **)calloc(capacity, sizeof(HashItem *));
    }
    HashTable() : HashTable(DEFAULT_SIZE)
    {
    }

    size_t getSize()
    {
        return size;
    }

    void insert(Key key, Value value)
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

    Value search(Key key)
    {
        size_t index = hashFunction(key, capacity);
        HashItem *item = items[index];

        while (item != NULL)
        {
            if (item->key == key)
            {
                return item->value;
            }
            item = item->next;
        }
        return -1;
    }

    bool deleteItem(Key key)
    {
        int index = hashFunction(key, capacity);
        HashItem *item = items[index];
        HashItem *prev = NULL;

        while (item != NULL)
        {
            if (item->key == key)
            {
                HashItem *old = item;
                item = item->next;
                if (prev == NULL)
                {
                    items[index] = item;
                }
                else
                {
                    prev->next = item;
                }
                free(old);
                size--;
                return true;
            }
            prev = item;
            item = item->next;
        }
        return false;
    }

private:
    std::hash<Key> hashMethod;
    size_t hashFunction(Key key, size_t arraySize)
    {
        return hashMethod(key) % arraySize;
    }

    void searchAll(Key key, std::vector<int> &values)
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

    bool insertItem(HashItem **tableItems, Key key, Value value, size_t tableSize)
    {
        // not - > using chaining to handle collisions
        int index = hashFunction(key, tableSize);
        HashItem *item = (HashItem *)malloc(sizeof(HashItem));
        item->key = key;
        item->value = value;
        item->next = NULL;

        if (tableItems[index] == NULL)
        {
            tableItems[index] = item;
        }
        else
        {
            HashItem *curr = tableItems[index];
            while (curr != NULL)
            {
                if (curr->key == item->key)
                {
                    return false;
                }
                curr = curr->next;
            }
            item->next = tableItems[index];
            tableItems[index] = item;
        }
        return true;
    }

    void reSize(int newCapacity)
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