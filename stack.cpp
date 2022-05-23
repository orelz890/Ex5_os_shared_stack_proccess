// We were aided by this site in this part:
// https://stackoverflow.com/questions/5422061/malloc-implementation

#include "stack.hpp"
#include <iostream>
#include <fcntl.h>

static free_block free_block_list_head = { 0, 0 };
static const size_t overhead = sizeof(size_t);
static const size_t align_to = 16;
struct flock locker; // we will use this to synchronize the operation of the processes

Stack::Stack(){
    stack_size = 0;
    stack = NULL;
    this->address = NULL;
    this->fd = open("locker.txt", O_WRONLY | O_CREAT);
    if (fd == -1) //The file didn't opened successfuly
    {
        printf("Error");
    }
    memset(&locker, 0, sizeof(locker));
}

Stack::~Stack(){
    cout << "\nfinito\n";
    fflush(stdout);
    while (stack != NULL)
    {
        pop();
    }
}

void* Stack::my_malloc() {
    this->address = this->address + sizeof(node);
    return this->address;
}

void Stack::my_free() {
    this->address = this->address - sizeof(node);
}

bool Stack::push(const char t[1024]){
    locker.l_type = F_WRLCK;    //write lock
    fcntl(fd, F_SETLKW, &locker);
    pnode new_space = (pnode)this->my_malloc();
    memset(new_space,0,sizeof(new_space));

    new_space->next = NULL;
    new_space->prev = NULL;
    this->stack_size++;
    strcpy(new_space->txt, t);
    new_space->next = this->stack;

    if (this->stack != NULL || this->stack_size <= 0)
    {
        this->stack->prev = new_space;
    }
    this->stack = new_space;
    locker.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &locker);
    return 0;
}

string Stack::pop(){
    locker.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &locker);
    if (stack_size <= 0 || this->stack == NULL){
    locker.l_type = F_UNLCK;
    fcntl (fd, F_SETLKW, &locker);
        return "Non stack is empty";
    }
    
    string ans = this->stack->txt;
    pnode temp = this->stack;
    this->stack = this->stack->next;
    if (this->stack != NULL)
    {
        this->stack->prev = NULL;
    }
    this->my_free();
    this->stack_size--;
    locker.l_type = F_UNLCK;
    fcntl (fd, F_SETLKW, &locker);
    return ans;
}

string Stack::top(){
    
    locker.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &locker);
    if (stack_size <= 0 || this->stack == NULL){
        locker.l_type = F_UNLCK;
        fcntl (fd, F_SETLKW, &locker);
        return "Non stack is empty";
    }
    locker.l_type = F_UNLCK;
    fcntl (fd, F_SETLKW, &locker);
    return this->stack->txt;
}