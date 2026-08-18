#include <vector>
#include <string>

struct Nodo {
    std::string Value;
    int Key;
    Nodo* next;
    //Inicialite Node with Constructor
    Nodo(int _key, std::string _Value){
        Key = _key;
        Value = _Value;
        next = nullptr;
    }
};

class HashTable {
    private:
    std::vector<Nodo*> Buckets(16);
    //List numbers heads
    void HashFunction(std::string& key){
        //Logic Function
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
        Buckets[index]->next = newNode->Value;
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
      while(!tmp == nullptr)
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
      Node* ptr = Buckets[index];
      Node* Prev = nullptr;
      while (!ptr == nullptr)
      {
        if (ptr->Key == key)
        {
          std::string Val = ptr->Value;
          std::string res;
          std::cout<<"You sure Delete "<< Val <<"? y/n"<<std::endl;
          std::cin >> res;
          //Get user response
          if(res == 'y' || res =='Y') 
          {
           if (Del == nullptr)
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
        Del = ptr;
        ptr = ptr->next;
      }
    }
};