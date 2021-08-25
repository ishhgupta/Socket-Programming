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

#define IP_ADDRESS "127.0.0.1" // localhost
#define PORT 8000
#define BUFFER_SIZE 10


void receivefile(char filename[1024],int sockfd)
{
    int fd = open(filename,O_WRONLY | O_CREAT | O_TRUNC,0664);
    if(fd < 0){
        perror("ERROR");
        return;
    }
    long int filelength = 0;
    int valread = read(sockfd,&filelength,sizeof(long int));
    printf("filelength is %ld\n",filelength);
    float progress = 0;
    int length_written = 0;
    // while (1)
    // {
    //     if(filelength == 0) break;
    //     char buffer[BUFFER_SIZE] = {0};
    //     int val = read(sockfd,buffer,sizeof(buffer));
    //     printf("buffer is with size %d and buffer is %s\n",val,buffer);
    //     int btr = write(fd,buffer,val);
    //     length_written += btr; 
    //     //printing progress
    //     progress = ((float)length_written/filelength)*100;
    //     printf("Downloading... "); printf("%0.2f percent\r",progress);
        
    //     if(length_written == filelength){break;}
    //     if(btr<=0) {printf("Read error\n"); break;}
    //     bzero (buffer,BUFFER_SIZE);
    // }
    while(1)
    {
        char buffer[BUFFER_SIZE] = {0};
        int val = read(sockfd,buffer,sizeof(buffer));
        printf("buffer is with size %d and buffer is %s\n",val,buffer);
        int btr = write(fd,buffer,val);
        length_written += btr;
        progress = ((float)length_written/filelength)*100;
        printf("Downloading... "); printf("%0.2fpercent\r",progress);

        if(length_written == filelength){break;}
        if(btr<=0){printf("Read error\n"); break;}
        bzero(buffer,BUFFER_SIZE);

    }
    printf("\x1B[1;34mFile Received.\n\x1B[0m");
    close(fd);
}
int main()
{
    //creating client socket
    int client_socket = socket(AF_INET,SOCK_STREAM,0);
    if(client_socket < 0){
        perror("Problem in creating the socket");
        exit(2);
    }

    //specifying an address for the socket i.e. server socket where client wants to connect
    struct sockaddr_in server_addr;
    memset(&server_addr, '0', sizeof(server_addr)); // to make sure the struct is empty. Essentially sets sin_zero as 0
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Converts an IP address in numbers-and-dots notation into either a 
    // struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
    if(inet_pton(AF_INET, IP_ADDRESS, &server_addr.sin_addr)<=0)
    {
        perror("\nInvalid address.\n");
        return -1;
    }

    //connect to the server address
    if(connect(client_socket,(struct sockaddr*)&server_addr,sizeof(server_addr))<0)
    {
        perror("Problem in connecting to the server");
        exit(2);
    }
    char *connection = "Connected with client.\n";
    send (client_socket,connection,strlen(connection),0);
    char conn[1024] = {0};
    int val=read(client_socket,conn,1024);
    printf("%s\n",conn);

    while(1){
        //TAKING INPUT 
        printf("\x1B[0;33mclient>\x1B[0m");
        size_t bytes_read,size = 0;
        char *read_comm = NULL;
        read_comm = (char *)malloc(size*sizeof(char));
        bytes_read = getline(&read_comm,&size,stdin); 

        //splitting the strings in the input given
        char filenames[100][1024]; //present given input of command and filenames
        char* token = strtok(read_comm," \t\n");
        int num_files=0;
        while (token != NULL)
        {
            strcpy(filenames[num_files],token);
            num_files++;
            token = strtok(NULL," \t\n");
        } 
        // printf("num of arguments %d\n",num_files);
        char files[100][1024];
        for(int i=0;i<num_files;i++)
        {
            strcpy(files[i],filenames[i]);
        }
        //Executing and downloading the files
        if(strcmp("get",files[0])==0){
            for(int i=1;i<num_files;i++)        //number of files is one less than num_files coz it includes command name also
            {
                send(client_socket,files[i],strlen(files[i]),0);
                char ifpresent[1024] = {0};
                val = read(client_socket,ifpresent,sizeof(ifpresent));
                if (strcmp(ifpresent,"no")==0){
                    printf("\x1B[0;31mFile '%s' not found on server.\n\x1B[0m",filenames[i]);
                    // continue;
                }
                else if(strcmp(ifpresent,"yes")==0){
                    printf("\x1B[0;32mFile '%s' found on server.\n\x1B[0m",filenames[i]);
                    receivefile(files[i],client_socket);
                }
                
            }
        }
        else if(strcmp("exit",files[0])==0){
            close(client_socket);
            printf("Connection with server is closed.\n");
            exit(0);
        }
        else{
            printf("\x1B[0;31mPlease enter a valid command. :)\n\x1B[0m");
        }      
    }    
    return 0;
}