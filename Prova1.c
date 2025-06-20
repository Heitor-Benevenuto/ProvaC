#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 

#define MAX_LINE_SIZE 256
#define MAX_SENSOR_ID_SIZE 50
#define MAX_STRING_VALUE_SIZE 17 
#define INITIAL_SENSOR_CAPACITY 10
#define INITIAL_READING_CAPACITY 100

typedef enum {
    INTEIRO,
    BOOLEANO,
    REAL,
    STRING
} TipoDado;

typedef union {
    long long inteiro;
    bool booleano;
    double real;
    char string[MAX_STRING_VALUE_SIZE];
} ValorLeitura;

typedef struct {
    long long timestamp;
    char id_sensor[MAX_SENSOR_ID_SIZE];
    ValorLeitura valor;
    TipoDado tipo_dado; 
} Leitura;

typedef struct {
    char id[MAX_SENSOR_ID_SIZE];
    TipoDado tipo_dado;
    Leitura *leituras;
    int num_leituras;
    int capacidade_leituras; 
} SensorInfo;


int comparar_leituras(const void *a, const void *b) {
    Leitura *la = (Leitura *)a;
    Leitura *lb = (Leitura *)b;
    if (la->timestamp > lb->timestamp) return -1;
    if (la->timestamp < lb->timestamp) return 1;
    return 0;
}

