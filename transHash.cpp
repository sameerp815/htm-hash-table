#include <cstdlib>
#include <iostream>
#include <vector>
#include <immintrin.h> 



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


//INSERTING WITH TRANSACTIONAL MEMORY
void insertTrans(HashTable *table, int key, int value) {
    bool transaction_success = false;
    unsigned test = _XBEGIN_STARTED;

    
    std::cout << test << std::endl;

    //NOTE: im not implementing rollback, because it should just work right, bounded waiting or some crap like that? UPDATE: WE MIGHT HAVE TO IMPLEMENT ROLLBACK
    //TODO: The primary problem right now is that the _xbegin should return 429496729, but it returns 0
    //https://github.com/andikleen/tsx-tools/blob/master/include/rtm.h -> if you look at this link, the possible return values are stated, 0 isn't one of them, no clue why this is happening

    while (!transaction_success) {
        unsigned status = _xbegin();
        
        if (status == _XBEGIN_STARTED) {
            //transactional region -> this is where you do the insert
            transaction_success = true;
             // this commits the transaction
             _xend();
        } else {
            //transaction was aborted -> another thread is prolly doin shit, we should just retry, that seems like the best option
            // std::cout << "ABORTING" << std::endl;
            
        }
    }
    
}






int main() {
    HashTable* myTable = createTable(10);

    insert(myTable, 1, 100);
    insert(myTable, 2, 200);
    insert(myTable, 2, 300);
    insert(myTable, 2, 400);
    insertTrans(myTable, 3, 300);

    std::vector<int> foundValues;
    searchAll(myTable, 2, foundValues);
    for (int value : foundValues) {
        std::cout << value << std::endl;
    }

    deleteItem(myTable, 2);
    deleteItem(myTable, 2);
    deleteItem(myTable, 2);
    std::cout << "Value for key 2 after deletion: " << search(myTable, 2) << std::endl;
    std::cout << "Value for key 3 checking for insertTrans: " << search(myTable, 2) << std::endl;

    freeTable(myTable);
    return 0;
}