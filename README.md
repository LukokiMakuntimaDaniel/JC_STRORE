# JCStore - Sistema para Armazenamento de Dados

## Universidade Agostinho Neto
### Faculdade de Ciências
### Departamento de Ciências da Computação
#### Sistemas Operativos

## Índice
- [Introdução](#introdução)
- [API do JCStore](#api-do-jcstore)
- [Estrutura e Organização do Projeto](#estrutura-e-organização-do-projeto)
- [Desenho e Concretização da Solução](#desenho-e-concretização-da-solução)
  - [Tabelas de Dispersão](#tabelas-de-dispersão)
  - [Buffer para Comunicação](#buffer-para-comunicação)
  - [Sincronização](#sincronização)
- [Instalação](#instalação)
- [Como Contribuir](#como-contribuir)

## Introdução
O objetivo deste trabalho é desenvolver um sistema para o armazenamento de dados, denominado JCStore. O JCStore utiliza um modelo lógico de organização de dados chamado chave-valor, em que cada dado é associado a um par: uma “chave” que permite identificar univocamente o dado, e um “valor”.

Este tipo de sistema de armazenamento de dados é usado em vários contextos, como por exemplo no registro do Windows para salvar informações de configuração e meta-dados do sistema operacional.

Os dados salvos no JCStore são organizados em partições (shards), identificados por um identificador numérico inteiro (shardId). O “sharding” otimiza o desempenho das primitivas de acesso, permitindo paralelizar o acesso aos dados.

## API do JCStore
O JCStore suporta a seguinte API:

- **char* get(int clientId, int shardId, char* key)**: Retorna o valor associado à chave passada como parâmetro de entrada, caso exista no JCStore, NULL em caso contrário.
- **char* put(int clientId, int shardId, char* key, char* value)**: Insere um par `<Key, Value>` e devolve NULL se a chave não existir, ou, caso contrário, o valor anterior associado a essa chave.
- **char* remove(int clientId, int shardId, char* key)**: Apaga a chave identificada pela string key, devolvendo o valor da chave, caso exista, e NULL em caso contrário.
- **KV_t* jcstore_getAllKeys(int clientId, int shardId, int* sizeKVarray)**: Devolve um apontador para um vetor de pares `<Key, Value>`. O tamanho do vetor devolvido é escrito no parâmetro sizeKVarray.

## Estrutura e Organização do Projeto
O projeto é estruturado em duas partes:

### Parte I: Desenvolvimento da arquitetura base do JCStore
- Conceber e concretizar um esquema básico de sincronização entre tarefas clientes e servidores.
- Concretizar a tabela de dispersão usada para salvar os dados do JCStore em memória volátil (RAM).

### Parte II: Otimização e persistência dos dados
- Tornar os esquemas de sincronização mais eficientes.
- Desenvolver mecanismos para garantir a persistência dos dados salvos no JCStore através das APIs do sistema de arquivos UNIX.

## Desenho e Concretização da Solução

### Tabelas de Dispersão
Cada partição de dados do JCStore é suportada por uma tabela de dispersão (hashmap), concretizada através de um vetor de listas ligadas. O vetor tem um número de elementos fixos (HT_SIZE). A posição no vetor é determinada pela função de dispersão.

### Buffer para Comunicação
Para a comunicação entre tarefas clientes e servidores, utiliza-se um buffer partilhado em memória. Cada elemento do vetor é estaticamente associado a um par `<tarefa cliente i, tarefa servidora i>`.

### Sincronização
Nesta primeira fase, um único trinco ou semáforo é usado para sincronizar o acesso das tarefas servidoras aos dados salvos no JCStore. Na segunda fase, o desempenho será melhorado com esquemas de sincronização mais avançados.

## Instalação
Para instalar e executar o JCStore, siga os seguintes passos:

1. Clone o repositório:
    ```sh
    git clone [https://github.com/teuusuario/jcstore.git](https://github.com/LukokiMakuntimaDaniel/JC_STRORE.git)
    ```
2. Navegue até o diretório do projeto:
    ```sh
    cd jc_store
    ```
3. Execute o projeto:
    ```sh
    ./programa
    ```

## Como Contribuir
1. Faça um fork do projeto.
2. Crie uma branch para a sua feature:
    ```sh
    git checkout -b minha-feature
    ```
3. Commit suas alterações:
    ```sh
    git commit -m 'Adicionando minha feature'
    ```
4. Push para a branch:
    ```sh
    git push origin minha-feature
    ```
5. Abra um Pull Request.


