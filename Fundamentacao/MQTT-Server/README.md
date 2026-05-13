# Servidor MQTT
Servidor simples que segue o protocolo MQTT (versão 3.1.1). Ele aceita pedidos "connect", "subscribe" e "publish", permitindo que múltiplos clientes se inscrevam e publiquem em tópicos simultaneamente.

# Modo de uso:
1. Compilar os códigos do servidor e da hashtable;
2. Rodar com uma porta como argumento. No geral, pode-se utilizar a porta 8080 (ex: ./server 8080);
3. A partir desse momento, o servidor está pronto para receber clientes. Eles podem ser conectados com o "mosquitto_subscribe" e "mosquitto_publish", informando o endereço IP do servidor e a porta utilizada;
