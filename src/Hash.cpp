#include <vector>
#include <string>
#include <iostream>
#include <fstream>

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
    public:
    HashTable() : Buckets(16) {};
    //Constructor Inicialize in 0

    void SET(std::string key, std::string Value)
    {
      int index = HashFunction(key);

      Nodo* newNode = new Nodo(key, Value);

      if (!Buckets[index])
      {
        //If is the first Value
        Buckets[index] = newNode;
      }
      else
      {
        newNode->next = Buckets[index]->next;
        Buckets[index]->next = newNode;
      }
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
      int index = HashFunction(key);
      Nodo* ptr = Buckets[index];
      Nodo* Prev = nullptr;
      while (ptr != nullptr)
      {
        if (ptr->Key == key)
        {
          std::string Val = ptr->Value;
          char res;
          std::cout<<"You sure Delete "<< Val <<"? y/n"<<std::endl;
          std::cin >> res;
          //Get user response
          if(res == 'y' || res =='Y') 
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
           delete[] ptr;
          }
          else
          {
           std::cout<<"This action is not result"<<std::endl;
          }
        }
        Prev = ptr;
        ptr = ptr->next;
        delete ptr;
      }
    }
  void LOG(const std::string& name){
    std::ifstream file(name); //Open the file in reading mode 
    if(!file.is_open()){ //If file is not open, error.
      std::cout <<"Error.."<<endl;
    }
    std::cout<<"=== LOGS IN YOUR PROGRAM ==="<<std::endl;

    std::cout << file.rdbuf(); //With rdbuf returning data in efficient RAM blocks for sending to the terminal

    std::cout<< "=== ENDING OF LOGS ==="<<std::endl;

    //View using rdbuf, is the function c++ but i using for my program and visualize the client they logs.
  }
};