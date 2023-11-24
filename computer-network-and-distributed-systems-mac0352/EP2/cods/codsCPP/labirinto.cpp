#include <bits/stdc++.h>
#include "labirinto.hpp"

using namespace std;

Labirinto::Labirinto(){
    this->linhas = 5;
    this->colunas = 29;

    memcpy(lab[0], "******.**... .....**.******", 29); 
    memcpy(lab[1], "******.**.*******.**.******", 29);
    memcpy(lab[2], "******.**.*.. ..*.**.******", 29);
    memcpy(lab[3], "..... ....*.....*.......F..", 29);
    memcpy(lab[4], "******.**.*.. ..*.**.******", 29);

    // F: Fantasma local
    // Centro: [2][13]
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

int Labirinto::updateLabirinto(char personagem, int x, int y){
    char save = this->lab[x][y];

    this->lab[x][y] = personagem;

    if(save == '.')
        return 1; // Comeu pac-dot
    
    return 0; // Não comeu pac-dot
}

