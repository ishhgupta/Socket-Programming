#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PORT 8000
#define MAX_CONNECTIONS 3
#define BUFFER_SIZE 10

void sendfile(char filename[1024], int *fileDes, int conn_fd)
{
    // printf("in sendfile\n");
    int fd = *fileDes,valread;
    long int  filelength = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    long int  length_to_read = filelength;
    printf("file length is %ld\n", filelength);
    valread = send(conn_fd,&filelength,sizeof(long int),0);
    float progress = 0;
    while (1)
    {
        char buffer[BUFFER_SIZE] = {0};
        if (length_to_read > BUFFER_SIZE)
        {
            valread = read(fd, buffer, BUFFER_SIZE);
            send(conn_fd, buffer, BUFFER_SIZE, 0);
            printf("buffer size is %d and buffer is %s\n", valread,buffer);
            length_to_read = length_to_read - BUFFER_SIZE;
        }
        else if (length_to_read <= BUFFER_SIZE)
        {
            valread = read(fd, buffer, length_to_read);
            send(conn_fd, buffer, length_to_read, 0);
            printf("buffer size is %d and buffer is %s\n", valread,buffer);
            break;
        }
        progress = ((float)(filelength - length_to_read)/filelength)*100;
        printf("Sending...  "); printf("%0.2f percent\r",progress);
    }
    printf("File sent.\n\n");
    close(fd);
    return;
}

int main()
{
    //creating server socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Problem in creating the socket");
        exit(EXIT_FAILURE);
    }
    // This is to lose the pesky "Address already in use" error message
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) // SOL_SOCKET is the socket layer itself
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_socket, MAX_CONNECTIONS) < 0)
    {
        perror("Listen error");
        exit(EXIT_FAILURE);
    }

    int conn_fd, addrlen = sizeof(server_address);
    if ((conn_fd = accept(server_socket, (struct sockaddr *)&server_address, (socklen_t *)&addrlen)) < 0)
    {
        perror("Accept error");
        exit(EXIT_FAILURE);
    }
    char conn[1024] = {0};
    int valread = read(conn_fd, conn, 1024);
    printf("%s\n", conn);
    char *connection = "Connected with server.\n";
    send(conn_fd, connection, strlen(connection), 0);

    while (1)
    {
        char filename[1024] = {0};
        valread = read(conn_fd, filename, sizeof(filename));
        // printf("Checking if file '%s' is available....\n",filename);
        int fd = open(filename, O_RDONLY);
        if (fd < 0)
        {
            // perror("Error");
            send(conn_fd, "no", sizeof("no"), 0);
            // continue;
        }
        else
        {
            printf("Required file is available.\n");
            send(conn_fd, "yes", sizeof("yes"), 0);
            sendfile(filename,&fd,conn_fd);
        }
    }
    return 0;
}