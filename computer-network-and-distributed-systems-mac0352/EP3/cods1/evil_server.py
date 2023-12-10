# coding:utf-8
from socket import *
from multiprocessing import *
from time import sleep

def dealWithClient(newSocket,destAddr):
    recvData = newSocket.recv(1024)
    # Resposta HTTP
    # 200 ok ou 100 continue ????
    # newSocket.send(b"""HTTP/1.1 100 OK\n""")
    newSocket.send(b"""HTTP/1.1 100 Continue\n""")

    # servidor echo (pelo menos parece)
    while True:
        # Quando essa linha está comentada envia várias vezes o "x:a"
        # Erro: http.client.HTTPException: got more than 100 headers
        #recvData = newSocket.recv(1024) # tava comentado
        newSocket.send(b"""foo\n""")
        # newSocket.send(b"""x:a\n""")

        # Se recebeu dados, printa 
        if len(recvData)>0:
            print('recv[%s]:%s'%(str(destAddr), recvData)) # tava comentado
            pass
        else:
            # Se não há mais dados recebidos printa "over", espera 10s e chega a conexão
            print('[%s]close'%str(destAddr))
            sleep(10)
            print('over')
            break

    #newSocket.close()

# OK
def main():

    # Cria  o socket do servidor que fica escutando em todas as interfaces na porta 8085
    serSocket = socket(AF_INET, SOCK_STREAM)
    # Permite reutilizar o endereço 
    serSocket.setsockopt(SOL_SOCKET, SO_REUSEADDR  , 1)
    localAddr = ('', 8085)
    serSocket.bind(localAddr)
    serSocket.listen(5)

    try:
        # Loop para a aceitação de conexões com clientes
        while True:
            # Aceita a conexão do cliente
            newSocket,destAddr = serSocket.accept()

            # Cria um novo processo -> paralelização
            client = Process(target=dealWithClient, args=(newSocket,destAddr))
            client.start()

            # Fecha o socket da nova conexão após iniciar o processo para que o servidor possa 
            # continuar ouvindo por novas conexões.
            newSocket.close()
    finally:
        # Fecha o socket do servidor
        serSocket.close()

if __name__ == '__main__':
    main()