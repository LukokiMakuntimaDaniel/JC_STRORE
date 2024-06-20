#include "structures.h" // Seu arquivo de definição de estruturas
#include "JCSTORE_FUNCTIONS.h" // Seu arquivo de protótipos de funções
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>



int main() {
    // Inicialização do JCStore
    if (jcstore_init(NUM_SERVER_THREADS, BUFFER_SIZE, NUM_SHARDS) != 0) {
        fprintf(stderr, "Erro ao inicializar o JCStore.\n");
        exit(EXIT_FAILURE);
    }

    // Criação de threads cliente
    pthread_t client_threads[NUM_CLIENTS];
    for (int i = 0; i < NUM_CLIENTS; i++) {
        if (pthread_create(&client_threads[i], NULL, client_thread, &i) != 0) {
            fprintf(stderr, "Erro ao criar a thread cliente %d.\n", i);
            exit(EXIT_FAILURE);
        }
    }

    // Criação de threads servidoras
    pthread_t server_threads[NUM_SERVER_THREADS];
    for (int i = 0; i < NUM_SERVER_THREADS; i++) {
        if (pthread_create(&server_threads[i], NULL, serverProcess, NULL) != 0) {
            fprintf(stderr, "Erro ao criar a thread servidora %d.\n", i);
            exit(EXIT_FAILURE);
        }
    }

    // Aguarda o término das threads cliente
    for (int i = 0; i < NUM_CLIENTS; i++) {
        pthread_join(client_threads[i], NULL);
    }

    // Aguarda o término das threads servidoras
    for (int i = 0; i < NUM_SERVER_THREADS; i++) {
        pthread_join(server_threads[i], NULL);
    }

    return 0;
}