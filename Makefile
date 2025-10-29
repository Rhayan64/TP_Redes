CC = gcc
CFLAGS = -Wall -O2

CLIENTE_SRC = TP_Redes_Cliente.c
SERVIDOR_SRC = TP_Redes_Cliente.c
CLIENTE_OUT = TP_Redes_Cliente
SERVIDOR_OUT = TP_Redes_Servidor

SITE_DIR = site

TEST_URL = http://example.com/

all: $(CLIENTE_OUT) $(SERVIDOR_OUT)

$(CLIENTE_OUT): $(CLIENTE_SRC)
	$(CC) $(CFLAGS) $(CLIENTE_SRC) -o $(CLIENTE_OUT)

$(SERVIDOR_OUT): $(SERVIDOR_SRC)
	$(CC) $(CFLAGS) $(SERVIDOR_SRC) -o $(SERVIDOR_OUT)

run-server: $(SERVIDOR_OUT)
	@echo "🚀 Iniciando servidor HTTP na porta 8080..."
	@echo "Diretório: $(SITE_DIR)"
	@echo "Acesse via: http://<seu_IP>:8080/"
	@echo
	./$(SERVIDOR_OUT) $(SITE_DIR) $(ARGS)

run-client: $(CLIENTE_OUT)
	@echo "🌐 Executando cliente HTTP para: $(TEST_URL)"
	./$(CLIENTE_OUT) $(TEST_URL)

clean:
	rm -f $(CLIENTE_OUT) $(SERVIDOR_OUT)

rebuild: clean all
