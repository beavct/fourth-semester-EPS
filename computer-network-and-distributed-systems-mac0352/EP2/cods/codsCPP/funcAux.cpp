#include <bits/stdc++.h> 
#include "funcAux.hpp"

using namespace std;

int charToInt(char* msg, int size) {
    int valor = 0;
    for (int i = 0; i < size; i++) {
        /*shift de 8 bits*/
        valor = valor << 8;
        valor += msg[i];
    }

    return valor;
}