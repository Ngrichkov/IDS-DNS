CC = gcc

all: DNS_proxy_server random_server

DNS_proxy_server:
	$(CC) DNS_proxy_server.c utils.c utils.h -o dns_proxy_server -lm -lpthread

random_server:
	$(CC) random_server.c -o random_server

.PHONY: clean
clean:
	rm -f dns_proxy_server random_server