#include <cstdlib>
#include <iostream>
#include <vector>
#include <immintrin.h>
#include <pthread.h>
#include <getopt.h>
#include "HashTable.cpp"
#include <chrono>

pthread_mutex_t mutex;

static int size = 0;
static int workers = 0;
static int per = 0;

HashTable<int> map;

void *tester_func(void *args) {
    int id = *((int*)args); 
    //do something

    for(int i = 0; i < per; i++) {
        pthread_mutex_lock(&mutex);
        int uniq = id * 1000000 + i;
        map.insert(uniq, 30);
        pthread_mutex_unlock(&mutex);
    }
}


int main(int argc, char **argv)
{

    pthread_mutex_init(&mutex, nullptr);


    int option;


    static struct option long_options[] = {
        {"size",    required_argument, 0, 's'},
        {"workers",     required_argument, 0, 'w'},
        {"per",     required_argument, 0, 'p'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    


    //READ IN OPTIONS
    while((option = getopt_long(argc, argv, "s:w:p:", long_options, &option_index)) != -1) {
        switch (option) {
            case 's':
                size = atoi(optarg);
                break;
            case 'w':
                workers = atoi(optarg);
                break;
            case 'p':
                per = atoi(optarg);
                break;
            default:
                printf("Invalid option or missing argument\n");
                break;
        }
    }


    pthread_t tid[workers];
    int elements[workers];


    auto start = std::chrono::high_resolution_clock::now();



    for(int i = 0; i < workers; i++) {
        elements[i] = i;
        int err = pthread_create(&tid[i], NULL, tester_func, &elements[i]);
    }

    for(int j = 0; j < workers; j++) {
        int err = pthread_join(tid[j], nullptr);
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    //prints out time in ms

    std::cout << duration.count() << std::endl;

    
}

// VALUE must be trivially copyable. This is because if, say, a string is copied, the state of the string is volatile and may not be able to be rolled back. specially for template types
// Downsides of transactions: difficult to implement when using library functions because they also have to be transaction safe, can end up going down the rabbit hole of basically making fine-grained transactions that are pretty similar to locks
// difficult to decide when to relax transaction (isolation) vs strengthen (atomic)
// Upside: no deadlock, composable
