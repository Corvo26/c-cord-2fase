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
    char escolha[BUF_SIZE];
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

    // --- CICLO PRINCIPAL ---
    while (1) {
        int saltar_input = 0;
        
        memset(buffer_msg, 0, BUF_SIZE);

        // 1. Espera pelo Menu ou Prompt
        // Correcção da lógica de leitura para evitar deadlock pós-QUIT
        while ((n = read(fd, buffer_msg, BUF_SIZE - 1)) > 0) {
            buffer_msg[n] = '\0';
            printf("%s", buffer_msg);
            fflush(stdout);

            // Se a sessão terminou, verificamos se o menu veio no mesmo pacote
            if (strstr(buffer_msg, "Sessao terminada") != NULL) {
                if (strstr(buffer_msg, "C-cord > ") == NULL) {
                    saltar_input = 1;
                    break;
                }
            }

            // Se encontrar um dos prompts de entrada, sai do loop de leitura
            if (strstr(buffer_msg, "C-cord > ") != NULL || strstr(buffer_msg, ">> ") != NULL) {
                break;
            }

            if (strstr(buffer_msg, "A desligar") != NULL || strstr(buffer_msg, "Adeus") != NULL) {
                goto fim;
            }
        }

        if (n <= 0) break;

        if (saltar_input) {
            continue; 
        }

        // 2. Lê comando do teclado e envia ao servidor
        if (fgets(escolha, sizeof(escolha), stdin) == NULL) break;
        write(fd, escolha, strlen(escolha));

        int opcao = atoi(escolha);

        // 3. Lógica específica para Login (1) ou Registo (2)
        if (opcao == 1 || opcao == 2) {
            // Espera pelo prompt "Username: " ou "Novo Username: "
            n = read(fd, buffer_msg, BUF_SIZE - 1);
            if (n > 0){
                buffer_msg[n] = '\0';
                printf("%s", buffer_msg);
                fflush(stdout);
                
                fgets(username, sizeof(username), stdin);
                write(fd, username, strlen(username));
    
                // Espera pelo prompt "Password: " ou "Nova Password: "
                n = read(fd, buffer_msg, BUF_SIZE - 1);
                if (n > 0) {
                    buffer_msg[n] = '\0';
                    printf("%s", buffer_msg);
                    fflush(stdout);
                    
                    fgets(password, sizeof(password), stdin);
                    write(fd, password, strlen(password));
                }
            }
        }
    }

fim:
    close(fd);
    printf("\nCliente encerrado.\n");
    return 0;
}

void erro(char *msg) {
    perror(msg);
    exit(-1);
}
