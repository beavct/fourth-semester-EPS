#pragma once

class Labirinto {
    private:
        int linhas;
        int colunas;
        int pacdots;
        char lab[5][30];
    public:
        Labirinto();
        ~Labirinto();
        void printLabirinto();
};