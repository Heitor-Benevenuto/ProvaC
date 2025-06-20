#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h> 
#include <math.h>    

#define MAX_LINE_SIZE 256
#define MAX_SENSOR_ID_SIZE 50
#define MAX_STRING_VALUE_SIZE 17 

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

// Converte data e hora para timestamp epoch
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

// Identifica o tipo do valor lido como string
TipoDado identificar_tipo_do_valor_string(const char *valor_str) {
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
    if (argc != 8) {
        fprintf(stderr, "Uso: %s <nome_sensor> <dd> <mm> <aaaa> <hh> <mm> <ss>\n", argv[0]);
        return 1;
    }

    char sensor_nome[MAX_SENSOR_ID_SIZE];
    strncpy(sensor_nome, argv[1], MAX_SENSOR_ID_SIZE - 1);
    sensor_nome[MAX_SENSOR_ID_SIZE - 1] = '\0';

    int day = atoi(argv[2]);
    int month = atoi(argv[3]);
    int year = atoi(argv[4]);
    int hour = atoi(argv[5]);
    int min = atoi(argv[6]);
    int sec = atoi(argv[7]);

    long long target_timestamp = datetime_to_epoch(day, month, year, hour, min, sec);

    char filename[MAX_SENSOR_ID_SIZE + 5]; 
    snprintf(filename, sizeof(filename), "%s.txt", sensor_nome);

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir o arquivo do sensor");
        fprintf(stderr, "Certifique-se de que o arquivo '%s' existe e foi gerado pelo Programa 1.\n", filename);
        return 1;
    }

    // Conta o nÃºmero de linhas (leituras)
    int count = 0;
    char temp_line[MAX_LINE_SIZE];
    while (fgets(temp_line, sizeof(temp_line), file)) {
        count++;
    }
    rewind(file);

    // Aloca memÃ³ria
    Leitura *readings = (Leitura *)malloc(count * sizeof(Leitura));
    if (readings == NULL) {
        perror("Erro ao alocar memÃ³ria para leituras");
        fclose(file);
        return 1;
    }

    // LÃª todas as linhas do arquivo
    for (int i = 0; i < count; i++) {
        char current_id_sensor[MAX_SENSOR_ID_SIZE];
        char valor_str[MAX_LINE_SIZE]; 

        if (fscanf(file, "%lld %s %s", &readings[i].timestamp, current_id_sensor, valor_str) != 3) {
            fprintf(stderr, "Erro de formato na linha %d do arquivo %s.txt\n", i + 1, sensor_nome);
            free(readings);
            fclose(file);
            return 1;
        }

        strncpy(readings[i].id_sensor, current_id_sensor, MAX_SENSOR_ID_SIZE - 1);
        readings[i].id_sensor[MAX_SENSOR_ID_SIZE - 1] = '\0';

        readings[i].tipo_dado = identificar_tipo_do_valor_string(valor_str);

        switch (readings[i].tipo_dado) {
            case INTEIRO:
                readings[i].valor.inteiro = atoll(valor_str);
                break;
            case BOOLEANO:
                readings[i].valor.booleano = (strcmp(valor_str, "true") == 0);
                break;
            case REAL:
                readings[i].valor.real = atof(valor_str);
                break;
            case STRING:
                strncpy(readings[i].valor.string, valor_str, MAX_STRING_VALUE_SIZE - 1);
                readings[i].valor.string[MAX_STRING_VALUE_SIZE - 1] = '\0';
                break;
        }
    }
    fclose(file);

    // Busca binÃ¡ria adaptada para ordem DECRESCENTE
    int left = 0, right = count - 1;
    int closest_index = -1;

    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (readings[mid].timestamp == target_timestamp) {
            closest_index = mid;
            break;
        } else if (readings[mid].timestamp > target_timestamp) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }

        if (closest_index == -1 ||
            llabs(readings[mid].timestamp - target_timestamp) <
            llabs(readings[closest_index].timestamp - target_timestamp)) {
            closest_index = mid;
        }
    }

    // Exibe o resultado
    if (closest_index != -1) {
        printf("Leitura mais prÃ³xima para o sensor '%s' no instante %02d/%02d/%04d %02d:%02d:%02d (epoch: %lld):\n",
               sensor_nome, day, month, year, hour, min, sec, target_timestamp);
        printf("  Timestamp: %lld\n", readings[closest_index].timestamp);
        printf("  ID Sensor: %s\n", readings[closest_index].id_sensor);
        printf("  Valor: ");
        switch (readings[closest_index].tipo_dado) {
            case INTEIRO:
                printf("%lld\n", readings[closest_index].valor.inteiro);
                break;
            case BOOLEANO:
                printf("%s\n", readings[closest_index].valor.booleano ? "true" : "false");
                break;
            case REAL:
                printf("%.2f\n", readings[closest_index].valor.real);
                break;
            case STRING:
                printf("%s\n", readings[closest_index].valor.string);
                break;
        }
    } else {
        printf("Nenhuma leitura encontrada para o sensor '%s' no arquivo '%s'.\n", sensor_nome, filename);
    }

    free(readings);
    return 0;
}