#include <iostream>
#include "Hash.h"

int main(void){
    HashTable tabla;
    //tabla.SET("Perro","Labrador");
    //tabla.SET("Gato","michi");
    tabla.GET("Perro");
    tabla.GET("Gato");
    //tabla.SET("Perro", "Pastor Aleman");
    return 0;
}