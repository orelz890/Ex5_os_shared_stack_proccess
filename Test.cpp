#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <string.h>
#include <iostream>
#include <unistd.h>

using namespace std;

#define PORT 6060
char buf[1024];
char rbuf[1024];

void send_msg(int socket, string txt){
    memset(buf,0,sizeof(buf));
    strcpy(buf,txt.c_str());
    send(socket, buf , sizeof(buf), 0);
}

string check_the_response(int socket, char expected[1024]){
    memset(rbuf,0,sizeof(rbuf));
    int rv = recv(socket, rbuf, sizeof(rbuf), 0);
    if (rv == -1)
    {
        perror("Sec recv prob!");
    }
    cout << "expected: " << expected;
    cout << "\nGot: " << rbuf << '\n';
    fflush(stdout);
    assert((strcmp(expected,rbuf) == 0));
}

int main()
{
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(PORT);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("Socket failed\n");
        exit(1);
    }
    
    if (connect(sock,(struct sockaddr*)&server_address, sizeof(server_address))!= 0)
    {
        perror("Connect failed");
        exit(1);
    }
    memset(rbuf,0,sizeof(rbuf));
    int rv = recv(sock, rbuf, sizeof(rbuf), 0);
    if (rv == -1)
    {
        perror("First recv prob!");
    }
    
    cout << "Begin: " << rbuf << '\n';
    cout << "We are connected! starting the tests..\n";
    
// ================================
    // Basic stack operation check
// ================================

    send_msg(sock, "PUSH Hey");
    check_the_response(sock, "Data pushed: Hey");
    send_msg(sock, "TOP");
    check_the_response(sock, "OUTPUT: Last string in stack is: Hey");
    send_msg(sock, "POP");
    check_the_response(sock, "Element poped: Hey");

// ===================
    // Special cases:
// ===================

    // Multiple pushes
    send_msg(sock, "PUSH I");
    check_the_response(sock, "Data pushed: I");
    send_msg(sock, "PUSH am");
    check_the_response(sock, "Data pushed: am");
    send_msg(sock, "PUSH working on");
    check_the_response(sock, "Data pushed: working on");
    send_msg(sock, "PUSH this");
    check_the_response(sock, "Data pushed: this");
    send_msg(sock, "PUSH assignment");
    check_the_response(sock, "Data pushed: assignment");
    send_msg(sock, "PUSH far");
    check_the_response(sock, "Data pushed: far");
    send_msg(sock, "PUSH too long");
    check_the_response(sock, "Data pushed: too long");
    cout << "\n\n";

    // Multiple pops
    send_msg(sock, "POP");
    check_the_response(sock, "Element poped: too long");
    send_msg(sock, "POP");
    check_the_response(sock, "Element poped: far");
    send_msg(sock, "POP");
    check_the_response(sock, "Element poped: assignment");
    send_msg(sock, "POP");
    check_the_response(sock, "Element poped: this");
    send_msg(sock, "POP");
    check_the_response(sock, "Element poped: working on");
    send_msg(sock, "POP");
    check_the_response(sock, "Element poped: am");
    send_msg(sock, "POP");
    check_the_response(sock, "Element poped: I");
    cout << "\n\n";

    // POP/TOP from empty stack
    send_msg(sock, "POP");
    check_the_response(sock, "Element poped: Non stack is empty");
    send_msg(sock, "TOP");
    check_the_response(sock, "OUTPUT: Last string in stack is: Non stack is empty");
    cout << "\n\n";

    // Invalid commends opper/lower case
    send_msg(sock, "BLABLA");
    check_the_response(sock, "ERROR: This commend is not suported! please read the instuction..");
    send_msg(sock, "JHJHJ");
    check_the_response(sock, "ERROR: This commend is not suported! please read the instuction..");
    send_msg(sock, "blabla");
    check_the_response(sock, "ERROR: This commend is not suported! please read the instuction..");
    send_msg(sock, "jhjhj");
    check_the_response(sock, "ERROR: This commend is not suported! please read the instuction..");
    send_msg(sock, "push aaa");
    check_the_response(sock, "ERROR: This commend is not suported! please read the instuction..");
    send_msg(sock, "push AAA");
    check_the_response(sock, "ERROR: This commend is not suported! please read the instuction..");
    send_msg(sock, "top");
    check_the_response(sock, "ERROR: This commend is not suported! please read the instuction..");
    send_msg(sock, "pop");
    check_the_response(sock, "ERROR: This commend is not suported! please read the instuction..");

    // If we press enter by mistake
    send_msg(sock, "");
    check_the_response(sock, "ERROR: This commend is not suported! please read the instuction..");

    // After this the client will disconnect but the server is still open!!
    send_msg(sock, "EXIT");

    sleep(1);
    close(sock);
    // Let's make sure we can still reconnect:
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("Socket failed\n");
        exit(1);
    }
    
    if (connect(sock,(struct sockaddr*)&server_address, sizeof(server_address))!= 0)
    {
        perror("Connect failed");
        exit(1);
    }
    memset(rbuf,0,sizeof(rbuf));
    rv = recv(sock, rbuf, sizeof(rbuf), 0);
    if (rv == -1)
    {
        perror("First recv prob!");
    }
    
    cout << "Begin: " << rbuf << '\n';
    cout << "We are connected! starting the tests..\n";

    send_msg(sock, "PUSH Im back!!");
    check_the_response(sock, "Data pushed: Im back!!");

    send_msg(sock, "EXIT");

    cout << "\nThe test was a success!!\n";


}
