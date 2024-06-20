#include <pthread.h>
#ifndef STRUCTURES_H
#define STRUCTURES_H


#define KV_SIZE 256  // Tamanho fixo para chave e valor
#define HT_SIZE 10 

#define NUM_THREADS 4  // Número de threads clientes e servidoras

#define NUM_CLIENTS 5
#define NUM_SERVER_THREADS 5
#define BUFFER_SIZE 10
#define NUM_SHARDS 10


typedef struct {
    int clientId;
    int shardId;
    char key[KV_SIZE];
    char value[KV_SIZE];
    char operation;  // 'g' para get, 'p' para put, 'r' para remove
    char result[KV_SIZE];
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int ready;
} Buffer;

typedef struct {
    char key[KV_SIZE];
    char value[KV_SIZE];
} KV_t;

typedef struct Node {
    KV_t kv;
    struct Node* next;
} Node;

 // Tamanho da tabela de dispersão
typedef struct {
    Node* buckets[HT_SIZE];
} HashTable;

#endif /* STRUCTURES_H */