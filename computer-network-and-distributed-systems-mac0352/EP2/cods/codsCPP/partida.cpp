#include <bits/stdc++.h>
#include "partida.hpp"
#include "personagem.hpp"
#include "fantasma.hpp"
#include "labirinto.hpp"

using namespace std;

Partida::Partida(){

}

Partida::~Partida(){

}

void Partida::iniPartidaLocal(){
    Labirinto Labirinto;
    Fantasma Fantasma;
    Personagem PacmanLocal; 
    Partida Partida;
    int Continue = 1, totalPacdots = 53, r = 0;

    PacmanLocal.setPositionAtu(2, 13);

    while(totalPacdots - PacmanLocal.getScore() != 0 && Continue == 1) {

        //move o fantasma local 
        Fantasma.moveFantasma(Labirinto, r);
        r++;

        if(Fantasma.getCollidedPacman())
            Continue = 0; //perdeu a partida

        cout << endl << endl;
        Labirinto.printLabirinto();
        cout << endl;

        //move o pacman
        if(Continue){
            PacmanLocal.movimentoLocal(Labirinto);

            if(PacmanLocal.getColidiuFantasma())
                Continue = 0;
        }   
    }

    if(!Continue){
        Labirinto.printLabirinto();
        cout << "Game over! Pontos = " << PacmanLocal.getScore() << endl;
    }
    else{ //Continue == 2
        Labirinto.printLabirinto();
        cout << "Congratulations! Pontos = " << PacmanLocal.getScore() << endl;
    } 
}

int main(){
    Partida Partida;

    Partida.iniPartidaLocal();
}

