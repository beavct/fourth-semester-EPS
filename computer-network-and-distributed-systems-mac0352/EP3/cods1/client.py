
# interface para abrir e ler URLs
import urllib.request

# Cria um objeto do tipo "request" com a URL 'http://127.0.0.1:8085'
# Será utilizada para a requisição do cliente
req = urllib.request.Request('http://127.0.0.1:8085')

# Realiza a abertura da URL especificada ('http://127.0.0.1:8085') utilizando urlopen. 
# O parâmetro timeout=1 define um limite de tempo de 1 segundo para a resposta do servidor. 
# Se o servidor não responder dentro desse limite, será gerado um erro de timeout.
response = urllib.request.urlopen(req, timeout=1)

# Servidor fica segurando quando não tem o timeout
#response = urllib.request.urlopen(req)
