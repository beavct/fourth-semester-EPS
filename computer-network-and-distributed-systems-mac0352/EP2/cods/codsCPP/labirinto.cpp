#include <bits/stdc++.h>
#include "labirinto.hpp"

using namespace std;

Labirinto::Labirinto(){
    linhas = 5;
    colunas = 27;

    memcpy(lab[0], "*****.*..**....  .   *..*****", 30); 
    memcpy(lab[1], "*****.*..*.*****.*..*.*****.*", 30);
    memcpy(lab[2], "*****.*..*.. ..  ..*..*****.*", 30);
    memcpy(lab[3], "...   .................F....", 30);
    memcpy(lab[4], "*****.*..*. ..  ..*.*****.*", 30);
}


Labirinto::~Labirinto(){

}

void Labirinto::printLabirinto(){
       int i, j;

    for (i = 0; i < this->linhas; i++) {
        for (j = 0; j < this->colunas; j++) {
            cout << this->lab[(this->linhas * i) + j];             
        }
       cout << endl;
    }
}



