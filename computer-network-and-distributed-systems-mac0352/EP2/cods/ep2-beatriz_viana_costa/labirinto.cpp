#include <bits/stdc++.h>
#include "labirinto.hpp"

using namespace std;

Labirinto::Labirinto(){
    this->linhas = 5;
    this->colunas = 27;
    this->pacdots = 0;
    string aux = "******.**... .....**.************.**.*******.**.************.**.*..C..*.**.******..... ....*.....*.......F..******.**.*.. ..*.**.******";

    this->lab = (char**)malloc(sizeof(char*)*5);

    int k = 0;
    for(int i=0; i<5; i++){
        this->lab[i] = (char*)malloc(sizeof(char)*27);
        for(int j=0; j<27; j++){
            this->lab[i][j] = aux[k];
            if(aux[k] == '.')
                this->pacdots++;
            k++;
        }

    }
            
    // F: Fantasma local
    // Centro: [2][13]
}


Labirinto::~Labirinto(){

}

void Labirinto::printLabirinto(){
    int i, j;

    for (i = 0; i < this->linhas; i++) {
        for (j = 0; j < this->colunas; j++) {
            cout << this->lab[i][j];             
        }
       cout << endl;
    }
}

// ERRO: A MUDANÇA NÃO ESTÁ SENDO SALVA NO LAB
int Labirinto::updateLabirinto(Labirinto Labirinto, char personagem, int x, int y){
    char save = this->lab[x][y];

    this->lab[x][y] = personagem;

    if(save == '.'){
        this->pacdots--;
        return 1; // Comeu pac-dot
    }
    
    return 0; // Não comeu pac-dot
}

char Labirinto::getElemento(int x, int y){
    return this->lab[x][y];
}

