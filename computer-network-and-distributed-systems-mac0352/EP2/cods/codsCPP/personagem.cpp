#include <bits/stdc++.h>
#include "personagem.hpp"
#include "labirinto.hpp"

using namespace std;

Personagem::Personagem(){
    this->score = 0;
    this->colidiuFantasma = 0;

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

int Personagem::getColidiuFantasma(){
    return this->colidiuFantasma;
}

void Personagem::setScore(int x){
    score = x;
}

int Personagem::verifyCycle(Labirinto Lab){
    int linhas, colunas;

    linhas = 5;
    colunas = 29;

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
    colunas = 29;
    if(error == 1) 
        this->setPositionProx(linhas - 1, y);
    else if(error == 2)
        this->setPositionProx(0, y);
    else if(error == 3) 
        this->setPositionProx(x, colunas - 1);
    else if(error == 4)
        this->setPositionProx(x, 0);
}

void Personagem::movimentoLocal(Labirinto Labirinto){
    char mov;
    int flag = 0;

    while (flag == 0) {
        cout << "Direção (a - esquerda, d - direita, w - cima, s - baixo): ";
        cin >> mov;

        if (mov == 'w')
            this->setPositionProx(this->getXAtu() - 1, this->getYAtu());
        else if(mov == 'a')
            this->setPositionProx(this->getXAtu(), this->getYAtu() - 1);
        else if(mov == 's')
            this->setPositionProx(this->getXAtu() + 1, this->getYAtu());
        else  //mov == 'd'
            this->setPositionProx(this->getXAtu(), this->getYAtu() + 1);

        flag = this->verifyPositionPacman(Labirinto);
    }

    if (flag == 2) {
        Labirinto.updateLabirinto(' ', this->getXAtu(), this->getYAtu());
        Labirinto.updateLabirinto('X', this->getXProx(), this->getYProx());
    }
    else if(flag == 1){
        this->fixPosition(this->verifyCycle(Labirinto), Labirinto, this->getXProx(), this->getYProx());
        Labirinto.updateLabirinto(' ', this->getXAtu(), this->getYAtu());

        this->setPositionAtu(this->getXAtu(), this->getYAtu());
    }
}

void Personagem::movimentoRede(){

}

int Personagem::verifyPositionPacman(Labirinto Labirinto){
    if(Labirinto.lab[this->getXProx()][this->getYProx()] == '*')
        return 0;
    else if(Labirinto.lab[this->getXProx()][this->getYProx()] == '.')
        this->score++;
    else if(Labirinto.lab[this->getXProx()][this->getYProx()] == 'F') {
        this->colidiuFantasma = 1;
        return 2;
    }
    return 1;
}