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

void *task1 (void *dummyPt)
{
    cout << "Thread No: " << pthread_self() << endl; //prints thread ID
    char test[300]; //buffer to store messages from client
    bzero(test, 300);
    bool loop = false;
    //loop that continues until client sends 'exit'
    while(!loop)
    {    
        bzero(test, 300);
        read(connFd, test, 300); //reads message from client
        
        string tester(test); //converts message to a string
        cout << tester << endl;
        
        //if client sent 'exit', break the loop
        if(tester == "exit") {
            cout << "\nClosing thread and conn" << endl;
            break;
        }
        else if (tester == "dir") {
            DIR * dir; //pointer to a directory stream
            struct dirent *dp;

            dir = opendir(".");

            if (dir == NULL) {
            }

            string all_files;

            while ((dp = readdir(dir)) != NULL) {
                all_files += dp -> d_name;
                all_files += "\n";
            }

            closedir(dir);
            all_files += "End_of_Dir\n";
            write(connFd, all_files.c_str(),all_files.size());
        }

        //checks if the file requested by the client exists
        ifstream file(tester);
        if (file.good()) {
            //if file exists, send message to client
            char res[] = "The file already exists!";
            write(connFd, res, strlen(res));
        }
        else {
            //if file doesn't exist, send message to client
            char res[] = "The file doesn't exist!";
            write (connFd, res, strlen(res));

            //creates file name with name from client
            ofstream outfile(tester);
            while (read(connFd, test, 300) > 0) {
                outfile << test;
                bzero(test, 300);
            }
            outfile.close();
        }

    }
    close(connFd);
    return 0;
}
