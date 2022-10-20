#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define RECEIVE_BUF_SIZE        1024
#define server_addr             "10.0.2.10"
#define EXAMPLE_PORT            3425

void print(const char *msg)
{
    printf("%s\n", msg);
}

int main(void)
{
    print("Server starting");
    /* Create socket for connection with server. */
    int clientSocketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (-1 == clientSocketFD)
    {
        perror("cannot create socket");
        return EXIT_FAILURE;
    }
    
    print("Socket init");
    /* Creating and initialisation of socket`s address structure for connection with server. */
    struct sockaddr_in stSockAddr;
    memset(&stSockAddr, 0, sizeof(stSockAddr));
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(EXAMPLE_PORT);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    print("Getting address info");
    struct addrinfo *resultHints = NULL;
    int res = getaddrinfo(server_addr, NULL, &hints, &resultHints);
    print("Connecting to %s:%s", server_addr, EXAMPLE_PORT);
    if (res != 0)
    {
        perror("cannot get address info");
        close(clientSocketFD);
        return EXIT_FAILURE;
    }

    struct sockaddr_in* in_addr = (struct sockaddr_in *)resultHints->ai_addr;
    memcpy(&stSockAddr.sin_addr.s_addr, &in_addr->sin_addr.s_addr, sizeof(in_addr_t));
    freeaddrinfo(resultHints);

    print("Connecting");
    res = -1;
    for (int i = 0; res == -1 && i < 10; i++)
    {
        sleep(1); // Wait some time for server be ready.
        res = connect(clientSocketFD, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr));
        print("Retrying...");
    }

    if (res == -1)
    {
        perror("connect failed");
        close(clientSocketFD);
        return EXIT_FAILURE;
    }

    print("Sending data");
    char buf[] = "perform read write operations ...\n";
    /* Send data to server. */
    if (-1 == send(clientSocketFD,buf,strlen(buf) + 1,0))
    {
        perror("send failed");
        close(clientSocketFD);
        return EXIT_FAILURE;
    }
    
    print("Shutting down");
    /* Close connection with server. */
    shutdown(clientSocketFD, SHUT_RDWR);
    close(clientSocketFD);

    return EXIT_SUCCESS;
}
