#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h> 

#define MAX_SENSOR_ID_SIZE 50
#define MAX_STRING_VALUE_SIZE 17 
#define NUM_LEITURAS_POR_SENSOR 2000

typedef enum {
    INTEIRO,
    BOOLEANO,
    REAL,
    STRING,
    DESCONHECIDO 
} TipoDado;

typedef struct {
    char id[MAX_SENSOR_ID_SIZE];
    TipoDado tipo_dado;
} SensorConfig;


TipoDado string_to_tipodado(const char *tipo_str) {
    if (strcmp(tipo_str, "CONJ_Z") == 0) return INTEIRO;
    if (strcmp(tipo_str, "BINARIO") == 0) return BOOLEANO;
    if (strcmp(tipo_str, "CONJ_Q") == 0) return REAL;
    if (strcmp(tipo_str, "TEXTO") == 0) return STRING;
    return DESCONHECIDO;
}

long long datetime_to_epoch(int day, int month, int year, int hour, int min, int sec) {
    struct tm t = {0};
    t.tm_mday = day;
    t.tm_mon = month - 1;   
    t.tm_year = year - 1900; 
    t.tm_hour = hour;
    t.tm_min = min;
    t.tm_sec = sec;
    t.tm_isdst = -1; 
    return (long long)mktime(&t);
}

void generate_random_string(char *str, int length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (int i = 0; i < length; i++) {
        str[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    str[length] = '\0';
}

int main(int argc, char *argv[]) {
    
    if (argc < 12 + 2 || (argc - 13) % 2 != 0) {
        fprintf(stderr, "Uso: %s <dd_inicio> <mm_inicio> <aaaa_inicio> <hh_inicio> <min_inicio> <ss_inicio> \\\n", argv[0]);
        fprintf(stderr, "                <dd_fim> <mm_fim> <aaaa_fim> <hh_fim> <min_fim> <ss_fim> \\\n");
        fprintf(stderr, "                <ID_SENSOR1> <TIPO_SENSOR1> [<ID_SENSOR2> <TIPO_SENSOR2> ...]\n");
        fprintf(stderr, "Tipos de dados suportados: CONJ_Z, BINARIO, CONJ_Q, TEXTO\n");
        return 1;
    }

    int dia_inicio = atoi(argv[1]);
    int mes_inicio = atoi(argv[2]);
    int ano_inicio = atoi(argv[3]);
    int hora_inicio = atoi(argv[4]);
    int min_inicio = atoi(argv[5]);
    int sec_inicio = atoi(argv[6]);

    int dia_fim = atoi(argv[7]);
    int mes_fim = atoi(argv[8]);
    int ano_fim = atoi(argv[9]);
    int hora_fim = atoi(argv[10]);
    int min_fim = atoi(argv[11]);
    int sec_fim = atoi(argv[12]);

    long long inicio_intervalo = datetime_to_epoch(dia_inicio, mes_inicio, ano_inicio, hora_inicio, min_inicio, sec_inicio);
    long long fim_intervalo = datetime_to_epoch(dia_fim, mes_fim, ano_fim, hora_fim, min_fim, sec_fim);

    if (inicio_intervalo == -1 || fim_intervalo == -1 || inicio_intervalo > fim_intervalo) {
        perror("Erro ao converter data para timestamp ou intervalo invÃ¡lido");
        return 1;
    }

    int num_sensores = (argc - 13) / 2;
    SensorConfig *sensores = (SensorConfig *)malloc(num_sensores * sizeof(SensorConfig));
    if (sensores == NULL) {
        perror("Erro ao alocar memÃ³ria para configuraÃ§Ãµes de sensores");
        return 1;
    }

    for (int i = 0; i < num_sensores; i++) {
        strncpy(sensores[i].id, argv[13 + i * 2], MAX_SENSOR_ID_SIZE - 1);
        sensores[i].id[MAX_SENSOR_ID_SIZE - 1] = '\0';

        sensores[i].tipo_dado = string_to_tipodado(argv[13 + i * 2 + 1]);
        if (sensores[i].tipo_dado == DESCONHECIDO) {
            fprintf(stderr, "Erro: Tipo de dado desconhecido para o sensor '%s': '%s'\n",
                    sensores[i].id, argv[13 + i * 2 + 1]);
            free(sensores);
            return 1;
        }
    }

    FILE *arquivo_teste = fopen("leituras.txt", "w");
    if (arquivo_teste == NULL) {
        perror("Erro ao abrir arquivo de teste");
        free(sensores);
        return 1;
    }

    srand(time(NULL)); 

    for (int i = 0; i < num_sensores; i++) {
        printf("Gerando %d leituras para o sensor '%s' (Tipo: %s)...\n",
               NUM_LEITURAS_POR_SENSOR, sensores[i].id, argv[13 + i * 2 + 1]);

        for (int j = 0; j < NUM_LEITURAS_POR_SENSOR; j++) {
            long long timestamp_aleatorio = inicio_intervalo + (long long)(((double)rand() / RAND_MAX) * (fim_intervalo - inicio_intervalo + 1));
            fprintf(arquivo_teste, "%lld %s ", timestamp_aleatorio, sensores[i].id);

            switch (sensores[i].tipo_dado) {
                case INTEIRO:
                    fprintf(arquivo_teste, "%lld\n", (long long)rand() % 10000); 
                    break;
                case BOOLEANO:
                    fprintf(arquivo_teste, "%s\n", (rand() % 2 == 0) ? "true" : "false");
                    break;
                case REAL:
                    fprintf(arquivo_teste, "%.2f\n", (double)rand() / RAND_MAX * 1000.0); 
                    break;
                case STRING: {
                    char random_string[MAX_STRING_VALUE_SIZE];
                    generate_random_string(random_string, 15); 
                    fprintf(arquivo_teste, "%s\n", random_string);
                    break;
                }
                default: 
                    fprintf(stderr, "Erro: Tipo de dado desconhecido durante a geraÃ§Ã£o.\n");
                    break;
            }
        }
    }

    fclose(arquivo_teste);
    free(sensores);
    printf("Arquivo de teste 'leituras.txt' gerado com sucesso.\n");

    return 0;
}
