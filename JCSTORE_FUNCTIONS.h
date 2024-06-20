#ifndef JCSTORE_FUNCTIONS_H
#define JCSTORE_FUNCTIONS_H

#include "structures.h"

int insert_request(int clientId, int shardId, char *key, char operation, char *value);
void *client_thread(void *arg);
int jcstore_init(int num_server_threads, int buf_size, int num_shards);
char *jcstore_get(int clientid, int shardId, char *key);
char *jcstore_put(int clientid, int shardId, char *key, char *value);
char *jcstore_remove(int clientid, int shardId, char *key);
KV_t *jcstore_getAllKeys(int clientid, int shardId, int *dim);

// Protótipo de função para a função de dispersão (hash)
int hash(char *key);

void *client_thread(void *arg);
// Protótipo de função para o processo do servidor
void *serverProcess(void *arg);


#endif /* JCSTORE_FUNCTIONS_H */