#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <platform/platform.h>
#include <kos_net.h>

#include <gpio/gpio.h>
#include <stdbool.h>

#if defined(__arm__) || defined(__aarch64__)
#include <bsp/bsp.h>
#endif


// extra headers from the net2 example
#include <netdb.h>
#include <stdlib.h>
#include <sys/stat.h>


// should be global defines
#define prINF "INF"
#define prDBG "DBG"
#define prINI "INIT"
#define prERR "ERROR"
#define prWRN "WRN"

#define opOK "OK\n"
#define opFAIL "FAILED\n"


// functions declarations
void workGpio(GpioHandle& handle)
{
    GpioOut(handle, 6, 1);
    GpioOut(handle, 26, 1);

    GpioOut(handle, 12, 1);
    GpioOut(handle, 21, 1);
    GpioOut(handle, 13, 0);
    GpioOut(handle, 20, 0);
    
    sleep(2);
    GpioOut(handle, 6, 0);
    GpioOut(handle, 26, 0);
}

void print(const char* msg, const char* prefix = nullptr)
{
    if(prefix) printf("[%s] %s\n", prefix, msg);
    else printf("%s", msg);
}

int main() 
{
    print("\n\n\n\n\n");
    print("System has started...", prINI);
    
    int sock, listener;
    struct sockaddr_in addr;
    char buf[1024];
    int bytes_read;

    print("Initializing components", prINI);
#if PLATFORM_OS(KOS)
#define server_addr "10.0.2.2"
    int num_tries = 5;
    print("[INIT] Staring network interface...");
    
    for(int i = 0; i < num_tries; i++)
    {
        if (!configure_net_iface(DEFAULT_INTERFACE, "10.0.2.10", DEFAULT_MASK, DEFAULT_GATEWAY, DEFAULT_MTU))
        {
            
            print("Network interface configuration failed... Retrying in 5 seconds...", prWRN);
            sleep(5);
            
            if(i >= num_tries - 1)
            {
                print(opFAIL);
                print("Network interface configuration failed", prERR);
                print("Out of tries. Terminating.", prERR);
                return EXIT_FAILURE;
            }
        }
    }

    if (!list_network_ifaces()) 
    {
        print("Listing of host network interfaces failes", prERR);
        return EXIT_FAILURE;
    }
    
    print(opOK);
#else
#define server_addr "localhost"
    print("Network is raised on localhost", prWRN);
#endif

    print("[INIT] Starting GPIO...");
#if defined(__arm__) || defined(__aarch64__)
    if (BspInit(NULL) != BSP_EOK)
    {
        print(opFAIL);
        print("Failed to initialize BSP", prERR);
        return EXIT_FAILURE;
    }

    if (BspSetConfig("gpio0", "raspberry_pi4b.default") != BSP_EOK)
    {
        print(opFAIL);
        print("Failed to set mux configuration for gpio0 module", prERR);
        return EXIT_FAILURE;
    }
#endif

    if (GpioInit())
    {
        print("GpioInit failed", prERR);
        return EXIT_FAILURE;
    }
    
    print(opOK);
    print("[INIT] Initializing GPIO ports...");
    
    GpioHandle handle = NULL;
	if (GpioOpenPort("gpio0", &handle) || handle == GPIO_INVALID_HANDLE)
	{
	    print(opFAIL);
	    print("GpioOpenPort failed", prERR);
	    return EXIT_FAILURE;
	}
    
    GpioSetMode(handle, 6, GPIO_DIR_OUT);
    GpioSetMode(handle, 26, GPIO_DIR_OUT);

    GpioSetMode(handle, 12, GPIO_DIR_OUT);
    GpioSetMode(handle, 13, GPIO_DIR_OUT);
    GpioSetMode(handle, 20, GPIO_DIR_OUT);
    GpioSetMode(handle, 21, GPIO_DIR_OUT);
    
    print(opOK);
    
    print("Initialization successful", prINI);
    
    print("Starting debug sequence", prINF);

    print("Connecting...", prINF);
    listener = socket(AF_INET, SOCK_STREAM, 0);

    if(listener < 0) 
    {
        print("Listener creation error", prERR);
        return EXIT_FAILURE;
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3425);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    print("Binding...", prINF);
    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) 
    {
        print("Binding error", prERR);
        return EXIT_FAILURE;
    }
    
    print("Awaiting for incoming connection", prINF);
    listen(listener, 1);
    sock = accept(listener, NULL, NULL);
    if(sock < 0)
    {
        print("Error while accepting connection", prERR);
        return EXIT_FAILURE;
    }
    print("Connected!", prINF);
    
    print("Listening for the messages...", prINF);
    while(1)
    {
        bytes_read = recv(sock, buf, 1024, 0);
        if(bytes_read <= 0)
            continue;
            
        printf("Received message: \'%s\'\n", buf);
        workGpio(handle);
        if(buf == "0") break;
    }
    
    print("Shutting down...", prINF);
    close(sock);
    close(listener);
    print("Bye!", prINF);
}
