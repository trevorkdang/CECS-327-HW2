//Trevor Dang Student ID: 029014893
/* This client program connects to a server with provided IP address and port number 
in order to request a specific file from the server, request the files of the current 
working directory, or exit the connection between the client and server*/

#include <string.h>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <strings.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <vector>

using namespace std;

/*essentailly connects to the server with an IP address and port number, sends specific requests, 
sends file contents if file doesn't exists, and exits connection*/
int main (int argc, char* argv[])
{
    int listenFd, portNo;
    bool loop = false;
    struct sockaddr_in svrAdd;
    struct hostent *server;
    
    if(argc < 3)
    {
        cerr<<"Syntax : ./client <host name> <port>"<<endl;
        return 0;
    }
    
    portNo = atoi(argv[2]);
    
    if((portNo > 65535) || (portNo < 2000))
    {
        cerr<<"Please enter port number between 2000 - 65535"<<endl;
        return 0;
    }       
    
    //create client skt
    listenFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if(listenFd < 0)
    {
        cerr << "Cannot open socket" << endl;
        return 0;
    }
    
    server = gethostbyname(argv[1]);
    
    if(server == NULL)
    {
        cerr << "Host does not exist" << endl;
        return 0;
    }
    
    bzero((char *) &svrAdd, sizeof(svrAdd));
    svrAdd.sin_family = AF_INET;
    
    bcopy((char *) server -> h_addr, (char *) &svrAdd.sin_addr.s_addr, server -> h_length);
    
    svrAdd.sin_port = htons(portNo);
    
    int checker = connect(listenFd,(struct sockaddr *) &svrAdd, sizeof(svrAdd));
    
    if (checker < 0)
    {
        cerr << "Cannot connect!" << endl;
        return 0;
    }
    
    //prints that the connection has been established
    cout << "Connection established!\nIP: " << argv[1] << " Port: " << argv[2] << endl;

    //send stuff to server using loop
    for(;;)
    {
        
        char s[300]; //buffer to store user input
        cout << "Enter a valid file name on the server (can type 'dir' to list files, or 'exit' to end connection): ";
        bzero(s, 300);
        cin.getline(s, 300);

        //if 'exit' is typed in command line, end connection
        if (strcmp(s, "exit") == 0){
            cout << "Closing thread and connection..." << endl;
            break;
        }
        
        //sends the file name to the server
        write(listenFd, s, strlen(s));

        //if 'dir' is typed in command line, goes and gets the directory from server
        if (strcmp(s, "dir") == 0) {
            while (true) {
                char res[300]; //buffer
                bzero(res, 300); //buffer reset
                read(listenFd, res, 300);
                //if server sends 'End of Directory\n' break the loop, else print server response
                if (strcmp(res, "End of Directory!\n") == 0) {
                    break;
                }
                else {
                    cout << res << endl;
                }
            }
            continue; //skips rest of the iteration and goes to the next one
        }
        else {
            //stores and waits for server response
            char res[300];
            bzero(res, 300);
            read(listenFd, res, 300);

            //check if the server responded that the file already exists
            if (strcmp(res, "The file already exists!") == 0) {
                //if the file already exists, print the server's response and break
                cout << res << endl;
                break;
            }
            //checks if the file doesnt exist
            else if (strcmp(res, "The file doesn't exist! Transferring file to server...") == 0) {
                cout << res << endl;
                //the file will be transferred to the server and then opened and read if it doesn't exist
                ifstream file(s);
                string line;
                while (getline(file, line)) {
                    line += "\n";
                    write(listenFd, line.c_str(), line.size());
                }
                file.close();
            }
        }
    }
    return 0;
}
