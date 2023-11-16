#include <bits/stdc++.h>
#define ll long long

using namespace std;

typedef struct{
    ll ini;
    ll fim;
    ll cost;
}info;

ll binSearch(vector<info>& data, int iniIndex){

    ll j = iniIndex-1;

    for(ll i = 0; i<= j; ){
        
        ll k = (i + j) /2;

        // olha para a direita do vetor
        // pois a atividade da mediana termina antes da atividade atual se iniciar
        if(data[k].fim < data[iniIndex].ini){
            if(data[k+1].fim < data[iniIndex].ini)
                i = k+1;
            else
                return k;
        }
        // a atividade da mediana termina depois do inicio da atividade atual, olhar a esquerda do vetor
        else    
            j = k-1;
    }

    // não encontrou projeto que não conflitasse
    return -1;
}

void projects(){
    ll n;

    cin >> n;

    vector<info> data;
    info aux;

    // leitura da entrada
    for(int i=0; i<n; i++){
        cin >> aux.ini >> aux.fim >> aux.cost;
        data.push_back(aux);
    }

    // inicializa vetor resposta
    vector<ll> response(n,0);

    // ordena com base no dia final (fim)
    // ou seja, os que terminam antes aparecem primeiro no vetor
    sort(data.begin(), data.end(), [](const info& aux1, const info& aux2){
        return aux1.fim < aux2.fim;
    });

    //teste
    //cout << "vetor ordenado\n";
    //for(int i=0; i<n; i++)
    //    cout << data[i].ini << " " << data[i].fim << " " << data[i].cost << endl;

    response[0] = data[0].cost;
    //cout << "response[0]: "<<response[0] << endl;

    for(int i=1; i<n; i++){
        ll latestProj = binSearch(data, i);

        ll lucro = data[i].cost;
        if(latestProj != -1){
            //cout << response[latestProj] <<" " << latestProj<< endl;
            lucro += response[latestProj];
        }

        response[i] = max(lucro, response[i-1]);
        //cout << lucro <<" response[" << i << "]: " <<response[i] << endl;
    }

    cout << response[n-1] << endl;
}

void ingenuousCubrency(){
    ll input;

    vector<ll> cubeCoins;

    for(int i=0; i<= 21; i++){
        cubeCoins.push_back(i*i*i);
    }

    vector<ll> ways(10001, 0);

    ways[0] = 1; // pagar com 0 moedas

    for(int i=1; i<= 21; i++){
        for(int j=0; j<=10000; j++){
                if(j>= cubeCoins[i])
                    ways[j] += ways[j - cubeCoins[i]];
        }
    }

    while(cin >> input)
        cout << ways[input] << endl;

}

void editDistance(){
    string str1, str2;

    cin >> str1 >> str2;

    ll size1, size2;

    size1 = strlen(str1.c_str());
    size2 = strlen(str2.c_str());

    ll d[size1+1][size2+1];

    // inicializa a linha da str1 com o vazio
    for(int i=0; i<= size1; i++)
        d[i][0] = i; 


    // inicializa a coluna da str2 com o vazio
    for(int i=0; i<= size2; i++)
        d[0][i] = i;

    for(int i=1; i<=size1; i++){
        for(int j=1; j<=size2; j++){

            if(str1[i-1] == str2[j-1])
                d[i][j] = d[i-1][j-1];
            else
                d[i][j] = min({1+d[i-1][j-1], 1+d[i][j-1], 1+d[i-1][j]});
            
        }
    }

    cout << d[size1][size2] << endl;

}


int main(){
    projects();
    return 0;
}
