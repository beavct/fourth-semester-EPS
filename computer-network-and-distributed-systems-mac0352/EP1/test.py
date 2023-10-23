import subprocess
import threading
import time
import psutil
import pandas as pd

# Número de threads
num_threads = 5
# Número de filas
num_queues = num_threads

# Iniciar o servidor AMQP em uma thread separada
def iniciar_servidor_amqp():
    subprocess.run(["./ep1", "5672"])

# Criar uma fila com o nome "fila{i}" em uma thread separada
def criar_fila(i):
    subprocess.run(["amqp-declare-queue", "-q", f"fila{i}"])

# Iniciar um consumidor em uma fila em uma thread separada
def iniciar_consumidor(queue_name):
    subprocess.run(["amqp-consume", "-q", queue_name, "cat"])

# Medir o uso de CPU em uma thread separada
def medir_cpu(queue_name):
    while True:
        pid = subprocess.Popen(["amqp-publish", "-r", queue_name, "-b", "mensagem"]).pid
        process = psutil.Process(pid)
        cpu_percent = process.cpu_percent(interval=1)
        # Salvar medições em um DataFrame
        data = {'Queue Name': queue_name, 'Uso de CPU (%)': cpu_percent}
        df = pd.DataFrame(data, index=[0])
        df.to_csv('uso_cpu.csv', mode='a', header=False, index=False)
        time.sleep(2)

# Iniciar o servidor AMQP em uma thread separada
thread_servidor_amqp = threading.Thread(target=iniciar_servidor_amqp)
thread_servidor_amqp.start()

# Criar threads para criar filas e consumidores, medir o uso de CPU e enviar mensagens
threads = []
queue_names = [f"fila{i}" for i in range(1, num_threads + 1)]
for i in range(num_threads):
    thread_criar_fila = threading.Thread(target=criar_fila, args=(i,))

for i in range(num_threads):
    thread_iniciar_consumidor = threading.Thread(target=iniciar_consumidor, args=(queue_names[i],))
    thread_medir_cpu = threading.Thread(target=medir_cpu, args=(queue_names[i],))
    threads.extend([thread_criar_fila, thread_iniciar_consumidor, thread_medir_cpu])

for thread in threads:
    thread.start()

# Aguardar até que todas as threads terminem
for thread in threads:
    thread.join()
thread_servidor_amqp.join()
