#pragma once

class Labirinto {
    private:
        char **lab;
        int linhas;
        int colunas;
        int pacdots;
    public:
        Labirinto();
        ~Labirinto();
        void printLabirinto();
        int updateLabirinto(Labirinto Labirinto, char personagem, int x, int y);  
        char getElemento(int x, int y);
};