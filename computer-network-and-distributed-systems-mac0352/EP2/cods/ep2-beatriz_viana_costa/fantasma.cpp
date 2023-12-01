#include <bits/stdc++.h>
#include "personagem.hpp"
#include "labirinto.hpp"
#include "fantasma.hpp"

using namespace std;

Fantasma::Fantasma(){
    this->collidedPacman = 0;
    this->onPacdot = 1;
    this->setPositionAtu(3, 24);
    this->setPositionProx(3, 24);
}

Fantasma::~Fantasma(){
    
}

bool Fantasma::getCollidedPacman(){
    return collidedPacman;
}

void Fantasma::moveFantasma(Labirinto Labirinto, int r){
    default_random_engine gerador(r);
    int mov = 0;

    while (!this->verifyPosition(Labirinto, mov)) {
        /*MOVIMENTAÇÃO:
        1: cima 
        2: direita
        3: baixo
        4: esquerda*/
        uniform_int_distribution<int> distrI(1,4);
        mov = distrI(gerador);

        if(mov == 1){
            this->setPositionProx(this->getXAtu(), this->getYAtu() - 1);
        }
        else if(mov == 2){
            this->setPositionProx(this->getXAtu() + 1, this->getYAtu());
        }
        else if(mov == 3){
            this->setPositionProx(this->getXAtu(), this->getYAtu() + 1);
        }
        else { //mov == 4
            this->setPositionProx(this->getXAtu() - 1, this->getYAtu());
        }

        this->fixPosition(this->verifyCycle(Labirinto), Labirinto, this->getXProx(), this->getYProx());
    }

    if (this->verifyPosition(Labirinto, mov) == 1) {
        this->putFantasma(Labirinto);
    }

    else if (this->verifyPosition(Labirinto, mov) == 2) {
        this->collidedPacman = 1;
        Labirinto.updateLabirinto(Labirinto, 'X', this->getXProx(), this->getYProx()); 
    }

}

int Fantasma::verifyPosition(Labirinto Labirinto, int mov){
    if(Labirinto.getElemento(this->getXProx(), this->getYProx()) == '*' || Labirinto.getElemento(this->getXProx(), this->getYProx()) == 'F' || mov == 0)
        return 0;
    else if(Labirinto.getElemento(this->getXProx(), this->getYProx()) == 'C') //colidiu com o Pacman
        return 2;
    else 
        return 1;
}

void Fantasma::putFantasma(Labirinto Labirinto){
    int flag = 0;

    //cout << "fantasma: " << this->getXAtu() << " " << this->getYAtu() << endl;

    if (Labirinto.getElemento(this->getXProx(), this->getYProx()) == '.')
        flag = 1;

    Labirinto.updateLabirinto(Labirinto, 'F', this->getXProx(), this->getYProx());

    if (this->getOnPacdot() == 1) {
        Labirinto.updateLabirinto(Labirinto, '.', this->getXAtu(), this->getYAtu());
        this->onPacdot = 0;
    }
    else
        Labirinto.updateLabirinto(Labirinto, ' ', this->getXAtu(), this->getYAtu());

    if (flag == 1)
        this->onPacdot = 1;

    this->setPositionAtu(this->getXProx(), this->getYProx());
}

int Fantasma::getOnPacdot(){
    return onPacdot;
}