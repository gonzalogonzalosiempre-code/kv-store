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
          Commands.push_back(tmp);
        }
     return Commands;
}

std::string Eject(HashTable& tabla,const std::vector<std::string>& Commands){
  if (Commands[0] == "SET") return std::cout<< tabla.SET(Commands[1], Commands[2]);
  else if (Commands[0] == "DEL") return std::cout<< tabla.DEL(Commands[1]);
  else if (Commands[0] == "GET") return std::cout<< tabla.GET(Commands[1]);
  else return "ERR unknow command\n";
}

void StartServer(){
  WSADATA wsadata;

  WORD model = MAKEWORD(2,2);

  int Start = WSAStartup(model, &wsadata);

  if (Start != 0){
    //if not start.
  }

  //...Here come WSAStartup, socket, bind, listening, Accept, and the read loop.

}