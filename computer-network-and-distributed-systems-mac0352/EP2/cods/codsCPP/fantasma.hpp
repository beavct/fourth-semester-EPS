#pragma once
#include "personagem.hpp"
#include "labirinto.hpp"

class Fantasma: public Personagem {
    private:
        bool collidedPacman;
        int onPacdot;
    public:
        Fantasma();
        ~Fantasma();
        bool getCollidedPacman();
        void moveFantasma(Labirinto Labirinto, int r);
        int verifyPosition(Labirinto Labirinto, int mov);
        void putFantasma(Labirinto Labirinto);
        int getOnPacdot();
};