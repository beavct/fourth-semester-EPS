#pragma once
#include "personagem.hpp"
#include "fantasma.hpp"
#include "labirinto.hpp"

class Partida{
    protected:
        int players;
        int *score;
    public:
        Partida();
        ~Partida();
        void iniPartidaLocal();
};