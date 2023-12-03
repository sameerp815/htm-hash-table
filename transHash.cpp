#include <cstdlib>
#include <iostream>
#include <vector>
#include <immintrin.h> 
#include <pthread.h>
#include <getopt.h>





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


void insert(HashTable* table, int key, int value);

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



void reSize(HashTable* table, int newSize) {
    HashTable* newTable = createTable(newSize);

    // Rehash the items from the old table to the new table
    for (int i = 0; i < table->size; i++) {
        HashItem* item = table->items[i];
        while (item != NULL) {
            // Insert into the new table
            insert(newTable, item->key, item->value);
            item = item->next;
        }
    }

    // Free the old table's items
    for (int i = 0; i < table->size; i++) {
        HashItem* item = table->items[i];
        while (item != NULL) {
            HashItem* temp = item;
            item = item->next;
            free(temp);
        }
    }

    // Free the old table's array and update the table's properties
    free(table->items);
    table->items = newTable->items;
    table->size = newTable->size;
    table->count = newTable->count;

    // Free the new table structure only (not its items and array)
    free(newTable);
}




void insert(HashTable* table, int key, int value) {

    // Check load factor
    float loadFactor = (float)table->count / table->size;
    if (loadFactor > 0.7) {
        // Resize the table, e.g., double the size
        reSize(table, table->size * 2);
    }


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


// //INSERTING WITH TRANSACTIONAL MEMORY
// void insertTrans(HashTable *table, int key, int value) {
//     bool transaction_success = false;
//     unsigned test = _XBEGIN_STARTED;

    
//     std::cout << test << std::endl;

//     //NOTE: im not implementing rollback, because it should just work right, bounded waiting or some crap like that? UPDATE: WE MIGHT HAVE TO IMPLEMENT ROLLBACK
//     //TODO: The primary problem right now is that the _xbegin should return 429496729, but it returns 0
//     //https://github.com/andikleen/tsx-tools/blob/master/include/rtm.h -> if you look at this link, the possible return values are stated, 0 isn't one of them, no clue why this is happening

//     while (!transaction_success) {
//         unsigned status = _xbegin();
        
//         if (status == _XBEGIN_STARTED) {
//             //transactional region -> this is where you do the insert
//             transaction_success = true;
//              // this commits the transaction
//              _xend();
//         } else {
//             //transaction was aborted -> another thread is prolly doin shit, we should just retry, that seems like the best option
//             // std::cout << "ABORTING" << std::endl;
            
//         }
//     }
    
// }

void insertTrans(HashTable *table, int key, int value) {


    // Check load factor
    float loadFactor = (float)table->count / table->size;
    if (loadFactor > 0.7) {
        // Resize the table, - >  double the size
        reSize(table, table->size * 2);
    }
    std::cout << "resize works" << std::endl;


    int index = hashFunction(key, table->size);
    HashItem* item = (HashItem*)malloc(sizeof(HashItem));
    item->key = key;
    item->value = value;
    item->next = NULL;

    bool success = false;

    __transaction_atomic {
        if (table->items[index] == NULL) {
        table->items[index] = item;
        } else {
            item->next = table->items[index];
            table->items[index] = item;
        }
        table->count++;
        success = true;
    } 

    std::cout << "KINDA WORKED" << std::endl;

    if(!success) {
        //we need to fallback
        std::cout << "FALLBACK" << std::endl;
    }


    
}

HashTable* myTable = nullptr;

void *tester_func(void *args) {
    //do something
    insertTrans(myTable, 1, 100);
}

static int size = 0;
static int workers = 0;





int main(int argc, char **argv) {

    int option;


    static struct option long_options[] = {
        {"size",    required_argument, 0, 's'},
        {"workers",     required_argument, 0, 'w'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    
    

    //READ IN OPTIONS
    while((option = getopt_long(argc, argv, "s:w:", long_options, &option_index)) != -1) {
        switch (option) {
            case 's':
                size = atoi(optarg);
                std::cout << size << std::endl;
                break;
            case 'w':
                workers = atoi(optarg);
                std::cout << workers << std::endl;
                break;
            default:
                printf("Invalid option or missing argument\n");
                break;
        }
    }



    myTable = createTable(100);

    int worker = 100;

    pthread_t tid[worker];


    for(int i = 0; i < worker; i++) {
        int err = pthread_create(&tid[i], NULL, tester_func, nullptr);
    }

    for(int j = 0; j < worker; j++) {
        int err = pthread_join(tid[j], nullptr);
    }

    std::vector<int> foundValues;
    searchAll(myTable, 1, foundValues);
    for (int value : foundValues) {
        std::cout << value << std::endl;
    }

    std::cout << "Size: " << myTable->size << std::endl;
    
    deleteItem(myTable, 2);
    deleteItem(myTable, 2);
    deleteItem(myTable, 2);
    std::cout << "Value for key 2 after deletion: " << search(myTable, 2) << std::endl;
    std::cout << "Value for key 3 checking for insertTrans: " << search(myTable, 3) << std::endl;

    freeTable(myTable);
    return 0;
}