#include <bits/stdc++.h> 
#include "estados.hpp"

MaquinaEstados::MaquinaEstados(){

    No noAux;

    // Inicializa a ação novo
    noAux.comando = "novo";
    noAux.indice = 0;
    noAux.argumentos = 2;
    //noAux.transicoes.push_back(1, 2, 3, );

    this->Estados.push_back(noAux);
    this->estadoAtual = 0;

    // Inicializa a ação senha
    noAux.comando = "senha";
    noAux.indice = 1;
    noAux.argumentos = 2;
    this->Estados.push_back(noAux);

    // Inicializa a ação entra
    noAux.comando = "entra";
    noAux.indice = 2;
    noAux.argumentos = 2;
    this->Estados.push_back(noAux);

    // Inicializa a ação l
    noAux.comando = "l";
    noAux.indice = 3;
    noAux.argumentos = 0;
    this->Estados.push_back(noAux);

    // Inicializa a ação desagio
    noAux.comando = "desafio";
    noAux.indice = 4;
    noAux.argumentos = 1;
    this->Estados.push_back(noAux);

    // Inicializa a ação move
    noAux.comando = "move";
    noAux.indice = 5;
    noAux.argumentos = 1;   
    this->Estados.push_back(noAux);

    // Inicializa a ação atraso
    noAux.comando = "atraso";
    noAux.indice = 6;
    noAux.argumentos = 0;
    this->Estados.push_back(noAux);

    // Inicializa a ação encerra
    noAux.comando = "encerra";
    noAux.indice = 7;
    noAux.argumentos = 0;
    this->Estados.push_back(noAux);

    // Inicializa a ação sai
    noAux.comando = "sai";
    noAux.indice = 8;
    noAux.argumentos = 0;
    this->Estados.push_back(noAux);

    // Inicializa a ação tchau
    noAux.comando = "tchau";
    noAux.indice = 9;
    noAux.argumentos = 0;
    this->Estados.push_back(noAux);
}

MaquinaEstados::~MaquinaEstados(){
    
}

void MaquinaEstados::Teste(){
    //for(int i=0; i<(int)this->Estados.size(); i++)
    //    cout << this->Estados[i].comando << endl;
}

bool MaquinaEstados::comandoPermitido(string comando){
    return true;
}  

int main(){
    MaquinaEstados teste;

    teste.Teste();
}