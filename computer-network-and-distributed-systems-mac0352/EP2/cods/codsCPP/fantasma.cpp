#include <bits/stdc++.h>
#include "personagem.hpp"
#include "labirinto.hpp"
#include "fantasma.hpp"

using namespace std;

Fantasma::Fantasma(){
    this->collidedPacman = 0;
}

Fantasma::~Fantasma(){
    
}

bool Fantasma::getCollidedPacman(){
    return collidedPacman;
}