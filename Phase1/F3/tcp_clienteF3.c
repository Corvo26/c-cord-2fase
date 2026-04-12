#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define BUF_SIZE 4096

void erro(char *msg);

int main(int argc, char *argv[]) {
    int fd, n;
    struct sockaddr_in addr;
    struct hostent *hostPtr;
    char buffer_msg[BUF_SIZE];
    char escolha[10];
    char username[50], password[50];

    if (argc != 3) {
        printf("USO: ./cliente <host> <porto>\n");
        exit(-1);
    }

    if ((hostPtr = gethostbyname(argv[1])) == 0)
        erro("Nao consegui obter endereco");

    bzero((void *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
    addr.sin_port = htons((short) atoi(argv[2]));

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        erro("socket");

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        erro("Connect");

    /* ---- Recebe cabeçalho + menu até ao prompt "C-cord > " ---- */
    while ((n = read(fd, buffer_msg, BUF_SIZE - 1)) > 0) {
        buffer_msg[n] = '\0';
        printf("%s", buffer_msg);
        fflush(stdout);
        if (strstr(buffer_msg, "C-cord > ") != NULL)
            break;
    }

    /* ---- Envia escolha do menu ---- */
    if (fgets(escolha, sizeof(escolha), stdin) != NULL)
        write(fd, escolha, strlen(escolha));

    int opcao = atoi(escolha);

    /* ---- Fluxo de autenticação (login ou registo) ---- */
    /* Lê prompt "Username: " / "Novo Username: " */
    if ((n = read(fd, buffer_msg, BUF_SIZE - 1)) > 0) {
        buffer_msg[n] = '\0';
        printf("\n%s", buffer_msg);
        fgets(username, sizeof(username), stdin);
        write(fd, username, strlen(username));
    }

    /* Lê prompt "Password: " / "Nova Password: " */
    if ((n = read(fd, buffer_msg, BUF_SIZE - 1)) > 0) {
        buffer_msg[n] = '\0';
        printf("%s", buffer_msg);
        fgets(password, sizeof(password), stdin);
        write(fd, password, strlen(password));
    }

    /* ----------------------------------------------------------------
     * F3 – Loop de comandos (apenas após login, opção 1)
     * Após registo (opção 2) o servidor envia confirmação e fecha.
     * ---------------------------------------------------------------- */
    if (opcao == 1) {
        char cmd[BUF_SIZE];

        /* Lê mensagem de boas-vindas + primeiro prompt ">> " */
        while ((n = read(fd, buffer_msg, BUF_SIZE - 1)) > 0) {
            buffer_msg[n] = '\0';
            printf("%s", buffer_msg);
            fflush(stdout);
            if (strstr(buffer_msg, ">> ") != NULL)
                break;
        }

        /* Loop bloqueante: envia comando → recebe resposta */
        while (1) {
            /* Lê comando do utilizador */
            if (fgets(cmd, sizeof(cmd), stdin) == NULL)
                break;

            /* Envia para o servidor */
            write(fd, cmd, strlen(cmd));

            /* Remove '\n' apenas para comparar localmente */
            cmd[strcspn(cmd, "\n")] = '\0';

            /* Recebe resposta do servidor */
            n = read(fd, buffer_msg, BUF_SIZE - 1);
            if (n <= 0) break;
            buffer_msg[n] = '\0';
            printf("%s", buffer_msg);
            fflush(stdout);

            /* Sai do loop após QUIT */
            if (strcmp(cmd, "QUIT") == 0)
                break;
        }
    } else {
        /* Registo: lê confirmação final do servidor */
        if ((n = read(fd, buffer_msg, BUF_SIZE - 1)) > 0) {
            buffer_msg[n] = '\0';
            printf("%s", buffer_msg);
        }
    }

    close(fd);
    return 0;
}

void erro(char *msg) {
    perror(msg);
    exit(-1);
}