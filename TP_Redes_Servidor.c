#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>

#define PORTA 8080
#define BUFFER_SIZE 4096

void enviar_resposta(int cliente, const char *caminho) {
    struct stat st;
    if (stat(caminho, &st) == -1) {
        char *erro = "HTTP/1.0 404 Not Found\r\nContent-Type: text/plain\r\n\r\nArquivo não encontrado.";
        send(cliente, erro, strlen(erro), 0);
        return;
    }

    if (S_ISDIR(st.st_mode)) {
        // Caminho é diretório
        char index_path[512];
        snprintf(index_path, sizeof(index_path), "%s/index.html", caminho);
        if (stat(index_path, &st) == 0) {
            caminho = index_path; // Tem index.html
        } else {
            // Gera listagem do diretório
            DIR *dir = opendir(caminho);
            if (!dir) {
                char *erro = "HTTP/1.0 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\nErro ao abrir diretório.";
                send(cliente, erro, strlen(erro), 0);
                return;
            }

            char resposta[BUFFER_SIZE * 4];
            strcpy(resposta, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h2>Listagem de arquivos</h2><ul>");

            struct dirent *ent;
            while ((ent = readdir(dir)) != NULL) {
                if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                    continue;
                strcat(resposta, "<li><a href=\"");
                strcat(resposta, ent->d_name);
                strcat(resposta, "\">");
                strcat(resposta, ent->d_name);
                strcat(resposta, "</a></li>");
            }
            closedir(dir);
            strcat(resposta, "</ul></body></html>");
            send(cliente, resposta, strlen(resposta), 0);
            return;
        }
    }

    // Envia o arquivo
    FILE *fp = fopen(caminho, "rb");
    if (!fp) {
        char *erro = "HTTP/1.0 403 Forbidden\r\nContent-Type: text/plain\r\n\r\nNão foi possível abrir o arquivo.";
        send(cliente, erro, strlen(erro), 0);
        return;
    }

    char cabecalho[] = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n";
    send(cliente, cabecalho, strlen(cabecalho), 0);

    char buffer[BUFFER_SIZE];
    size_t lidos;
    while ((lidos = fread(buffer, 1, sizeof(buffer), fp)) > 0)
        send(cliente, buffer, lidos, 0);

    fclose(fp);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <diretorio> <ip_para_escutar>\n", argv[0]);
        printf("Exemplo: %s site 192.168.0.10\n", argv[0]);
        return 1;
    }

    const char *diretorio_base = argv[1];
    const char *ip = argv[2];

    chdir(diretorio_base); // muda para o diretório base

    int servidor = socket(AF_INET, SOCK_STREAM, 0);
    if (servidor < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(PORTA);

    if (bind(servidor, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(servidor);
        return 1;
    }

    listen(servidor, 5);
    printf("Servidor rodando em http://%s:%d/\n", ip, PORTA);

    while (1) {
        int cliente = accept(servidor, NULL, NULL);
        if (cliente < 0) {
            perror("accept");
            continue;
        }

        char buffer[BUFFER_SIZE];
        int lidos = recv(cliente, buffer, sizeof(buffer) - 1, 0);
        if (lidos <= 0) {
            close(cliente);
            continue;
        }
        buffer[lidos] = '\0';

        // Lê o caminho solicitado
        char metodo[8], caminho[256];
        sscanf(buffer, "%s %s", metodo, caminho);

        // Remove o '/' inicial
        if (caminho[0] == '/')
            memmove(caminho, caminho + 1, strlen(caminho));

        if (strlen(caminho) == 0)
            strcpy(caminho, ".");

        enviar_resposta(cliente, caminho);
        close(cliente);
    }

    close(servidor);
    return 0;
}