#include "structures.h"
#include "JCSTORE_FUNCTIONS.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

 // Número de threads clientes e servidoras

Buffer *buffer; // Buffer compartilhado entre threads cliente e servidor
HashTable shards[HT_SIZE]; // Tabelas de dispersão para as partições do JCStore

int hash(char *key) {
    int i = 0;
    if (key == NULL)
        return -1;
    while (*key != '\0') {
        i += (int)*key;
        key++;
    }
    return i % HT_SIZE;
}

char *jcstore_get(int clientid, int shardId, char *key) {
    if (key == NULL || shardId < 0 || shardId >= HT_SIZE) {
        return NULL;
    }

    int index = hash(key);
    Node *current = shards[shardId].buckets[index];

    while (current != NULL) {
        if (strcmp(current->kv.key, key) == 0) {
            return current->kv.value;
        }
        current = current->next;
    }

    return NULL;
}

char *jcstore_put(int clientid, int shardId, char *key, char *value) {
    if (key == NULL || value == NULL || shardId < 0 || shardId >= HT_SIZE) {
        return NULL;
    }

    int index = hash(key);
    Node *current = shards[shardId].buckets[index];
    Node *prev = NULL;

    while (current != NULL) {
        if (strcmp(current->kv.key, key) == 0) {
            char *old_value = strdup(current->kv.value);
            strcpy(current->kv.value, value);
            return old_value;
        }
        prev = current;
        current = current->next;
    }

    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node == NULL) {
        return NULL;
    }
    strcpy(new_node->kv.key, key);
    strcpy(new_node->kv.value, value);
    new_node->next = NULL;

    if (prev != NULL) {
        prev->next = new_node;
    } else {
        shards[shardId].buckets[index] = new_node;
    }

    return NULL;
}

char *jcstore_remove(int clientid, int shardId, char *key) {
    if (key == NULL || shardId < 0 || shardId >= HT_SIZE) {
        return NULL;
    }

    int index = hash(key);
    Node *current = shards[shardId].buckets[index];
    Node *prev = NULL;

    while (current != NULL) {
        if (strcmp(current->kv.key, key) == 0) {
            if (prev != NULL) {
                prev->next = current->next;
            } else {
                shards[shardId].buckets[index] = current->next;
            }
            char *removed_value = strdup(current->kv.value);
            free(current);
            return removed_value;
        }
        prev = current;
        current = current->next;
    }

    return NULL;
}

KV_t *jcstore_getAllKeys(int clientid, int shardId, int *dim) {
    if (shardId < 0 || shardId >= HT_SIZE || dim == NULL) {
        *dim = -1;
        return NULL;
    }

    Node *current;
    int count = 0;

    for (int i = 0; i < HT_SIZE; i++) {
        current = shards[shardId].buckets[i];
        while (current != NULL) {
            count++;
            current = current->next;
        }
    }

    KV_t *key_value_array = (KV_t *)malloc(count * sizeof(KV_t));
    if (key_value_array == NULL) {
        *dim = -1;
        return NULL;
    }

    int index = 0;
    for (int i = 0; i < HT_SIZE; i++) {
        current = shards[shardId].buckets[i];
        while (current != NULL) {
            strcpy(key_value_array[index].key, current->kv.key);
            strcpy(key_value_array[index].value, current->kv.value);
            index++;
            current = current->next;
        }
    }

    *dim = count;
    return key_value_array;
}

int jcstore_init(int num_server_threads, int buf_size, int num_shards) {
    // Aloca memória para as partições do JCStore
    for (int i = 0; i < HT_SIZE; i++) {
        for (int j = 0; j < num_shards; j++) {
            shards[i].buckets[j] = NULL;
        }
    }

    // Inicializa o buffer compartilhado
    buffer = (Buffer *)malloc(sizeof(Buffer));
    if (buffer == NULL) {
        return -1;
    }
    buffer->clientId = 0;
    buffer->shardId = 0;
    buffer->operation = '\0';
    buffer->ready = 0;
    if (pthread_mutex_init(&(buffer->mutex), NULL) != 0) {
        free(buffer);
        return -1;
    }
    if (pthread_cond_init(&(buffer->cond), NULL) != 0) {
        pthread_mutex_destroy(&(buffer->mutex));
        free(buffer);
        return -1;
    }

    return 0; // Retorna 0 para indicar que a inicialização foi bem-sucedida
}


