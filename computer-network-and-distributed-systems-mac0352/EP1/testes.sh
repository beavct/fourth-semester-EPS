#!/bin/bash

#  compilar e executar
make
gnome-terminal -- bash -c "./ep1 5672"   # abre uma nova janela de terminal e executa um comando

# pegar o PID do processo EP1
PID=$(pgrep -o ep1)

# declarar a fila de nome fila
gnome-terminal -- bash
for i in $(seq 1 $1); 
do
    amqp-declare-queue -q fila$i
done

#cadastrar $1 consumers na fila
for i in $(seq 1 $1); 
do
    gnome-terminal -- bash -c "amqp-consume -q fila$i cat"
done

# Nome do arquivo CSV para salvar os dados
arquivo_csv="$2"

# Tempo total de monitoramento em segundos (60 segundos)
tempo_total=$3

# Intervalo entre as medições em segundos (1 segundo)
intervalo=1

# Número de medições a serem realizadas
num_medicoes=$((tempo_total / intervalo))

# Crie o cabeçalho do arquivo CSV
echo "Tempo(s),Uso de CPU(%)" > "$arquivo_csv"

# Loop para coletar as medições e salvar no arquivo CSV
for ((i = 0; i < num_medicoes; i++)); do
    # clientes publicam mensagem a cada 2 segundos
    if [[ $((tempo % 2)) -eq 0 && $1 -ne 0 ]];
    then
        for j in $(seq 1 $1); 
        do
            gnome-terminal -- bash -c "amqp-publish -r fila$j -b 'teste'"
        done
    fi

    # Obtenha o uso de CPU do resultado
    cpu_usage=$(pidstat -u -p $PID | tail -n 1 | awk '{print $8}' | sed 's/,/./g')

    # Obtenha o tempo atual em segundos
    tempo=$((i * intervalo))

    # Adicione a medição ao arquivo CSV
    echo "$tempo,$cpu_usage" >> "$arquivo_csv"

    # Pausa de 1 segundo entre as medições
    sleep $intervalo
done

echo "Dados salvos em $arquivo_csv"

# Armazena os PIDs dos terminais em variáveis
PID_EP1=$(ps -ef | grep ep1 | awk '{print $2}' | head -n 1)

# Para fechar os terminais, você pode usar o comando 'kill'
 kill -TERM $PID_EP1