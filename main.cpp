#include <cstdlib>
#include <iostream>
#include <vector>
#include <immintrin.h>
#include <pthread.h>
#include <getopt.h>
#include "TransactionalHashTable.cpp"

static int size = 0;
static int workers = 0;

int main(int argc, char **argv)
{
    TransactionalHashTable<float> map;

    map.insert(100, 17.7);
    std::cout << map.containsKey(100) << std::endl;
}

// VALUE must be trivially copyable. This is because if, say, a string is copied, the state of the string is volatile and may not be able to be rolled back. specially for template types
// Downsides of transactions: difficult to implement when using library functions because they also have to be transaction safe, can end up going down the rabbit hole of basically making fine-grained transactions that are pretty similar to locks
// difficult to decide when to relax transaction (isolation) vs strengthen (atomic)
// Upside: no deadlock, composable