void *client_thread(void *arg) {
    int client_id = *((int *)arg);
    printf("Cliente %d iniciado.\n", client_id);

    while (1) {
        // Simulação de escolha de operação pelo cliente
        char operation;
        printf("\nCliente %d: Digite a operação desejada \ng para GET\np para PUT\nr para REMOVE\n: ", client_id);
        scanf("%c", &operation);

        char key[KV_SIZE];
        char value[KV_SIZE];

        switch (operation) {
            case 'g':
                printf("Cliente %d: Digite a chave para a operação GET:\n ", client_id);
                scanf("%s", key);
                break;
            case 'p':
                printf("Cliente %d: Digite a chave para a operação PUT: \n", client_id);
                scanf("%s", key);
                printf("Cliente %d: Digite o valor para a chave %s: \n", client_id, key);
                scanf("%s", value);
                break;
            case 'r':
                printf("Cliente %d: Digite a chave para a operação REMOVE: \n", client_id);
                scanf("%s", key);
                break;
            default:
                printf("Cliente %d: Operação inválida.\n", client_id);
                continue;
        }

        insert_request(client_id, hash(key), key, operation, value);

        sleep(20); // Simulação de tempo entre operações do cliente
    }

    pthread_exit(NULL);
}

int insert_request(int clientId, int shardId, char *key, char operation, char *value) {
    // Trava o mutex para garantir exclusão mútua ao acessar o buffer
    pthread_mutex_lock(&(buffer->mutex));

    // Verifica se o buffer está cheio
    while (buffer->ready == 1) {
        // Aguarda até que o buffer esteja disponível (não esteja cheio)
        pthread_cond_wait(&(buffer->cond), &(buffer->mutex));
    }

    // Insere os dados da solicitação no buffer
    buffer->clientId = clientId;
    buffer->shardId = shardId;
    strcpy(buffer->key, key);
    buffer->operation = operation;
    if (value != NULL) {
        strcpy(buffer->value, value);
    } else {
        strcpy(buffer->value, "");
    }
    buffer->ready = 1; // Indica que o buffer está pronto para ser processado

    // Sinaliza que o buffer está pronto para ser processado
    pthread_cond_signal(&(buffer->cond));

    // Libera o mutex para permitir que outros clientes insiram solicitações
    pthread_mutex_unlock(&(buffer->mutex));

    return 0; // Retorno de sucesso
}

void *serverProcess(void *arg) {
    printf("Thread do servidor iniciada.\n");

    while (1) {
        pthread_mutex_lock(&(buffer->mutex));
        while (buffer->ready == 0) {
            pthread_cond_wait(&(buffer->cond), &(buffer->mutex));
        }

        int client_id = buffer->clientId;
        int shard_id = buffer->shardId;
        char operation = buffer->operation;
        char *key = strdup(buffer->key);
        char *value = strdup(buffer->value);

        pthread_mutex_unlock(&(buffer->mutex));

        switch (operation) {
            case 'g':
                printf("Servidor: Recebida operação GET do cliente %d para a chave %s.\n", client_id, key);
                char *result = jcstore_get(client_id, shard_id, key);
                printf("Servidor: Enviando resultado %s para o cliente %d.\n", result != NULL ? result : "NULL", client_id);
                break;
            case 'p':
                printf("Servidor: Recebida operação PUT do cliente %d para a chave %s e valor %s.\n", client_id, key, value);
                char *old_value = jcstore_put(client_id, shard_id, key, value);
                printf("Servidor: Chave %s atualizada com valor %s (valor anterior: %s).\n", key, value, old_value != NULL ? old_value : "NULL");
                break;
            case 'r':
                printf("Servidor: Recebida operação REMOVE do cliente %d para a chave %s.\n", client_id, key);
                char *removed_value = jcstore_remove(client_id, shard_id, key);
                printf("Servidor: Chave %s removida (valor removido: %s).\n", key, removed_value != NULL ? removed_value : "NULL");
                break;
        }
        free(key);
        free(value);
        pthread_mutex_lock(&(buffer->mutex));
        buffer->ready = 0;
        pthread_mutex_unlock(&(buffer->mutex));
    }

    pthread_exit(NULL);
}