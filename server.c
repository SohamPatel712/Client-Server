// Project Server

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

// custom flags
char getAction[] = "-1";
char putAction[] = "-2";
char ack[] = "-ack";

// custom messages
char exitMsg[] = "quit\n";
char wellcomeMsg[] = "Welcome to File Server!\n=========================================\nPlease try commands like\n > get <fileName> - to download,\n > put <filename> - to upload or\n > quit - to exit\n=========================================";
char errorMsg[] = "Invalid command, please try again!";
char foundMsg[] = "I have the file!";
char notFoundMsg[] = "I don't have the file!";
char downloadMsg[] = "Please wait till I download on your machine!";

void serviceClient(int);
void getFile(char *file, int sd);
void putFile(char *file, int sd);
void tokenize(char msg[]);
int getCommand(char msg[]);
int processRequest(char msg[], int sd);
bool getAck(int sd);

int main(int argc, char *argv[])
{
    int sd, client, portNumber, status;
    struct sockaddr_in socAddr;

    // if there are not enough arguements
    if (argc != 2)
    {
        printf("Synopsis: %s <Port Number>\n", argv[0]);
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
    socAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // on port no - portNumber
    sscanf(argv[1], "%d", &portNumber);
    socAddr.sin_port = htons((uint16_t)portNumber);

    // binding socket to socket addr
    bind(sd, (struct sockaddr *)&socAddr, sizeof(socAddr));

    // listening on that socket with max backlog of 5 connections
    listen(sd, 5);

    while (1)
    {
        // block process until a client appears on this socket
        client = accept(sd, NULL, NULL);

        // let the child process handle the rest
        if (!fork())
        {
            printf("Client/%d is connected!\n", getpid());
            serviceClient(client);
        }

        close(client);
    }
}

void serviceClient(int sd)
{
    char message[255];
    int n;

    write(sd, wellcomeMsg, sizeof(wellcomeMsg));

    while (1)
        // reads message from client
        if (n = read(sd, message, 255))
        {
            message[n] = '\0';
            printf("Client/%d: %s", getpid(), message);
            if (!strcasecmp(message, exitMsg))
            {
                printf("Client/%d is disconnected!\n", getpid());
                close(sd);
                exit(0);
            }

            processRequest(message, sd);

            // tokenize(message);
            // int commandId = getCommand(message);
            // if (commandId == 1)
            //     getFile(message);
            // else if (commandId == 2)
            //     putFile(message);
            // else
            //     write(sd, errorMsg, sizeof(errorMsg));
        }

    // pid = fork();
    // if (pid) /* reading client messages */
    //     while (1)
    //         if (n = read(sd, message, 255))
    //         {
    //             message[n] = '\0';
    //             fprintf(stderr, "Client: %s", message);
    //             if (!strcasecmp(message, exitMsg))
    //             {
    //                 printf("Bye Bye!!\n");
    //                 kill(pid, SIGTERM);
    //                 exit(0);
    //             }
    //         }
    // if (!pid) /* sending messages to the client  */
    //     while (1)
    //         if (n = read(0, message, 255))
    //         {
    //             message[n] = '\0';
    //             write(sd, message, strlen(message) + 1);
    //             if (!strcasecmp(message, exitMsg))
    //             {
    //                 printf("Bye Bye!!\n");
    //                 kill(getppid(), SIGTERM);
    //                 exit(0);
    //             }
    //         }
}

int processRequest(char msg[], int sd)
{
    char *command, *fileName;

    // coppying original msg to save it from mutating
    char tempMessage[255];
    strcpy(tempMessage, msg);

    // tokenizing every command with space - " " & '\n'
    int i = 0;
    char *tokens = strtok(tempMessage, " \n");
    while (tokens != NULL && i < 2)
    {
        if (i == 0)
            command = tokens;
        else
            fileName = tokens;

        i++;
        tokens = strtok(NULL, " \n");
    }

    // improper command arguement- no file name
    if (i != 2)
    {
        write(sd, errorMsg, sizeof(errorMsg));
        return 0;
    }

    // verify requested command
    if (strcasecmp(command, "get") == 0)
        getFile(fileName, sd);
    else if (strcasecmp(command, "put") == 0)
        putFile(fileName, sd);
    else
    {
        write(sd, errorMsg, sizeof(errorMsg));
        return 0;
    }
}

void tokenize(char msg[])
{
    printf("-------------------\n");
    printf("%s", msg);
    printf("str length: %d\n", strlen(msg));

    // coppying original msg to save it from mutating
    char tempMessage[255];
    strcpy(tempMessage, msg);

    char *command, *fileName;

    // tokenizing every command with space - " "
    int i = 0;
    char *tokens = strtok(tempMessage, " \n");
    while (tokens != NULL && i < 2)
    {
        if (i == 0)
            command = tokens;
        else
            fileName = tokens;

        i++;
        tokens = strtok(NULL, " \n");
    }
    printf("%d\n", i);

    printf("%s\n", command);
    printf("%s\n", fileName);

    if (i != 2)
        printf("no File\n");

    printf("-------------------\n");
}

int getCommand(char msg[])
{
    // coppying original msg to save it from mutating
    char tempMessage[255];
    strcpy(tempMessage, msg);

    char *command = strtok(tempMessage, " \n");
    if (strcasecmp(command, "get") == 0)
        return 1;
    else if (strcasecmp(command, "put") == 0)
        return 2;
    else
        return 0;
}

bool getAck(int sd)
{
    char ackMsg[10];
    int n;
    if (n = read(sd, ackMsg, sizeof(ackMsg)))
    {
        ackMsg[n] = '\0';
    }
    if (strcmp(ack, ackMsg) == 0)
        return true;
    else
        return false;
}

void getFile(char *file, int sd)
{
    printf("> getFile Called: %s\n", file);

    // defining required var
    DIR *dp;
    dp = opendir("./");
    struct dirent *dirp;
    bool isFound = false;

    // read all dirs
    while ((dirp = readdir(dp)) != NULL)
    {
        // find that file
        if (strcmp(dirp->d_name, file) == 0)
        {
            isFound = true;
            printf("> '%s' found\n", file);
            write(sd, foundMsg, sizeof(foundMsg));
            write(sd, downloadMsg, sizeof(downloadMsg));
            sleep(1);
            // after this, client will be in - donwloadFile() method
            write(sd, getAction, sizeof(getAction));
            printf("> downloadFile() in client called\n", file);
            sleep(1);
            // sending file name
            write(sd, dirp->d_name, strlen(dirp->d_name));
            printf("> fileName sent\n", file);
            sleep(1);

            int fd = open(file, O_RDONLY, S_IRUSR | S_IWUSR);

            long fileSize = lseek(fd, 0L, SEEK_END);

            // send filesize
            write(sd, &fileSize, sizeof(long));
            printf("> fileSize sent\n", file);
            sleep(1);

            // create buffer of fileSize
            char *buffer = malloc(fileSize);
            lseek(fd, 0L, SEEK_SET);
            read(fd, buffer, fileSize);
            write(sd, buffer, fileSize);

            free(buffer);
            close(fd);
            printf("> '%s' sent\n", file);
        }
    }
    if (!isFound)
    {
        printf("> '%s' not found\n", file);
        write(sd, notFoundMsg, sizeof(notFoundMsg));
        // write(sd, exitMsg, sizeof(exitMsg));
    }

    closedir(dp);
}

void putFile(char *file, int sd)
{
    printf("> putFile Called: %s\n", file);

    // after this, client will be in - uploadFile() method
    write(sd, putAction, sizeof(putAction));
    int n;
    char fileName[255];

    printf("> Uploading!!!\n");
    if (n = read(sd, fileName, 255))
    {
        fileName[n] = '\0';
    }
    printf("> fileName: %s\n", fileName);

    int fd = open(fileName, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);

    long fileSize;
    read(sd, &fileSize, sizeof(long));
    printf("> fileSize: %ld\n", fileSize);

    char *buffer = malloc(fileSize);
    read(sd, buffer, fileSize);
    write(fd, buffer, fileSize);

    free(buffer);
    close(fd);

    printf("> File successfully uploaded!!!\n");
}