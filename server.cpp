#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/mman.h>

#include "stack.hpp"

#define PORT "6060"
// how many pending connections queue will hold
#define BACKLOG 10
// max number of bytes we can get at once
#define MAXDATASIZE 1024 
#define MEGABYTES 1048576

// listen on sock_fd, new connection on new_fd
int sockfd;

class Stack* my_stack;

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void *handle_commend(void *newfd) {
    int byteslen;
    int new_fd = *(int*)newfd;  
    string msg = "You are a stack maneger now!\n";
    if (send(new_fd, msg.c_str() , msg.length(), 0) == -1)
    {
        perror("send");
        exit(1);
    }

    char txt_buf[MAXDATASIZE];
    string data;
    string response;
    bool flag;

    while (1)
    {
        flag = false;
        memset(txt_buf,0,sizeof(txt_buf));
        byteslen = recv(new_fd,txt_buf,sizeof(txt_buf),0);
        if (byteslen == -1)
        {
            perror("Recv txt error\n");
            break;
        }

        if (strncmp(txt_buf, "POP", 3) == 0){
            data.clear();
            response.clear();
            data = my_stack->pop();
            response.append("Element poped: " + data);
            cout << response << '\n';
        }
        else if (strncmp(txt_buf, "TOP", 3) == 0){
            data.clear();
            response.clear();
            data = my_stack->top();
            response.append("OUTPUT: Last string in stack is: " + data);
            cout << response << '\n';
        }
        else if (strncmp(txt_buf, "PUSH", 4) == 0)
        {
            data.clear();
            response.clear();
            data.append(txt_buf);
            my_stack->push(data.substr(5).c_str());
            response.append("Data pushed: " + data.substr(5));
            cout << response << '\n';
        }
        else if (strncmp(txt_buf, "EXIT", 4) == 0)
        {
            cout << "Client " << (new_fd - 3) << " disconnected!" << "\n";
            close(new_fd);
            break;

        }
        else{
            response.clear();
            response.append("ERROR: This commend is not suported! please read the instuction..");
        }
        if (!response.empty())
        {
            int num = send(new_fd , response.c_str(), response.length(), 0);
            if (num == -1)
            {
                perror("Send eror\n");
            }
            response.clear();
        }
        
    }
    return 0;
}

int main(void)
{
    unsigned long num_of_nodes_shared = 10*MEGABYTES / sizeof(struct node);
    my_stack = (class Stack*)mmap(NULL, sizeof(class Stack), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    my_stack->address = (char*)mmap(NULL, sizeof(struct node)*num_of_nodes_shared, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0); 
    // listen on sock_fd, new connection on new_fd
    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *p;
    // connector's address information
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    // Use my IP
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("ERROR: server socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("ERROR: setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("ERROR: server bind");
            continue;
        }

        break;
    }
    // All done with this structure
    freeaddrinfo(servinfo);

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("ERROR: listen");
        exit(1);
    }
    // reap all dead processes
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");
    
    // main accept() loop
    while(1) {
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("ERROR: accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        // Child process
        if (!fork()) {
            close(sockfd);
            handle_commend(&new_fd);
            close(new_fd);
            exit(0);
        }
        close(new_fd);
    }

    return 0;
}
