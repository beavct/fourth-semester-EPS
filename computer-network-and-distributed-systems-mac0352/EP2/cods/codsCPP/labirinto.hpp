#pragma once

class Labirinto {
    private:
        int linhas;
        int colunas;
        int pacdots;
    public:
        char lab[5][29];
        Labirinto();
        ~Labirinto();
        void printLabirinto();
        int updateLabirinto(char personagem, int x, int y);  
};