#pragma once

#include <iostream>
#include "Hash.h"
#include <string>
#include <vector>
#include <sstream>
#include <winsock2.h> //Librery por win
#include <ws2tcpip.h> //Librery for tcp
#pragma comment(lib, "ws2_32.lib") //Vinculed librery of winsock.

std::vector<std::string> ParserCommand(const std::string& Line){ //First, we create a function that breaks down the commands one by one so they can be used by our class functions.
        std::string res = Line; //res = Line, for using with substr and append to tmp
        std::vector<std::string> Commands;
        size_t i = res.find_first_of(' ');
        std::string tmp = res.substr(0, i); // Get string for Command
        Commands.push_back(tmp); //If Set
        if (tmp == "SET"){
            size_t key = res.find(' ', i + 1);
            tmp = res.substr(i + 1, key - i - 1);
            Commands.push_back(tmp);
            tmp = res.substr(key + 1);
            Commands.push_back(tmp);
        }
        else if (tmp == "DEL" || tmp == "GET"){ //If del o get, Note using similar logic for two commands, this is because Both require only two in the parameters.
          tmp = res.substr(i + 1);
          size_t end = tmp.find_last_not_of("\r\n"); //Find up to the end of the word
          if (end != std::string::npos) {
           tmp = tmp.substr(0, end + 1);
          }
          Commands.push_back(tmp); //Push back on Commands
        }
     return Commands;//Return vector the strings
}

std::string Eject(HashTable& tabla,const std::vector<std::string>& Commands){ //Ejecuting Functions for commands, this is for Vector of string called commands.
  if (Commands[0] == "SET") return tabla.SET(Commands[1], Commands[2]);
  else if (Commands[0] == "DEL") return tabla.DEL(Commands[1]);
  else if (Commands[0] == "GET") return tabla.GET(Commands[1]);
  else return "ERR unknow command\n";
}

void StartServer(){
  WSADATA wsadata;  //Notice how we use `WSADATA` here; it is used to create—so to speak—the structure that holds the specifications for the Winsock program we are going to use, which is what Winsock requires.

  WORD model = MAKEWORD(2,2); //This is WORD is a data type created by Windows to handle 16 bits—comprising 8 low-order bits and 8 high-order bits—to suit system specifications or Winsock version requirements. Why WORD? It is an unsigned integer; this avoids the hassle of isolating the low-order byte from the mix of negative and positive values ​​found in a standard `int`, saving that extra step.

  int Start = WSAStartup(model, &wsadata); //Start WSAStartup function, Fill everything in and use the parameters we provided to do so; this will initialize Winsock and allow you to use its ecosystem.

  if (Start != 0){ //If error inicialiting.
    std::cout << "ERR inicialiting Code:" << Start <<std::endl;
    return;
  }
  
  SOCKET socketlisten = socket(AF_INET, SOCK_STREAM,0); //Well CREATING SOCKET, AF_INET It is basically the addressing model that will be used—either IPv4 or IPv6. We use IPv4 because that is what we need; although we could use IPv6, it isn't necessary, and IPv4 is just right.
  //SOCK_STREAM It's how the data is sent—like TPS or UDS. We use TPS; although it is slower, it ensures the data is transmitted securely, and it is the default option.
  if (socketlisten == INVALID_SOCKET){
    std::cout <<"ERR socket in" << WSAGetLastError() <<std::endl; //WSASGetLastError Provide the specific error code that occurred; it’s not strictly necessary, but it’s a good habit to keep in mind for future bugs or fixes.
    WSACleanup(); //Closed.
    return;
  }
  sockaddr_in Serveradd; //This is interesting: `sockaddr_in` is like the form (the struct) used for IPv4 addresses.
  Serveradd.sin_family = AF_INET; //AF_INET, view in socket
  Serveradd.sin_addr.s_addr = INADDR_ANY; //It allows for any type of address without requiring specification; it is excellent for communication programs, such as those used with Raspberry Pi, etc.
  Serveradd.sin_port = htons(8080); //Htons converts a 16-bit short integer from host byte order to network byte order

  int Bindresult = bind(socketlisten, (sockaddr*)&Serveradd, sizeof(Serveradd)); //Note the cast to `sockaddr*`; this is because `sockaddr` acts as a generic template. In reality, the cast allows the system to inspect the first two bytes to determine whether it is dealing with IPv4 or IPv6 and populate the appropriate data fields accordingly, without needing separate templates for each type—this is how `bind` supports any address type.

  if (Bindresult != 0){
   std::cout <<"ERR socket in" << WSAGetLastError() <<std::endl;
   closesocket(socketlisten);
   WSACleanup();
   return;
  }

  int ListenResult = listen(socketlisten, SOMAXCONN); //It listens like a socket in listening mode; SOMAXCONN is the per-user limit, and it places the connection in the queue.

  if (ListenResult != 0){
    std::cout<<"ERR socket in" << WSAGetLastError() <<std::endl;//The similar en ERR socket and Bind
    closesocket(socketlisten);
    WSACleanup();
    return;
  }

  HashTable table; //Creating HashTable for Client in queue


 while (true){

  std::cout<<"Waiting.." <<std::endl; //Message por waiting for clients

  SOCKET SocketClient = accept(socketlisten, nullptr,nullptr);//Accept is what determines whether the client gets through; it accepts the client and creates a new socket specifically for them.

  if (SocketClient == INVALID_SOCKET){
   std::cout<<"ERR socket in"<< WSAGetLastError() <<std::endl;
   continue;
  }

  char buff[1024]; //Buf por recv 
  while (true){ //While for waiting clients

  int Numbersbuff = recv(SocketClient, buff, sizeof(buff) - 1, 0);//Recive the buff for action

  if (Numbersbuff <= 0) break;//Is not action, break

  buff[Numbersbuff] = '\0';

  std::string line(buff);

  std::vector<std::string> Commands = ParserCommand(line);

  std::string res = Eject(table, Commands);

  send(SocketClient, res.c_str(), res.length(), 0); //Send commands for client
  }

  std::cout<<"Client is out, Program is closed.."<<std::endl;

  closesocket(SocketClient);//Close socketClient

  //This is Winsock, which enables network communication between a client and a server; `bind` brings the socket to life by assigning it a place on the network, while the other functions facilitate program communication. However, the implementation still needs refinement—specifically using threads and improving it to handle multiple clients.


  }
}