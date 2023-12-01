#pragma once
#include "labirinto.hpp"

class Personagem {
    protected:
        char caractere;
        int xAtu;
        int yAtu;
        int xProx;
        int yProx;
        int score;
        int colidiuFantasma;
    public:
        Personagem();
        ~Personagem();
        void setCaractere(char caractere);
        void setPositionAtu(int x, int y);
        void setPositionProx(int x, int y);
        int getXAtu();
        int getYAtu();
        int getXProx();
        int getYProx();
        int getScore();
        int getColidiuFantasma();
        void setScore(int x);
        int verifyCycle(Labirinto Lab);
        void fixPosition(int error, Labirinto Lab, int x, int y);
        int verifyPositionPacman(Labirinto Labirinto);
        void movimentoLocal(Labirinto Labirinto);
        void movimentoRede();
};