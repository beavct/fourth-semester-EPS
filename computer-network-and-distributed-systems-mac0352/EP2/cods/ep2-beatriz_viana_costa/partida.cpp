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
    int Continue = 1, totalPacdots = 54, r = 0;

    PacmanLocal.setPositionAtu(2, 13);
    PacmanLocal.setCaractere('C');

    while(totalPacdots - PacmanLocal.getScore() != 0 && Continue == 1) {

        //move o fantasma local 
        Fantasma.moveFantasma(Labirinto, r);
        r++;

        if(Fantasma.getCollidedPacman())
            Continue = 0; //perdeu a partida

        system("clear");
        //cout << endl << endl;
        Labirinto.printLabirinto();
        //cout << endl;

        //move o pacman
        if(Continue){
            //cout << "Pac-Man> ";
            PacmanLocal.movimentoLocal(Labirinto);

            if(PacmanLocal.getColidiuFantasma())
                Continue = 0;
        }   

        system("clear");
        Labirinto.printLabirinto();
    }

    if(!Continue){
        system("clear");
        Labirinto.printLabirinto();
        cout << "Game over! Pontos = " << PacmanLocal.getScore() << endl;
    }
    else{ //Continue == 2
        system("clear");
        Labirinto.printLabirinto();
        cout << "Congratulations! Pontos = " << PacmanLocal.getScore() << endl;
    } 
}