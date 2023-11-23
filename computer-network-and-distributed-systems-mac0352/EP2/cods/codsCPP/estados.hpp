#pragma once
#include <vector>
#include <string>

using namespace std;

typedef struct No{
    string comando;
    vector<int> transicoes;
    int indice;
    int argumentos;
}No;

class MaquinaEstados{
    private:
        vector<No> Estados; 
        int estadoAtual;

    public:
        MaquinaEstados();
        ~MaquinaEstados();
        bool comandoPermitido(string comando);
        void Teste();

};