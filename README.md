## SOCKET PROGRAMMING
This is a simple client-server socket program in which files are downloaded to client directory from server using socket programming.

### RUNNING INSTRUCTIONS
1. Firstly compile the program using command ``` gcc server.c -o server ``` in server directory. 
2. Run the server program using ``` ./server ```.Server should be run before client. 
3. Run ``` gcc client.c -o client ``` to compile the program in client directory.
4. Run the program using ``` ./client ``` .

Files to be downloaded by client should be present in server directory.
Files are overwritten in the case they are previously present in client directory.

#### Commands
1. ``` get <filename> ``` is used in the running program of client to download file from server.
2. ``` exit ``` is used to close the connection from server.

