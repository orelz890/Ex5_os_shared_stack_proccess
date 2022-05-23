// We were aided by this site in this part:
// https://stackoverflow.com/questions/5422061/malloc-implementation

#include "stack.hpp"
#include <iostream>
#include <fcntl.h>

static free_block free_block_list_head = { 0, 0 };
static const size_t overhead = sizeof(size_t);
static const size_t align_to = 16;
struct flock mutex_lock; // we will use this to synchronize the operation of the processes

Stack::Stack(){
    stack_size = 0;
    stack = NULL;
    this->address = NULL;
    this->fd = open("mutex.txt", O_WRONLY | O_CREAT);
    if (fd == -1) //The file didn't opened successfuly
    {
        printf("Error");
    }
    memset(&mutex_lock, 0, sizeof(mutex_lock));
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
    mutex_lock.l_type = F_WRLCK;    //write lock
    fcntl(fd, F_SETLKW, &mutex_lock);
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
    mutex_lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &mutex_lock);
    return 0;
}

string Stack::pop(){
    mutex_lock.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &mutex_lock);
    if (stack_size <= 0 || this->stack == NULL){
    mutex_lock.l_type = F_UNLCK;
    fcntl (fd, F_SETLKW, &mutex_lock);
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
    mutex_lock.l_type = F_UNLCK;
    fcntl (fd, F_SETLKW, &mutex_lock);
    return ans;
}

string Stack::top(){
    
    mutex_lock.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &mutex_lock);
    if (stack_size <= 0 || this->stack == NULL){
        mutex_lock.l_type = F_UNLCK;
        fcntl (fd, F_SETLKW, &mutex_lock);
        return "Non stack is empty";
    }
    mutex_lock.l_type = F_UNLCK;
    fcntl (fd, F_SETLKW, &mutex_lock);
    return this->stack->txt;
}
