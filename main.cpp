#include <cstdlib>
#include <iostream>
#include <vector>
#include <immintrin.h>
#include <pthread.h>
#include <getopt.h>
#include "HashTable.cpp"

static int size = 0;
static int workers = 0;

int main(int argc, char **argv)
{
    HashTable<std::string, int> map;

    map.insert("hello", 100);
    std::cout << map.search("hello") << std::endl;
    map.deleteItem("hello");
    std::cout << map.search("hello") << std::endl;
}