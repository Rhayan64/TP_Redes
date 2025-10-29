#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <url> [arquivo_saida]\n", argv[0]);
        return 1;
    }

    char *url = argv[1];
    char *arquivo_saida = (argc >= 3) ? argv[2] : "saida.html";

    // Verifica se começa com http://
    if (strncmp(url, "http://", 7) != 0) {
        fprintf(stderr, "Apenas URLs HTTP são suportadas (ex: http://example.com)\n");
        return 1;
    }

    // Remove "http://"
    url += 7;

    // Separa host e caminho
    char host[256], path[1024];
    if (sscanf(url, "%255[^/]/%1023[^\n]", host, path) < 2)
        strcpy(path, "");

    if (strlen(path) == 0)
        strcpy(path, "");

    // Conecta ao servidor
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, "80", &hints, &res) != 0) {
        perror("getaddrinfo");
        return 1;
    }

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        perror("socket");
        freeaddrinfo(res);
        return 1;
    }

    if (connect(sock, res->ai_addr, res->ai_addrlen) != 0) {
        perror("connect");
        freeaddrinfo(res);
        close(sock);
        return 1;
    }
    freeaddrinfo(res);

    // Monta a requisição HTTP
    char req[2048];
    snprintf(req, sizeof(req),
             "GET /%s HTTP/1.0\r\n"
             "Host: %s\r\n"
             "User-Agent: cliente_http_c/1.0\r\n"
             "Connection: close\r\n\r\n",
             path, host);

    // Envia a requisição
    send(sock, req, strlen(req), 0);

    // Abre o arquivo de saída
    FILE *fp = fopen(arquivo_saida, "wb");
    if (!fp) {
        perror("fopen");
        close(sock);
        return 1;
    }

    // Lê a resposta e grava apenas o corpo (após o cabeçalho)
    char buffer[4096];
    int in_body = 0;
    ssize_t n;

    while ((n = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        // Procura pelo fim dos cabeçalhos (\r\n\r\n)
        if (!in_body) {
            char *body_start = strstr(buffer, "\r\n\r\n");
            if (body_start) {
                in_body = 1;
                body_start += 4; // pula cabeçalho
                fwrite(body_start, 1, n - (body_start - buffer), fp);
            }
        } else {
            fwrite(buffer, 1, n, fp);
        }
    }

    fclose(fp);
    close(sock);

    printf("Arquivo salvo em: %s\n", arquivo_saida);
    return 0;
}