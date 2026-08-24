#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

struct Nodo {
    std::string Value;
    std::string Key;
    Nodo* next;
    //Inicialite Node with Constructor
    Nodo(std::string _key, std::string _Value){
        Key = _key;
        Value = _Value;
        next = nullptr;
    }
};

class HashTable {
    private:
    std::ofstream logFile;
    std::ifstream logFileR;

    std::vector<Nodo*> Buckets;
    //List numbers heads

    int HashFunction(const std::string& key){
      long hash = 0;
      int p = 31; //31 is a prime number; it was chosen because it mixes the numbers perfectly, avoiding collisions—such as with "ROMA," "AMOR," and "RAMO"—that would otherwise yield the exact same number.
      for (char c : key){
        hash = hash * p + c; // hash = First value hash(0) or Value Hash(Other num or sum) * 31 + char in ASCI.
      }
      return hash % Buckets.size(); //We divide by the array size because, otherwise, the number would be extremely large, exceeding the bucket limit.
    }

    void writeLog(const std::string& Function, const std::string& key,const std::string& Value){
      if (Function == "DEL"){
        logFile << "DEL|" << key << "\n";
      }
      else {
        logFile <<"SET|" << key <<"|"<< Value << "\n";
      } 
      logFile.flush();
    }
    void InsertSet(std::string key, std::string Value){
      int index = HashFunction(key);
      Nodo* ptr = Buckets[index];
      if (!Buckets[index])
      {
        Nodo* newNode = new Nodo(key, Value);
        //If is the first Value
        Buckets[index] = newNode;
      }
      else {
        while(ptr != nullptr)
         {
          if (key == ptr->Key)
           {
            ptr->Value = Value;
            return;
           }
           ptr = ptr->next;
         }
         Nodo* newNode = new Nodo(key, Value);
         newNode->next = Buckets[index];
         Buckets[index] = newNode;
        }
    }

    void InsertDel(std::string key){
      int index = HashFunction(key);
      Nodo* ptr = Buckets[index];
      Nodo* Prev = nullptr;
      while (ptr != nullptr){
        if (ptr->Key == key)
        {
          if (Prev == nullptr)
           {
            //if it is at the top of the list
            Buckets[index] = ptr->next;
           }
           else
           {
            //Skip pointer to delete
            Prev->next = ptr->next;
           }
           delete ptr;
           return;
        }
       Prev = ptr;
       ptr = ptr->next;
      }
    }

    void ReadData(){
      std::string line;
      std::vector<std::string> Func;
      logFileR.open("kv.log");
      if (logFileR.is_open()){
        while(std::getline(logFileR, line)){
           Func.push_back(line);
          }
      }
      for (std::string l : Func){
        std::string res;
        std::stringstream lin(l);
        std::vector<std::string> Parts;
        while (std::getline(lin,res,'|')){
         Parts.push_back(res);
        }
        if (Parts.size() >= 2){
          if (Parts[0] == "SET" && Parts.size() >= 3){
           InsertSet(Parts[1],Parts[2]);
          }
          if (Parts[0] == "DEL"){
           InsertDel(Parts[1]);
          }
        }
       }
      }
    public:
    HashTable() : Buckets(16) {
      ReadData();
      logFile.open("kv.log", std::ios::app);
    };
    //Constructor Inicialize in 0

    void SET(std::string key, std::string Value)
    {
     InsertSet(key,Value);
     writeLog("SET",key,Value);
    }

    void GET(std::string key)
    {
      int index = HashFunction(key);
      Nodo* tmp = Buckets[index];
      while(tmp != nullptr)
      //Search until nullptr is obtained
      {
       if(tmp->Key == key)
       {
       std::string Val = tmp->Value;
       std::cout<<"Result is"<< Val <<std::endl;
       break;
       }
       tmp = tmp->next;
      }
      if (tmp == nullptr) std::cout<<"Not Found"<<std::endl;
    }
    void DEL(std::string key)
    {
      std::cout<<"You sure delete?"<<std::endl;
      char res;
      std::cin >> res;
      if (res == 'Y' || res == 'y' ){
        InsertDel(key);
        writeLog("DEL",key,"");
      }
      else{
        std::cout<<"This action is not result"<<std::endl;
      }
    }
  void LOG(const std::string& name){
    std::ifstream file(name); //Open the file in reading mode 
    if(!file.is_open()){ //If file is not open, error.
      std::cout <<"Error.."<<std::endl;
    }
    std::cout<<"=== LOGS IN YOUR PROGRAM ==="<<std::endl;

    std::cout << file.rdbuf(); //With rdbuf returning data in efficient RAM blocks for sending to the terminal

    std::cout<< "=== ENDING OF LOGS ==="<<std::endl;

    //View using rdbuf, is the function c++ but i using for my program and visualize the client they logs.
  }
};