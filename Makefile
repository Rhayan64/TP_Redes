# Compilador e flags
CC = gcc
CFLAGS = -Wall -O2

# Fontes e executáveis
CLIENTE_SRC = cliente_http.c
SERVIDOR_SRC = servidor_http.c
CLIENTE_OUT = cliente_http
SERVIDOR_OUT = servidor_http

# Diretório padrão do servidor
SITE_DIR = site

# URL padrão para teste do cliente
TEST_URL = http://example.com/

# Alvo padrão
all: $(CLIENTE_OUT) $(SERVIDOR_OUT)

# Compila o cliente HTTP
$(CLIENTE_OUT): $(CLIENTE_SRC)
	$(CC) $(CFLAGS) $(CLIENTE_SRC) -o $(CLIENTE_OUT)

# Compila o servidor HTTP
$(SERVIDOR_OUT): $(SERVIDOR_SRC)
	$(CC) $(CFLAGS) $(SERVIDOR_SRC) -o $(SERVIDOR_OUT)

# Executa o servidor HTTP
run-server: $(SERVIDOR_OUT)
	@echo "🚀 Iniciando servidor HTTP na porta 8080..."
	@echo "Diretório: $(SITE_DIR)"
	@echo "Acesse via: http://<seu_IP>:8080/"
	@echo
	./$(SERVIDOR_OUT) $(SITE_DIR)

# Executa o cliente HTTP
run-client: $(CLIENTE_OUT)
	@echo "🌐 Executando cliente HTTP para: $(TEST_URL)"
	./$(CLIENTE_OUT) $(TEST_URL)

# Remove os executáveis
clean:
	rm -f $(CLIENTE_OUT) $(SERVIDOR_OUT)

# Recompila tudo do zero
rebuild: clean all
