#pragma once
#include <string>
#include <mutex>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <iostream>


using namespace std;

#define PUSH 1
#define POP 2
#define TOP 3

#define BACKLOG 10   // how many pending connections queue will hold
#define SIZE 1024

typedef struct free_block {
    size_t size;
    struct free_block* next;
} free_block;


// void* my_malloc(size_t size);

// void my_free(void* ptr);

typedef struct node
{
    char txt[1024];
    struct node* next;
    struct node* prev;

    node(char t[1024]){
        strcmp(txt,t);
        next = NULL;
        prev = NULL;
    }
}node , *pnode;

class Stack{

    private:
    pnode stack;
    int stack_size;
    int fd;

    public:
    char* address;
    
    Stack();

    ~Stack();
    
    void* my_malloc();

    void my_free();

    // void* my_caloc();

    bool push(const char t[1024]);

    string pop();

    string top();

};