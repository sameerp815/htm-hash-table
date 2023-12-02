#include <cstdlib>
#include <iostream>
#include <vector>



typedef struct HashItem {
    int key;
    int value;
    struct HashItem* next;
} HashItem;



typedef struct {
    HashItem** items;
    int size;
    int count;
} HashTable;

int hashFunction(int key, int size) {
    return key % size;
}

HashTable* createTable(int size) {
    HashTable* table = (HashTable*)malloc(sizeof(HashTable));
    table->size = size;
    table->count = 0;
    table->items = (HashItem**)calloc(table->size, sizeof(HashItem*));
    return table;
}

void insert(HashTable* table, int key, int value) {
    //not - > using chaining to handle collisions
    int index = hashFunction(key, table->size);
    HashItem* item = (HashItem*)malloc(sizeof(HashItem));
    item->key = key;
    item->value = value;
    item->next = NULL;

    if (table->items[index] == NULL) {
        table->items[index] = item;
    } else {
        item->next = table->items[index];
        table->items[index] = item;
    }
    table->count++;
}

int search(HashTable* table, int key) {
    int index = hashFunction(key, table->size);
    HashItem* item = table->items[index];

    while (item != NULL) {
        if (item->key == key) {
            return item->value;
        }
        item = item->next;
    }
    return -1;
}

void searchAll(HashTable* table, int key, std::vector<int>& values) {
    int index = hashFunction(key, table->size);
    HashItem* item = table->items[index];
    values.clear();

    while (item != NULL) {
        if (item->key == key) {
            values.push_back(item->value);
        }
        item = item->next;
    }
}

void deleteItem(HashTable* table, int key) {
    int index = hashFunction(key, table->size);
    HashItem* item = table->items[index];
    HashItem* prev = NULL;

    while (item != NULL) {
        if (item->key == key) {
            if (prev == NULL) {
                table->items[index] = item->next;
            } else {
                prev->next = item->next;
            }
            free(item);
            table->count--;
            return;
        }
        prev = item;
        item = item->next;
    }
}

void freeTable(HashTable* table) {
    for (int i = 0; i < table->size; i++) {
        HashItem* item = table->items[i];
        while (item != NULL) {
            HashItem* temp = item;
            item = item->next;
            free(temp);
        }
    }
    free(table->items);
    free(table);
}

int main() {
    HashTable* myTable = createTable(10);

    insert(myTable, 1, 100);
    insert(myTable, 2, 200);
    insert(myTable, 2, 300);
    insert(myTable, 2, 400);
    insert(myTable, 3, 300);

    std::vector<int> foundValues;
    searchAll(myTable, 2, foundValues);
    for (int value : foundValues) {
        std::cout << value << std::endl;
    }

    deleteItem(myTable, 2);
    deleteItem(myTable, 2);
    deleteItem(myTable, 2);
    std::cout << "Value for key 2 after deletion: " << search(myTable, 2) << std::endl;

    freeTable(myTable);
    return 0;
}