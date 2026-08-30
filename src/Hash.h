#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <mutex>

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
    std::mutex mtx;
    std::ofstream logFile; //Let's open a reserved object for a file stream for writing.
    std::ifstream logFileR; //We open another one in read-only mode.

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
        logFile << "DEL|" << key << "\n";  //This function writes to logFile; since we will soon open the file as kv.log, I will write to logFile.
      }
      else {
        logFile <<"SET|" << key <<"|"<< Value << "\n";
      } 
      logFile.flush(); //The function pushes all data directly to the hard drive, thereby preventing data loss due to potential crashes or sudden shutdowns.
    }
    void InsertSet(std::string key, std::string Value){ // Function created to solve the problem: to ensure that the subsequent ReadData function—which reads the entire log to save data upon program closure—can successfully reconstruct it, this function is dedicated solely to inserting files, thereby separating the tasks.
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

    bool InsertDel(std::string key){ //Same the function InsertSet
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
           return true;
        }
       Prev = ptr;
       ptr = ptr->next;
      }
      return false;
    }

    void ReadData(){  //This function reads data from the log file and iterates through each entry to call `insertSet` and `insertDel`.
      std::string line;
      std::vector<std::string> Func;
      logFileR.open("kv.log");
      if (logFileR.is_open()){  //If the file was opened using `ifstream` in read mode...
        while(std::getline(logFileR, line)){ //As `getline` iterates over each line, add it to the `func` vector.
           Func.push_back(line);
          }
      }
      for (std::string l : Func){
        std::string res;
        std::stringstream lin(l); //We convert it into a data stream.
        std::vector<std::string> Parts; //Note that we use `std::string<std::string> Parts` inside the loop; this is intentional so that we don't have to call `clear()` on it—it gets cleared automatically.
        while (std::getline(lin,res,'|')){ //For each line in the function, using `getline`, we split by '|'.
         Parts.push_back(res); //Add res in Parts
        }
        if (Parts.size() >= 2){ //If it has two or more values, then it executes one of the two functions.
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
    HashTable() : Buckets(16) { //Constructor Inicialize in 0
      ReadData(); //Calling the Function on constructor
      logFile.open("kv.log", std::ios::app); //Note open with std::ios::app This ensures that the existing content in kv.log isn't overwritten, but rather that new data is written into the existing file, allowing for reconstruction.
    };

    std::string SET(std::string key, std::string Value)
    {
     std::lock_guard<std::mutex> lock(mtx);
     InsertSet(key,Value); //Calling the function for task
     writeLog("SET",key,Value);
     return "OK\n";
    }

    std::string GET(std::string key)
    {
      std::lock_guard<std::mutex> lock(mtx);
      int index = HashFunction(key);
      Nodo* tmp = Buckets[index];
      while(tmp != nullptr)
      //Search until nullptr is obtained
      {
       if(tmp->Key == key)
       {
       std::string Val = tmp->Value;
       std::string res = "Result is " + Val;
       return res;
       }
       tmp = tmp->next;
      }
      return "Not Found\n";
    }
    std::string DEL(std::string key){
      std::lock_guard<std::mutex> lock(mtx);
      bool Succes = InsertDel(key);
      if (Succes){
        writeLog("DEL",key,"");
        return "Delete Successful\n";
      }
      else{
        return "This action is not result\n"; //If not view the message
      }
     }
  //This function is for future in new updates.. Is log for every in terminal using streambuf and rdbuf and virtual for function of low code c++.
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