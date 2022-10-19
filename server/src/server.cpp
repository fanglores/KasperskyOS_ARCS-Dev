#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include<arpa/inet.h>

//#include <tracing.h> 

void print(const char* msg)
{
    printf("%s", msg);
}

void println(const char* msg)
{
    printf("%s\n", msg);
}

int main()
{
    int sock;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3425);
    addr.sin_addr.s_addr = inet_addr("10.0.2.10");

    print("Opening socket...");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        println("FAILED");
        perror("");
        return 1;
    }
    println("OK");  

    print("Connecting...");
    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) 
    {
        println("FAILED");
        perror("");
        return 1;
    }
    println("OK");

    char* msg;
    for(int i = 0; i < 5; i++)
    {
        *msg = char('0' + i);
        send(sock, msg, sizeof(msg), 0);
    }
    
    close(sock);
    return 0;
}
