#pragma once
#include "labirinto.hpp"

class Personagem {
    protected:
        int xAtu;
        int yAtu;
        int xProx;
        int yProx;
        int score;
    public:
        Personagem();
        ~Personagem();
        void setPositionAtu(int x, int y);
        void setPositionProx(int x, int y);
        int getXAtu();
        int getYAtu();
        int getXProx();
        int getYProx();
        int getScore();
        void setScore(int x);
        int verifyCycle(Labirinto Lab);
        void fixPosition(int error, Labirinto Lab, int x, int y);
        void movimentoRemoto(Labirinto Lab, Personagem P, int r);
        void movimentoLocal();
        void movimentoRede();
};