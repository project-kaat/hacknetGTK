#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <poll.h>
#include <fcntl.h>
#include <ctype.h>

/*-=-=-=CONSTANTS=-=-=-*/
#define PATH_BUF_SIZE           1024
#define COM_BUF_SIZE            1024
#define POLLING_INTERVAL        0.5
const char * DISPLAY_SOCKET_PATH  = "/tmp/multix.socket";

/*-=-=-=VARIABLES=-=-=-*/
int socketDescriptor = -1;
extern char **environ;

/*-=-=-=FUNCTIONS=-=-=-*/
bool handleCommand(char * command) {

    char * com = command;
    char * args;
    for (int i = 0; i<strlen(command);i++) {
        
        if (isspace(command[i])) {
            command[i] = '\0';
            args = command + i + 1;
            break;
        }
    }

    //main command handling
    if (strcmp(com, "CWD") == 0) {

        char * path = args;

        chdir(path);
        if (getenv("PWD")) {
            setenv("PWD", path, 1);
        }

    }

    else if (strcmp(com, "RUN") == 0) {

        char * command = args;

        system(command);
    }

    else {
        return false;
    }

    return true;

}

bool sendCommand(const int dst, const char * command, size_t n) {

    if (send(dst, command, n, 0) == -1) {
        return false;
    }
    else {
        return true;
    }
}

bool checkIncomingCommands(const struct pollfd * pollPtr) {

    if (poll(pollPtr, 1, 0) > 0) {
        return true;
    }
    else {
        return false;
    }
}

bool getCurrentCommand(const int src, char * dstBuffer, size_t n) {

    memset(dstBuffer, 0, COM_BUF_SIZE);

    int ret = recv(src, dstBuffer, n, 0);

    if (ret > 0) {
        return true;
    }
    else {
        return false;
    }

}

//main thread of the multix server
//all added functionality runs from here
void serverMain(void) {

    printf("Initializing the multix server\n");

    /*Initialize some internal vars to track state*/
    char lastPath[PATH_BUF_SIZE] = {0};
    char currentPath[PATH_BUF_SIZE] = {0};
    if(getcwd(lastPath, PATH_BUF_SIZE) == NULL) {
        fprintf(stderr, "Can't get cwd\n");
        exit(1);
    }
    char commandBuffer[COM_BUF_SIZE] = {0};

    /*Create a Unix Domain socket descriptor*/

    socketDescriptor = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketDescriptor == -1) {
        fprintf(stderr, "Can't acquire a socket\n");
        exit(1);
    }

    /*Initialize remote address*/

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, DISPLAY_SOCKET_PATH, sizeof(addr.sun_path) - 1);
    int addrLen = strlen(addr.sun_path) + sizeof(addr.sun_family);

    /*Connect to DISPLAY server via Unix Domain sockets*/

    if (connect(socketDescriptor, (struct sockaddr*)&addr, addrLen) == -1) {
        fprintf(stderr, "Could not connect to DISPLAY server socket\n");
        exit(1);
    }

    /*Create a pollfd struct for polling the socket input*/
    const struct pollfd socketPollFd = {
        .fd = socketDescriptor,
        .events = POLLIN
    };

    useconds_t realPollingInterval = POLLING_INTERVAL * 1000000; //convert seconds to microseconds in whole numbers

    //main server event loop
    //only handle events here
    while (1) {

        if (checkIncomingCommands(&socketPollFd)) {

            if (getCurrentCommand(socketDescriptor, commandBuffer, COM_BUF_SIZE) != true) {
                fprintf(stderr, "Failed to get remote command\n");
                exit(1);
            }
            if (handleCommand(commandBuffer) != true) {
                fprintf(stderr, "Failed to handle command: %s\n", commandBuffer);
            }

        }

        //update cwd; check for changes
        memset(currentPath, 0, PATH_BUF_SIZE);
        if (getcwd(currentPath, PATH_BUF_SIZE) == NULL) {
            fprintf(stderr, "Can't get cwd\n");
            exit(1);
        }

        if (strcmp(currentPath, lastPath) != 0) {

            char outgoingCom[COM_BUF_SIZE] = {0};
            snprintf(outgoingCom, COM_BUF_SIZE, "CWD %s", currentPath);
            
            //cwd changed; notify display
            sendCommand(socketDescriptor, outgoingCom, COM_BUF_SIZE);
            //update lastcwd
            memset(lastPath, 0, PATH_BUF_SIZE);
            strncpy(lastPath, currentPath, PATH_BUF_SIZE);

        }

        usleep(realPollingInterval); 

    }

}

//constructor function. will run before main
__attribute__((constructor))
void initialize (void) {

    //our library should only be injected into the shell process
    //shells are assumed to use and possibly override environment variable behaviour
    //therefore, we manually terminate LD_PRELOAD variable
    for (int i = 0;environ[i];i++) {

        if (strstr(environ[i], "LD_PRELOAD")) {
            printf("Modifying environ directly to get rid of LD_PRELOAD\n");
            environ[i][0] = '\0';
        }

    }

    //create a thread for the multix server
    //the whole process lives in its own thread and only sometimes interferes with global process resources
    pthread_t server;
    pthread_create(&server, NULL, serverMain, NULL);

}

//destructor function. will run after main
__attribute__((destructor))
void cleanup (void) {

    //close the socket descriptor
    printf("Unloading the multix server\n");
    close(socketDescriptor);
    system("reset");

}
