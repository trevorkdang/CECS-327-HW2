//Trevor Dang Student ID: 029014893
/* This server program looks for a connection to a client with provided port number 
in order to fulfil a request of a specific file from the client, send the files 
of the current working directory to the client, or exit the connection between the 
client and server by responding to the 'exit' command from client.*/

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include <strings.h>
#include <stdlib.h>
#include <string>
#include <pthread.h>
#include <dirent.h>
using namespace std;

void *task1(void *);

static int connFd;

/*essentially creates a TCP socket and is binded to the specified port number, takes in connections, 
and does tasks by threading*/
int main(int argc, char* argv[])
{
    int pId, portNo, listenFd;
    socklen_t len; //store size of the address
    bool loop = false;
    struct sockaddr_in svrAdd, clntAdd;
    
    pthread_t threadA[3];
    
    if (argc < 2)
    {
        cerr << "Syntam : ./server <port>" << endl;
        return 0;
    }
    
    portNo = atoi(argv[1]);
    
    if((portNo > 65535) || (portNo < 2000))
    {
        cerr << "Please enter a port number between 2000 - 65535" << endl;
        return 0;
    }
    
    //create socket
    //listenFd = socket(AF_INET, SOCK_STREAM, 0);
    listenFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);    

    if(listenFd < 0)
    {
        cerr << "Cannot open socket" << endl;
        return 0;
    }
    
    bzero((char*) &svrAdd, sizeof(svrAdd));
    
    svrAdd.sin_family = AF_INET;
    svrAdd.sin_addr.s_addr = INADDR_ANY;
    svrAdd.sin_port = htons(portNo);
    
    //bind socket
    if(bind(listenFd, (struct sockaddr *)&svrAdd, sizeof(svrAdd)) < 0)
    {
        cerr << "Cannot bind" << endl;
        return 0;
    }
    
    listen(listenFd, 5);
    
    len = sizeof(clntAdd);
    
    int noThread = 0;

    while (noThread < 3)
    {
        
        cout << "Listening" << endl;

        //this is where client connects. svr will hang in this mode until client conn
        connFd = accept(listenFd, (struct sockaddr *)&clntAdd, &len);

        if (connFd < 0)
        {
            cerr << "Cannot accept connection" << endl;
            return 0;
        }
        else
        {
            cout << "Connection successful!" << endl;
        }
        
        pthread_create(&threadA[noThread], NULL, task1, NULL); 
        
        noThread++;
    }
    
    for(int i = 0; i < 3; i++)
    {
        pthread_join(threadA[i], NULL);
    }
    
    
}

//handles communication with client and responds to requests from client
void *task1 (void *dummyPt)
{
    cout << "Thread No: " << pthread_self() << endl; //prints thread ID
    char test[300]; //buffer to store messages from client
    bzero(test, 300); //clears buffer
    bool loop = false;

    //loop that continues until client sends 'exit'
    while(!loop)
    {    
        bzero(test, 300); //clears buffer 
        read(connFd, test, 300); //reads message from client
        
        string tester(test); //converts message to a string
        cout << tester << endl;
        
        //if client sent 'exit', break the loop
        if(tester == "exit") {
            break;
        }

        //if client sent 'dir', send all files in current directory
        else if (tester == "dir") {
            DIR * dir; //pointer to a directory stream
            struct dirent *dp;

            dir = opendir("."); //uses opendir function to open directory

            //this while loop helps to get each entry in the directory
            while ((dp = readdir(dir)) != NULL) {
                string all_files(dp -> d_name);
                all_files += "\n";
                if (write(connFd, all_files.c_str(), all_files.size()) < 0) {
                    cerr << "Error writing to socket" << endl;
                }
                else {
                    cout << endl;
                }
            }
            closedir(dir); //uses closedir to close directory
            string end_of_dir = "End of Directory!\n"; //string that signals end of the directory
            if (write(connFd, end_of_dir.c_str(), end_of_dir.size()) < 0) {
                cerr << "Error writing to socket" << endl;
            }
            else {
                cout << endl; 
            }
        }
        else {
            //checks if the file requested by the client exists, if not transfer file to server
            ifstream file(tester);
            if (file.good()) {
                //if file exists, send message to client
                char res[] = "The file already exists!";
                write(connFd, res, strlen(res));
            }
            else {
                char res[] = "The file doesn't exist! Transferring file to server...";
                write(connFd, res, strlen(res)); 
            }
        }

    }
    close(connFd);
    return 0;
}
