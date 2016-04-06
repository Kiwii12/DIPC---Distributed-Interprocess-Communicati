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


//For Parsing strings
#include <vector>
#include <sstream>

#include <iostream> //cout cin endl

#include <cstring>

using namespace std;

//global handlers
bool allocateGlobals(char *argv[]);
void deallocateGlobals(char *argv[]);

bool checkArgc(int argc);

bool checkArgments(char *currentArg, int sock);

void handleRead(int mailBox, int sock);
void handleWrite(int mailBox, int sock);
 

char** mailBoxes_global;
int*  mutexLocks_global;

int packetSize_global;
int numberOfMailBoxes_global;

//the thread function
void *connection_handler(void *);

/*********************************************************************//**
 *	Main
 *
 * 	Creates the Server and allocateds the shared memory
*//**********************************************************************/
int main(int argc , char *argv[])
{
    //check argc value
    if(!checkArgc(argc))
    {
        cerr << "Usage: DIPC <number of mailboxes><size of mailbox in kbytes><port> <size of packet in kbytes>" << endl;
        return -1;
    }
    else //allocate mems
    {
        if( !allocateGlobals(argv) )
        {
            cerr << "failed to allocate memory, exiting" << endl;
            return -2;   
        }
        else
        {
        	for( int i = 0; i < atoi(argv[1]); i++)
        	{
        		cout << i << " is in i" << endl;
        		mutexLocks_global[i] = 0;
        		memset(mailBoxes_global[i], 0, sizeof(mailBoxes_global[i]));
        		strcpy(mailBoxes_global[i], "Empty");
        	}
        }
    }

    int currentPort = atoi(argv[3]);
    packetSize_global = (atoi(argv[4]) * 1000);
    numberOfMailBoxes_global = (atoi(argv[1]));
    
    
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    cout << "Attempting to connect to port: " << currentPort << endl;
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( currentPort );
     
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

    cout << "set up on port " << currentPort << endl;
     
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
        new_sock = (int*)malloc(1);
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
     
    //clean mems
    deallocateGlobals(argv);
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


    char* client_message = new (nothrow) char [packetSize_global];

    string message;
     
    //Send some messages to the client
    message = "Connected, ready to access shared memory\n";
    write(sock , message.c_str() , strlen(message.c_str()));
     
    message = "type r to read whats in memory, w to change it, and q to quit \n";
    write(sock , message.c_str() , strlen(message.c_str()));
    message = "of course, you will also need to supply a mailbox number \n";
    write(sock , message.c_str() , strlen(message.c_str()));
    message = "There are " + to_string(numberOfMailBoxes_global) + " mailboxes \n";
    write(sock , message.c_str() , strlen(message.c_str()));
    message = "Mailbox numbers start at 1 \n";
    write(sock , message.c_str() , strlen(message.c_str()));
     
    //Receive a message from client
    while( (read_size = recv(sock , client_message , packetSize_global , 0)) > 0 )
    {
        //Send the message back to client
        //write(sock , client_message , strlen(client_message));

        //needs to get r, w, q
        if ( !checkArgments(client_message, sock) )
    	{
    		puts("Client disconnected");
        	fflush(stdout);
        	return 0;
    	}

    	memset(client_message, 0, sizeof(client_message));
    	fflush(stdout);
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
         
    //Free the socket pointer
    free(socket_desc);

    delete [] client_message;
     
    return 0;
}

/*********************************************************************//**
 *	checkArgc
 *
 * 	simple function that just checks if the right amount of args are
 *		passed in.
*//**********************************************************************/
bool checkArgc(int argc)
{
    if( argc != 5 )
    {
        return false;
    }
    else
    {
        return true;
    }
}

/*********************************************************************//**
 *	allocateGlobals
 *
 * 	allocates the memory for the global arrays
*//**********************************************************************/
bool allocateGlobals(char *argv[])
{
    //argv should consist of <number of mailboxes>  
    //<size of mailbox in kbytes>  <port>  
    //<size of packet in kbytes>
    mailBoxes_global = nullptr;
    mutexLocks_global = nullptr;
    
    mailBoxes_global = new (nothrow) char* [atoi(argv[1])];
    mutexLocks_global = new (nothrow) int [atoi(argv[1])];
                                            
    if( mailBoxes_global == nullptr || mutexLocks_global == nullptr)
    {
        return false;   
    }

    //loop variables
	int i = 0;
	int j = 0;

	//loops through and allocated the second part
	for (i=0; i < atoi(argv[1]); i++)
	{
		mailBoxes_global[i] = new (nothrow) char [((atoi(argv[2])) * 1000)]; //converted to kilobytes

		//delets all allocated memory if an error occured
		if (mailBoxes_global[i] == nullptr)
		{
			for (j=0; j < i; j++)
			{
				delete [] mailBoxes_global[j];
			}
			delete [] mailBoxes_global;
			mailBoxes_global = nullptr;
			return nullptr;
		}
	}
    
    return true;    
}

/*********************************************************************//**
 *	deallocateGlobals
 *
 * 	deallocates the memory for the global arrays
*//**********************************************************************/
void deallocateGlobals(char *argv[])
{
	//loop variable
	int j = 0;

	//deletes array starting with furthest allocation
	for (j=0; j < atoi(argv[1]); j++)
	{
		delete [] mailBoxes_global[j];
	}
	delete [] mailBoxes_global;
	delete [] mutexLocks_global;

	//sets array back to nullptr
	mailBoxes_global = nullptr;
	mutexLocks_global = nullptr;
}


/*********************************************************************//**
 *	checkArgments
 *
 * 	parses the input from user and determines what the user is trying
 *		to do with the mail box system.
*//**********************************************************************/
bool checkArgments(char *args, int sock)
{
	string curretnArgs = args;
	string tempForArgs = "";
	vector<string> parseStringVector;

	string message = "";

	istringstream myStringStream(curretnArgs);
	while (getline(myStringStream, tempForArgs, ' '))
	{
		istringstream myStringStream2(tempForArgs);
		//attempted to remove charage return so that telnet would work
		//it didn't work
		while (getline(myStringStream2, message, '\r')) 
		{
			if(!tempForArgs.empty())
			{
				cout << tempForArgs << " is in tempForArgs" << endl;
				parseStringVector.push_back(tempForArgs);
			}
		}
	}

	string currentFront = parseStringVector.at(0);
	cout << currentFront << " is in currentFront" << endl;

	//if only 1 argument that isn't q
	if( ( parseStringVector.size() != 2 ) || currentFront == "q")
	{
		message = "Usage: <option> <mailbox #>\n";
		write(sock , message.c_str() , strlen(message.c_str()));
		parseStringVector.clear();
		return true;
	} //if argument is r
	else if ( currentFront == "r")
	{
		int tempInt;
		string tempString = parseStringVector.at(1);
		istringstream ( tempString ) >> tempInt;
		handleRead(tempInt, sock);
		parseStringVector.clear();
	} //if argument is w
	else if ( currentFront == "w")
	{
		int tempInt;
		string tempString = parseStringVector.at(1);
		istringstream ( tempString ) >> tempInt;
		handleWrite(tempInt, sock);
		parseStringVector.clear();
	} //if argument is q
	else if ( strcmp(currentFront.c_str(), "q"))
	{ 
		parseStringVector.clear();
		return false;
	}

	parseStringVector.clear(); //make gosh darn sure there is no overwrite
	return true;
}


/*********************************************************************//**
 *	handleRead
 *
 * 	handles reading from mailbox - only allows read when no one is writing
*//**********************************************************************/
void handleRead(int mailBox, int sock)
{
	mailBox--; //puts number in line with arrays
	string message;

	cout << "This is the handleRead function" << endl;

	cout << "Global Memory:" << endl;
	for( int i = 0; i < numberOfMailBoxes_global; i++)
	{
		cout << "mutexLocks_global [i] = " << mutexLocks_global[i] << endl;
		cout << "mailBoxes_global[i]   = " << mailBoxes_global[i] << endl;
	}


	if( mailBox >= numberOfMailBoxes_global || mailBox < 0)
	{
		//Send some messages to the client
		message = "invalid mailbox\n";
		write(sock , message.c_str() , strlen(message.c_str()));
		return;
	}

	int count = 0;

	//cout << "mailbox = " << mailBox << " and mutexLocks_global[mailBox] = " << mutexLocks_global[mailBox] < endl;
	//waiting until unlocked
	while(mutexLocks_global[mailBox] != 0)
	{
		if( count == 0)
		{
			message = "waiting for writer\n";
			write(sock , message.c_str() , strlen(message.c_str()));
		}
		// if( count > 1000 )
		// {
		// 	return;
		// }
		count++;
	}

	message = "Information currently in mailbox " + to_string(mailBox + 1) + ":\n";
	write(sock , message.c_str() , strlen(message.c_str()));
	//message = str_cpy(mailBoxes_global[mailBox]) + "\n";
	//cout << "string length of mailbox shared memory is " << strlen(mailBoxes_global[mailBox]) << endl;
	if( strlen(mailBoxes_global[mailBox]) > 1)
	{
		write(sock , mailBoxes_global[mailBox] , strlen(mailBoxes_global[mailBox]));
	}
	else
	{
		message = "Mailbox currently empty\n";
		write(sock , message.c_str() , strlen(message.c_str()));
	}
}

/*********************************************************************//**
 *	handleWrite
 *
 * 	handles writing mailbox - will lock mailbox until done.
*//**********************************************************************/
void handleWrite(int mailBox, int sock)
{
	mailBox--; //puts number in line with arrays
	string message;

	cout << "This is the handleWrite function" << endl;




	char* client_message = new (nothrow) char [packetSize_global];

	if( mailBox >= numberOfMailBoxes_global || mailBox < 0)
	{
		//Send some messages to the client
		message = "invalid mailbox\n";
		write(sock , message.c_str() , strlen(message.c_str()));
		return;
	}

	int count = 0;

	//cout << "mailbox = " << mailBox << " and mutexLocks_global[mailBox] = " << mutexLocks_global[mailBox] < endl;
	//waiting until unlocked
	while(mutexLocks_global[mailBox] != 0)
	{
		if( count == 0)
		{
			message = "waiting for writer\n";
			write(sock , message.c_str() , strlen(message.c_str()));
		}
		// if( count > 1000 )
		// {
		// 	return;
		// }
		count++;
	}
	mutexLocks_global[mailBox] = 1;

	message = "Information currently in mailbox " + to_string(mailBox + 1) + ":\n";
	write(sock , message.c_str() , strlen(message.c_str()));
	//message = str_cpy(mailBoxes_global[mailBox]) + "\n";
	cout << "string length of mailbox shared memory is " << strlen(mailBoxes_global[mailBox]) << endl;
	if( strlen(mailBoxes_global[mailBox]) == 0)
	{
		write(sock , mailBoxes_global[mailBox] , strlen(mailBoxes_global[mailBox]));
	}
	else
	{
		message = "Mailbox currently empty\n";
		write(sock , message.c_str() , strlen(message.c_str()));
	}

	message = "Input new message now\n";
	write(sock, message.c_str(), strlen(message.c_str()));

	int read_size = recv(sock , client_message , packetSize_global , 0);

	//Send the message back to client
    write(sock , client_message , strlen(client_message));

    message = "Will be writen to the mailbox\n";
    write(sock, message.c_str(), strlen(message.c_str()));

    strcpy(mailBoxes_global[mailBox], client_message);

    mutexLocks_global[mailBox] = 0;

    delete [] client_message;
}