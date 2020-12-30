/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2016 Sean Bradley
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.

 * Author: Sean Bradley
 * Created on 31 January 2016, 08:43
 */

#include<iostream>
#include<cstdio>
#include<cstring>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include <ifaddrs.h>
#include <thread>
#include <chrono>
#include <list>

using namespace std;

int ClientCount = 0;
std::list<int> clientList;

char* DestinationIP;
char* LocalListenPort;
char* DestinationPort;

void print_hex(const char *title, char *buff, int len)
{
    int pos = 0;
    char hex[128] = {0};
    char str[32] = {0};
    int i = 0;
	
	printf("\n%s\n", title);
    
    while ( pos < len )
    {
		unsigned char ch = buff[pos++] & 0xFF;
        str[i++] = ( ch == '\r' || ch == '\n' || ch == 0 ) ? ' ' : ch ;
        sprintf(hex + strlen(hex), "%02X ", ch);
        if ( pos % 16 == 0 )
        {
            printf("%s", hex);
            printf("   %s", str);
            printf("\n");
            memset(hex, 0, sizeof(hex));
            memset(str, 0, sizeof(str));
            i = 0;
        }
    }
    
    if ( pos % 16 != 0 )
    {
        memset(hex + strlen(hex) , ' ', 48 - strlen(hex));
        printf("%s", hex);
        printf("   %s", str);
        printf("\n");
    }
}

void *destinationResponseHandler(void *client_desc, void *destination_desc) {
    int ClientFD = *(int*) client_desc;
    int DestinationFD = *(int*) destination_desc;
    int numbytes;
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    //send(DestinationFD, "get\0", 5, 0);
    while (0 < (numbytes = recv(DestinationFD, buf, sizeof buf, 0))) {
        //buf[numbytes] = '\0';
        //cout << "EM to T : " << numbytes << " : <- " << buf << " -> " << endl;
		print_hex("<<--- ", buf, numbytes);
        send(ClientFD, buf, numbytes, 0);
        //memset(buf, 0, sizeof(buf));
    }

    return 0;
}

void *clientConnectionHandler(void *socket_desc) {
	
	std::cout << "Client connected" << std::endl;
	
    int ClientFD = *(int*) socket_desc;
    int DestinationFD;
    struct sockaddr_in Destination;
    DestinationFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    Destination.sin_family = AF_INET;
    Destination.sin_port = htons(atoi(DestinationPort));
    Destination.sin_addr.s_addr = inet_addr(DestinationIP);
    connect(DestinationFD, (struct sockaddr *) &Destination, sizeof Destination);
	std::cout << "Server connected" << std::endl;
    std::thread destinationResponseThread(destinationResponseHandler, (void*) &ClientFD, (void*) &DestinationFD);
    destinationResponseThread.detach();

    int numbytes;
	bool corrupted = false;
	int attempt = 0;
	char buf_corrupt[2048] = {0};
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    while (0 < (numbytes = recv(ClientFD, buf, sizeof buf, 0))) {
		//cout << "T to EM : " << numbytes << " : <- " << buf << " -> " << endl;
		
		attempt++;
		/*if ( attempt == 3 )
		{
			//buf[0] = 'X';
			//buf[1] = 'X';
			buf[2] = 'X';
			buf[3] = 'X';
			buf[4] = 'X';
			buf[5] = 'X';
			//corrupted = true;
		}
		if ( strstr(buf, "TERMINAL.CNF") != NULL )
		{
			memcpy(buf_corrupt, buf, numbytes);
			numbytes =  numbytes + recv(ClientFD, buf_corrupt + numbytes, sizeof buf, 0);
			print_hex(buf_corrupt, numbytes);
			send(DestinationFD, buf_corrupt, numbytes, 0);
			memset(buf, 0, sizeof(buf));
			memset(buf_corrupt, 0, sizeof(buf_corrupt));
			printf("\n<<<<<<---- Yes ---------------- We got it -------------->>>>>\n");
			continue;
		}*/
		print_hex("--->> ", buf, numbytes);
        send(DestinationFD, buf, numbytes, 0);
        memset(buf, 0, sizeof(buf));
    }
    close(DestinationFD);
    close(ClientFD);
	cout << "Client and Destination sockets are closed" << std::endl;

    return 0;
}

int list_network_interface() {
  struct ifaddrs *ifaddr, *ifa;

  if (getifaddrs(&ifaddr) == -1) {
    perror("getifaddrs");
    return -1;
  }
  
  printf("\n ----------- List interface detail -----------------\n");

  /* Walk through linked list, maintaining head pointer so we
     can free list later */
  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
 
    struct sockaddr_in *address = (struct sockaddr_in *)ifa->ifa_addr;
    printf("interface name : %s %s\n", ifa->ifa_name, inet_ntoa(address->sin_addr));
  }

  freeifaddrs(ifaddr);
  return 0;
}

int main(int argc, char** argv) {
    /*if (argc < 4) {
        cout << "Usage: " << argv[0] << " LocalListeningPort DestinationIPAddress DestinationPort" << std::endl;
        return 1;
    }
    LocalListenPort = argv[1];
    DestinationIP = argv[2];
    DestinationPort = argv[3];*/
    LocalListenPort = (char *)"5004";
    DestinationIP = (char *)"208.224.248.175";
    DestinationPort = (char *)"5004";
	
	list_network_interface();
    
    

    //create the server socket
    int ListenSocket;
    struct sockaddr_in saServer;

    ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == ListenSocket) {
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }

    saServer.sin_family = AF_INET;
    //saServer.sin_addr.s_addr = inet_addr(LocalListenIP);
    saServer.sin_addr.s_addr = INADDR_ANY;
    saServer.sin_port = htons(atoi(LocalListenPort));

    if (-1 == bind(ListenSocket, (struct sockaddr *) &saServer, sizeof (saServer))) {
        perror("Error : Bind Failed");
        close(ListenSocket);
        exit(EXIT_FAILURE);
    }
    if (-1 == listen(ListenSocket, 10)) {
        perror("Error : Listen Failed");
        close(ListenSocket);
        exit(EXIT_FAILURE);
    }
    cout << "Server listening on port " << ntohs(saServer.sin_port) << endl;

    while (1) {
        int NewClient = accept(ListenSocket, NULL, NULL);
        std::thread clientThread(clientConnectionHandler, (void*) &NewClient);
        clientThread.detach();
    }
    return 0;
}
