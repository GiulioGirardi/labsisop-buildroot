#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// buffer e ponteiro global
char* buffer;
char* buffer_ptr;
int buf_size;

// barreira para sincronizar o inicio das threads
pthread_barrier_t barrier;

// semaforo para sincronizar o acesso ao buffer
sem_t sem;

// array para contar o numero de caracteres
int counts[26] = {0};
// array para contar as mudanças de cada caractere
int changes[26] = {0};

// coloca o caracter no buffer
void* printaNoBuffer(void* args) {
    char thread_char = *(char*)args;

    for (;;) {
        sem_wait(&sem); // espera pelo semaforo

        // verifica se o buffer eh nulo
        if (buffer_ptr - buffer >= buf_size) {
            sem_post(&sem); // da o sinal para o semaforo e finaliza a thread
            break;
        }

        // Escreve um caractere e incrementa o ponteiro do buffer
        *buffer_ptr = thread_char;
        buffer_ptr++;

        // sinal para o semaforo
        sem_post(&sem);

        usleep(100000); // 100 ms delay        
    }
    return NULL;
}

// Define a funcao que a thread vai executar
void* printaCharacter(void* arg) {
    char thread_char = *(char*)arg;
    // Threads esperam na
    pthread_barrier_wait(&barrier);

    // chama a funcao printaNoBuffer com o argumento
    printaNoBuffer(arg);
    free(arg);  // desaloca a memoria
    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <buffer_size> <num_threads>\n", argv[0]);
        return 1;
    }

    buf_size = atoi(argv[1]);
    int num_threads = atoi(argv[2]);

    if (num_threads > 26) {
        fprintf(stderr, "Number of threads should not exceed 26\n");
        return 1;
    }

    // Aloca o buffer
    buffer = malloc(buf_size * sizeof(char));
    if (buffer == NULL) {
        perror("Failed to allocate buffer");
        return 1;
    }
    buffer_ptr = buffer;

    // inicializa semaforo
    if (sem_init(&sem, 0, 1) != 0) {
        perror("Failed to initialize semaphore");
        return 1;
    }

    // inicializa barreira para sincronizar as threads
    if (pthread_barrier_init(&barrier, NULL, num_threads) != 0) {
        perror("Failed to initialize barrier");
        return 1;
    }

    // array para as threads
    pthread_t threads[num_threads];

    // cria as threads
    for (int i = 0; i < num_threads; i++) {
        // aloca memoria
        char* thread_char = malloc(sizeof(char));
        if (thread_char == NULL) {
            perror("Failed to allocate memory");
            return 1;
        }
        // cria thread
        *thread_char = 'A' + i;
        if (pthread_create(&threads[i], NULL, printaCharacter, thread_char) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }

    // espera todas as threads finalizarem
    for (int i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Failed to join thread");
            return 1;
        }
    }

    // destroi semaforo
    if (sem_destroy(&sem) != 0) {
        perror("Failed to destroy semaphore");
        return 1;
    }

    // destroi barreira
    if (pthread_barrier_destroy(&barrier) != 0) {
        perror("Failed to destroy barrier");
        return 1;
    }

    // printa o buffer na tela apos a execucao
    for (int i = 0; i < buf_size; i++) {
        // printa na tela
        printf("%c", buffer[i]);

        // conta as ocorrencias de cada caractere
        if (buffer[i] >= 'A' && buffer[i] <= 'Z') counts[buffer[i] - 'A']++;

        // conta o numero de mudancas de caractere no buffer
        if (buffer[i] != buffer[i - 1]) changes[buffer[i] - 'A']++;
    }
    printf("\n");

    // printa numero de ocorrencias e mudancas
    for (int i = 0; i < 26; i++) {
        if (counts[i] > 0) printf("%c = %d (mudancas: %d)\n", 'A' + i, counts[i], changes[i]);
    }

    // limpa o buffer
    free(buffer);
    return 0;
}