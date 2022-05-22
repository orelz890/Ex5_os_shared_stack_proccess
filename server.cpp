/*
** server.c -- a stream socket server demo
*/

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

#define PORT "6060"  // the port users will be connecting to

#define BACKLOG 10   // how many pending connections queue will hold
#define MAXDATASIZE 1024 // max number of bytes we can get at once 
int sockfd;  // listen on sock_fd, new connection on new_fd
Stack* my_stack;

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
            // send(new_fd,poped.c_str(),poped.length(),0);
            response.append("Element poped: " + data);
            cout << response << '\n';
        }
        else if (strncmp(txt_buf, "TOP", 3) == 0){
            data.clear();
            response.clear();
            data = my_stack->top();
            // send(new_fd,top.c_str(),top.length(),0);
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
    int fd = my_stack->open_new_file();
    my_stack = (Stack*)mmap(NULL, sizeof(Stack), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
            handle_commend(&new_fd);
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }

    return 0;
}
