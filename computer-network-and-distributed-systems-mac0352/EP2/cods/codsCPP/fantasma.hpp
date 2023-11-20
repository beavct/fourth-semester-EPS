#pragma once
#include "personagem.hpp"
#include "labirinto.hpp"

class Fantasma: public Personagem {
    private:
        bool remoto;
        bool collidedPacman;
    public:
        Fantasma();
        ~Fantasma();
        bool getCollidedPacman();
};