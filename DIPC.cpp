/*
    C socket server example
*/

/*
    example code from
    http://www.binarytides.com/server-client-example-c-sockets-linux/
*/
 
#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread

//global handlers
bool allocateGlobals(char *argv[]);
void deallocateGlobals();

bool checkArgc(argc);
 

char** mailBoxes_global;
int*  mutexLocks_global;

//the thread function
void *connection_handler(void *);


int main(int argc , char *argv[])
{
    //check argc value
    if(checkArgc(argc))
    {
        cout << "Usage: DIPC <number of mailboxes><size of mailbox in kbytes><port> <size of packet in kbytes>" << endl;
        return -1;
    }
    else //allocate mems
    {
        if( !allocateGlobals(char *argv[]) )
        {
            cerr << "failed to allocate memory, exiting" << endl;
            return -2;   
        }
    }
    
    
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");
         
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;
         
        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
        puts("Handler assigned");
    }
     
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
     
    return 0;
}
 
/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[2000];
     
    //Send some messages to the client
    message = "Greetings! I am your connection handler\n";
    write(sock , message , strlen(message));
     
    message = "Now type something and i shall repeat what you type \n";
    write(sock , message , strlen(message));
     
    //Receive a message from client
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
        //Send the message back to client
        write(sock , client_message , strlen(client_message));
    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
         
    //clean mems
    //Free the socket pointer
    free(socket_desc);
    
    deallocateGlobals();
     
    return 0;
}

bool checkArgc(argc)
{
    if( argc != 4 )
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool allocateGlobals(char *argv[])
{
    //argv should consist of <number of mailboxes>  
    //<size of mailbox in kbytes>  <port>  
    //<size of packet in kbytes>
    mailBoxes_global = nullptr;
    mutexLocks_global = nullptr;
    
    mailBoxes_global = new (nothrow) char [argv[0]];
    mutexLocks_global = new (nothrow) char [argv[0]];
                                            
    if( mailBoxes_global == nullptr || mutexLocks_global == nullptr)
    {
        return false;   
    }
    
    return true;    
}

void deallocateGlobals()
{
    delete [] mailBoxes_global;
    delete [] mutexLocks_global;
}