TipoDado inferir_tipo_dado(const char *valor_str) {
    if (strcmp(valor_str, "true") == 0 || strcmp(valor_str, "false") == 0) {
        return BOOLEANO;
    }

    if (strchr(valor_str, '.') != NULL) {
        char *endptr;
        strtod(valor_str, &endptr);
        if (*endptr == '\0') { 
            return REAL;
        }
    }

    char *endptr_ll;
    strtoll(valor_str, &endptr_ll, 10);
    if (*endptr_ll == '\0') { 
        return INTEIRO;
    }

    return STRING;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <nome_arquivo_entrada>\n", argv[0]);
        return 1;
    }

    FILE *arquivo_entrada;
    char linha[MAX_LINE_SIZE];
    char valor_str[MAX_LINE_SIZE]; 

    SensorInfo *sensores = NULL;
    int num_sensores = 0;
    int capacidade_sensores = INITIAL_SENSOR_CAPACITY;

    sensores = (SensorInfo *)malloc(capacidade_sensores * sizeof(SensorInfo));
    if (sensores == NULL) {
        perror("Erro ao alocar memÃ³ria para sensores");
        return 1;
    }

    arquivo_entrada = fopen(argv[1], "r");
    if (arquivo_entrada == NULL) {
        perror("Erro ao abrir o arquivo de entrada");
        free(sensores);
        return 1;
    }

    while (fgets(linha, MAX_LINE_SIZE, arquivo_entrada) != NULL) {
        Leitura leitura;

        if (sscanf(linha, "%lld %s %s", &leitura.timestamp, leitura.id_sensor, valor_str) == 3) {
            int sensor_index = -1;
            for (int i = 0; i < num_sensores; i++) {
                if (strcmp(sensores[i].id, leitura.id_sensor) == 0) {
                    sensor_index = i;
                    break;
                }
            }

            if (sensor_index == -1) {
                if (num_sensores == capacidade_sensores) {
                    capacidade_sensores *= 2;
                    SensorInfo *temp_sensores = (SensorInfo *)realloc(sensores, capacidade_sensores * sizeof(SensorInfo));
                    if (temp_sensores == NULL) {
                        perror("Erro ao realocar memÃ³ria para sensores");
                        for (int i = 0; i < num_sensores; i++) {
                            free(sensores[i].leituras);
                        }
                        free(sensores);
                        fclose(arquivo_entrada);
                        return 1;
                    }
                    sensores = temp_sensores;
                }

                sensor_index = num_sensores++;
                strncpy(sensores[sensor_index].id, leitura.id_sensor, MAX_SENSOR_ID_SIZE - 1);
                sensores[sensor_index].id[MAX_SENSOR_ID_SIZE - 1] = '\0';
                sensores[sensor_index].num_leituras = 0;
                sensores[sensor_index].capacidade_leituras = INITIAL_READING_CAPACITY;
                sensores[sensor_index].leituras = (Leitura *)malloc(INITIAL_READING_CAPACITY * sizeof(Leitura));
                if (sensores[sensor_index].leituras == NULL) {
                    perror("Erro ao alocar memÃ³ria inicial para leituras do sensor");
                    for (int i = 0; i < num_sensores - 1; i++) {
                        free(sensores[i].leituras);
                    }
                    free(sensores);
                    fclose(arquivo_entrada);
                    return 1;
                }

                sensores[sensor_index].tipo_dado = inferir_tipo_dado(valor_str);
                printf("Novo sensor '%s' detectado com tipo de dado: ", sensores[sensor_index].id);
                switch (sensores[sensor_index].tipo_dado) {
                    case INTEIRO: printf("INTEIRO\n"); break;
                    case BOOLEANO: printf("BOOLEANO\n"); break;
                    case REAL: printf("REAL\n"); break;
                    case STRING: printf("STRING\n"); break;
                }
            }

            leitura.tipo_dado = sensores[sensor_index].tipo_dado;

            switch (leitura.tipo_dado) {
                case INTEIRO:
                    leitura.valor.inteiro = atoll(valor_str);
                    break;
                case BOOLEANO:
                    leitura.valor.booleano = (strcmp(valor_str, "true") == 0);
                    break;
                case REAL:
                    leitura.valor.real = atof(valor_str);
                    break;
                case STRING:
                    strncpy(leitura.valor.string, valor_str, MAX_STRING_VALUE_SIZE - 1);
                    leitura.valor.string[MAX_STRING_VALUE_SIZE - 1] = '\0';
                    break;
            }

            if (sensores[sensor_index].num_leituras == sensores[sensor_index].capacidade_leituras) {
                sensores[sensor_index].capacidade_leituras *= 2;
                Leitura *temp_leituras = (Leitura *)realloc(sensores[sensor_index].leituras, sensores[sensor_index].capacidade_leituras * sizeof(Leitura));
                if (temp_leituras == NULL) {
                    perror("Erro ao realocar memÃ³ria para leituras do sensor");
                    for (int i = 0; i < num_sensores; i++) {
                        free(sensores[i].leituras);
                    }
                    free(sensores);
                    fclose(arquivo_entrada);
                    return 1;
                }
                sensores[sensor_index].leituras = temp_leituras;
            }

            sensores[sensor_index].leituras[sensores[sensor_index].num_leituras++] = leitura;

        } else {
            fprintf(stderr, "Aviso: Linha mal formatada ignorada: %s", linha);
        }
    }

    fclose(arquivo_entrada);

    for (int i = 0; i < num_sensores; i++) {
        qsort(sensores[i].leituras, sensores[i].num_leituras, sizeof(Leitura), comparar_leituras);

        char nome_arquivo_saida[MAX_SENSOR_ID_SIZE + 5];
        sprintf(nome_arquivo_saida, "%s.txt", sensores[i].id);

        FILE *arquivo_saida = fopen(nome_arquivo_saida, "w");
        if (arquivo_saida == NULL) {
            perror("Erro ao abrir arquivo de saÃ­da");
            for (int j = 0; j < num_sensores; j++) {
                free(sensores[j].leituras);
            }
            free(sensores);
            return 1;
        }

        for (int j = 0; j < sensores[i].num_leituras; j++) {
            fprintf(arquivo_saida, "%lld %s ", sensores[i].leituras[j].timestamp, sensores[i].leituras[j].id_sensor);

            switch (sensores[i].leituras[j].tipo_dado) {
                case INTEIRO:
                    fprintf(arquivo_saida, "%lld\n", sensores[i].leituras[j].valor.inteiro);
                    break;
                case BOOLEANO:
                    fprintf(arquivo_saida, "%s\n", sensores[i].leituras[j].valor.booleano ? "true" : "false");
                    break;
                case REAL:
                    fprintf(arquivo_saida, "%.2f\n", sensores[i].leituras[j].valor.real);
                    break;
                case STRING:
                    fprintf(arquivo_saida, "%s\n", sensores[i].leituras[j].valor.string);
                    break;
            }
        }

        fclose(arquivo_saida);
        printf("Dados do sensor '%s' salvos em '%s'\n", sensores[i].id, nome_arquivo_saida);
        free(sensores[i].leituras);
    }

    free(sensores);
    printf("Processamento concluÃ­do.\n");
    return 0;
}
