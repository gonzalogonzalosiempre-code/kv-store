#pragma once

#include <iostream>
#include "Hash.h"
#include <string>
#include <vector>
#include <sstream>
#include <winsock2.h> //Librery por win
#include <ws2tcpip.h> //Librery for tcp
#pragma comment(lib, "ws2_32.lib") //Vinculed librery of winsock.

std::vector<std::string> ParserCommand(const std::string& Line){
        std::string res = Line;
        std::vector<std::string> Commands;
        size_t i = res.find_first_of(' ');
        std::string tmp = res.substr(0, i);
        Commands.push_back(tmp);
        if (tmp == "SET"){
            size_t key = res.find(' ', i + 1);
            tmp = res.substr(i + 1, key - i - 1);
            Commands.push_back(tmp);
            tmp = res.substr(key + 1);
            Commands.push_back(tmp);
        }
        else if (tmp == "DEL" || tmp == "GET"){
          tmp = res.substr(i + 1);
          size_t end = tmp.find_last_not_of("\r\n");
          if (end != std::string::npos) {
           tmp = tmp.substr(0, end + 1);
          }
          Commands.push_back(tmp);
        }
     return Commands;
}

std::string Eject(HashTable& tabla,const std::vector<std::string>& Commands){
  if (Commands[0] == "SET") return tabla.SET(Commands[1], Commands[2]);
  else if (Commands[0] == "DEL") return tabla.DEL(Commands[1]);
  else if (Commands[0] == "GET") return tabla.GET(Commands[1]);
  else return "ERR unknow command\n";
}

void StartServer(){
  WSADATA wsadata;

  WORD model = MAKEWORD(2,2);

  int Start = WSAStartup(model, &wsadata);

  if (Start != 0){
    std::cout << "ERR inicialiting Code:" << Start <<std::endl;
    return;
  }
  
  SOCKET socketlisten = socket(AF_INET, SOCK_STREAM,0);

  if (socketlisten == INVALID_SOCKET){
    std::cout <<"ERR socket in" << WSAGetLastError() <<std::endl;
    WSACleanup();
    return;
  }
  sockaddr_in Serveradd;
  Serveradd.sin_family = AF_INET;
  Serveradd.sin_addr.s_addr = INADDR_ANY;
  Serveradd.sin_port = htons(8080);

  int Bindresult = bind(socketlisten, (sockaddr*)&Serveradd, sizeof(Serveradd));

  if (Bindresult != 0){
   std::cout <<"ERR socket in" << WSAGetLastError() <<std::endl;
   closesocket(socketlisten);
   WSACleanup();
   return;
  }

  int ListenResult = listen(socketlisten, SOMAXCONN);

  if (ListenResult != 0){
    std::cout<<"ERR socket in" << WSAGetLastError() <<std::endl;
    closesocket(socketlisten);
    WSACleanup();
    return;
  }

  HashTable table;


 while (true){

  std::cout<<"Waiting.." <<std::endl;

  SOCKET SocketClient = accept(socketlisten, nullptr,nullptr);

  if (SocketClient == INVALID_SOCKET){
   std::cout<<"ERR socket in"<< WSAGetLastError() <<std::endl;
   continue;
  }

  char buff[1024];
  while (true){

  int Numbersbuff = recv(SocketClient, buff, sizeof(buff) - 1, 0);

  if (Numbersbuff <= 0) break;

  buff[Numbersbuff] = '\0';

  std::string line(buff);

  std::vector<std::string> Commands = ParserCommand(line);

  std::string res = Eject(table, Commands);

  send(SocketClient, res.c_str(), res.length(), 0);
  }

  std::cout<<"Client is out, Program is closed.."<<std::endl;

  closesocket(SocketClient);

  }
}