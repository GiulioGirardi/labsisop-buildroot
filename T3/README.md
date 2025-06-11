# Programa de Buffer com Threads

Este programa cria múltiplas threads, cada uma escrevendo um caractere único em um buffer compartilhado. O tamanho do buffer e o número de threads são especificados pelo usuário. O programa garante que as threads escrevam no buffer de maneira sincronizada usando semáforos e barreiras.

## Compilação
Para compilar o programa utilize o seguinte comando: <br>
`gcc sched_profiler.c -o sched_profiler -pthread` 

## Uso
`sched_profiler <tamanho_buffer> <num_threads>`

- `<tamanho_buffer>`: O tamanho do buffer a ser preenchido com caracteres.
- `<num_threads>`: O número de threads a serem criadas (máximo de 26).

Para simular mais de um core no simulador QEMU usamos o comando: `-smp cores = 2` no final do script _./qemu-up.sh_, onde 2 é o número de cores.
Para verificar o número de cores utilizamos o comando dentro do QEMU:  `cat proc/cpuinfo`.

Temos o seguinte resultado: <br>
processor       : 0
vendor_id       : GenuineIntel
cpu family      : 6
model           : 6
model name      : QEMU Virtual CPU version 2.5+
stepping        : 3
cpu MHz         : 2445.291
cache size      : 16384 KB
physical id     : 0
siblings        : 2
core id         : 0
cpu cores       : 2
apicid          : 0
initial apicid  : 0
fdiv_bug        : no
f00f_bug        : no
coma_bug        : no
fpu             : yes
fpu_exception   : yes
cpuid level     : 4
wp              : yes
flags           : fpu de pse tsc msr pae mce cx8 apic sep pge cmov mmx fxsr sse sse2 ht cpuid pni hypervisor
bugs            :
bogomips        : 4890.58
clflush size    : 32
cache_alignment : 32
address sizes   : 36 bits physical, 32 bits virtual
power management:

processor       : 1
vendor_id       : GenuineIntel
cpu family      : 6
model           : 6
model name      : QEMU Virtual CPU version 2.5+
stepping        : 3
cpu MHz         : 2445.291
cache size      : 16384 KB
physical id     : 0
siblings        : 2
core id         : 1
cpu cores       : 2
apicid          : 1
initial apicid  : 1
fdiv_bug        : no
f00f_bug        : no
coma_bug        : no
fpu             : yes
fpu_exception   : yes
cpuid level     : 4
wp              : yes
flags           : fpu de pse tsc msr pae mce cx8 apic sep pge cmov mmx fxsr sse sse2 ht cpuid pni hypervisor
bugs            :
bogomips        : 4890.94
clflush size    : 32
cache_alignment : 32
address sizes   : 36 bits physical, 32 bits virtual
power management:

Onde podemos ver os 2 cores descritos no script _./qemu-up.sh_.

## Descrição do Programa
1. Alocação do Buffer: Um buffer do tamanho especificado por parâmetro é alocado.
2. Inicialização do Semáforo: Um semáforo é inicializado para sincronizar o acesso ao buffer.
3. Inicialização da Barreira: Uma barreira é inicializada para sincronizar o início de todas as threads.
4. Criação das Threads: O número especificado por parâmetro de threads é criado. Cada thread escreve seu caractere único (A, B, C, etc.) no buffer.
5. Execução das Threads:
* Cada thread espera na barreira.
* Uma vez que todas as threads chegam à barreira, elas começam a escrever no buffer.
* Cada thread escreve seu caractere no buffer, garantindo o acesso sincronizado usando o semáforo.
6. Junção das Threads: A thread principal espera que todas as threads completem sua execução.
7. Destruição do Semáforo e da Barreira: O semáforo e a barreira são destruídos após a conclusão das threads.
8. Saída do Buffer: O conteúdo do buffer é impresso.
9. Contagem de Caracteres e Mudanças: O programa conta e imprime o número de ocorrências de cada caractere e o número de mudanças entre caracteres no buffer.

## Exemplos
Executando o programa com um tamanho de buffer de 128 e 4 threads usando __1__ core no simulador QEMU:<br>
`sched_profiler 128 4`

Saída:
DCBACDBACDBACDBACDBACDBACDBACDBACDBACDBACDBACDBACDBACDBACDBACDBACDBACDBACDBACDBADCBADCBACDBADCBACDBADCBACDBADCBACDBACDBADCBACDBA<br>
A = 32 (mudancas: 32)<br>
B = 32 (mudancas: 32)<br>
C = 32 (mudancas: 32)<br>
D = 32 (mudancas: 32)


Executando o programa com um tamanho de buffer de 128 e 4 threads usando __2__ cores no simulador QEMU:<br>
`sched_profiler 128 4`

Saída:
ACDBBCABDCDADBBCABDDCADBBCABDDCADBBCABDDCADBBCABDDCADBBCABDDCADBBCDABDCBABDDCADBBCABDCDADBBCABDCDADBBCABDDCADBBCABDCDADBCBABDCDA<br>
A = 26 (mudancas: 26)<br>
B = 38 (mudancas: 27)<br>
C = 26 (mudancas: 26)<br>
D = 38 (mudancas: 31)