// Project Client

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <fcntl.h>

// global vars
char getAction[] = "-1";
char putAction[] = "-2";
char ackMsg[] = "-ack";
char exitMsg[] = "quit\n";

void donwloadFile(int sd);
void uploadFile(int sd);

int main(int argc, char *argv[])
{
    char message[255];
    int sd, portNumber, pid, n;
    struct sockaddr_in socAddr;

    // if there are not enough arguements
    if (argc != 3)
    {
        printf("Call model: %s <IP Address> <Port Number>\n", argv[0]);
        exit(0);
    }

    // creating socket descriptor
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "Cannot create socket\n");
        exit(1);
    }

    // creating socket address
    socAddr.sin_family = AF_INET;
    // on port no - portNumber
    sscanf(argv[2], "%d", &portNumber);
    socAddr.sin_port = htons((uint16_t)portNumber);

    // converting addr from presentation to network format
    if (inet_pton(AF_INET, argv[1], &socAddr.sin_addr) < 0)
    {
        fprintf(stderr, " inet_pton() has failed\n");
        exit(2);
    }

    // connect to the server socket
    if (connect(sd, (struct sockaddr *)&socAddr, sizeof(socAddr)) < 0)
    {
        fprintf(stderr, "connect() has failed, exiting\n");
        exit(3);
    }

    read(sd, message, 255);
    printf("Message from Server: %s\n", message);

    pid = fork();

    if (pid)
        while (1)
            // reads message from server
            if (n = read(sd, message, 255))
            {
                message[n] = '\0';
                if (strcmp(message, getAction) == 0)
                    donwloadFile(sd);
                else if (strcmp(message, putAction) == 0)
                    uploadFile(sd);
                else
                    fprintf(stderr, "Server: %s\n", message);
                // if (!strcasecmp(message, exitMsg))
                // {
                //     printf("Bye Bye!!\n");
                //     close(sd);
                //     kill(pid, SIGTERM);
                //     exit(0);
                // }
            }

    if (!pid)
        // writes message to server
        while (1)
            if (n = read(0, message, 255))
            {
                message[n] = '\0';
                write(sd, message, strlen(message) + 1);
                if (!strcasecmp(message, exitMsg))
                {
                    printf("Bye Bye!!\n");
                    close(sd);
                    kill(getppid(), SIGTERM);
                    exit(0);
                }
            }
}

void donwloadFile(int sd)
{
    int n;
    char fileName[255];

    printf("> Downloading!!!\n");
    if (n = read(sd, fileName, 255))
    {
        fileName[n] = '\0';
    }
    printf("> fileName: %s\n", fileName);

    int fd = open("test.txt", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);

    long fileSize;
    read(sd, &fileSize, sizeof(long));
    printf("> fileSize: %ld\n", fileSize);

    char *buffer = malloc(fileSize);
    read(sd, buffer, fileSize);
    write(fd, buffer, fileSize);

    free(buffer);
    close(fd);

    printf("> File download complete!!!\n");
}
void uploadFile(int sd)
{
    printf("> Uploading!!!\n");
}