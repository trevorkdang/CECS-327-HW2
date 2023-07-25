
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
            break;
        }
        
        //sends t he file name to the server
        write(listenFd, s, strlen(s));

        if (strcmp(s, "dir") == 0) {
            string rec;
            do {
                char res[300];
                bzero(res, 300);
                read(listenFd, res, 300);
                rec = res;
                if (res != "End_of_Dir") {
                    cout << rec << endl;
                }
            } while (rec != "End_of_Dir");
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
            else if (strcmp(res, "The file doesn't exist!") == 0) {
                cout << res << endl;
                //the file will be transferred to the server and then opened and read if it doesn't exist
                ifstream file(s);
                string line;
                while (getline(file, line)) {
                    write(listenFd, line.c_str(), line.size());
                }
                file.close();
            }
        }
    }
    return 0;
}
