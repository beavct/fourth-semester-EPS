#include <bits/stdc++.h>
#include "personagem.hpp"
#include "labirinto.hpp"

using namespace std;

Personagem::Personagem(){
    score = 0;
}

Personagem::~Personagem(){
    
}

void Personagem::setPositionAtu(int x, int y){
    this->xAtu = x;
    this->yAtu = y;
}

void Personagem::setPositionProx(int x, int y){
    this->xProx = x;
    this->yProx = y;
}

int Personagem::getXAtu(){
    return xAtu;
}

int Personagem::getYAtu(){
    return yAtu;
}

int Personagem::getXProx(){
    return xProx;
}

int Personagem::getYProx(){
    return yProx;
}

int Personagem::getScore(){
    return score;
}

void Personagem::setScore(int x){
    score = x;
}

int Personagem::verifyCycle(Labirinto Lab){
    int linhas, colunas;

    linhas = 5;
    colunas = 27;

    if (this->xProx < 0) //Foi para cima do tabuleiro, ou seja, na próx rodada tem que ir para a última linha.
        return 1; 
    else if(this->xProx > linhas) //Foi para baixo do tabuleiro, ou seja, na próx rodada tem que ir para a primeira linha.
        return 2;
    else if (this->yProx < 0) //Foi para esquerda do tabuleiro, ou seja, na próx rodada tem que ir para a última coluna.
        return 3;
    else if (this->yProx > colunas) //Foi para a direita do tabuleiro, ou seja, na próx rodada tem que ir para a primeira coluna.
        return 4;
    else
        return 0; //Se ta tudo ok e não precisa mudar a posição por conta do ciclo.
}

void Personagem::fixPosition(int error, Labirinto Lab, int x, int y) {
    int linhas, colunas;

    linhas = 5;
    colunas = 27;
    if(error == 1) 
        this->setPositionProx(linhas - 1, y);
    else if(error == 2)
        this->setPositionProx(0, y);
    else if(error == 3) 
        this->setPositionProx(x, colunas - 1);
    else if(error == 4)
        this->setPositionProx(x, 0);
}

void Personagem::movimentoRemoto(Labirinto Lab, Personagem P, int r){
    default_random_engine gerador(r);
    //int mov = 0;
//
    //while (!this->verifyPosition(Lab, P, mov)) {
    //    /*MOVIMENTAÇÃO:
    //    1: cima 
    //    2: direita
    //    3: baixo
    //    4: esquerda*/
    //    uniform_int_distribution<int> distrI(1,4);
    //    mov = distrI(gerador);
//
    //    if(mov == 1){
    //        this->setPositionProx(this->getXAtu(), this->getYAtu() - 1);
    //    }
    //    else if(mov == 2){
    //        this->setPositionProx(this->getXAtu() + 1, this->getYAtu());
    //    }
    //    else if(mov == 3){
    //        this->setPositionProx(this->getXAtu(), this->getYAtu() + 1);
    //    }
    //    else { //mov == 4
    //        this->setPositionProx(this->getXAtu() - 1, this->getYAtu());
    //    }
//
    //    this->fixPosition(this->verifyCycle(Lab), Lab, this->getXProx(), this->getYProx());
    //}

    //if (this->verifyPosition(Lab, P, mov) == 1) {
    //    this->putGhost(Lab, P);
    //}
//
    //else if (this->verifyPosition(Lab, P, mov) == 2) {
    //    this->collidedPacman = 1;
    //    Lab.updateLab(Lab, 'X', this->getXProx(), this->getYProx()); 
    //}

}

void Personagem::movimentoLocal(){

}

void Personagem::movimentoRede(){

}