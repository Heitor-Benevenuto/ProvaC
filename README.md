O programa prova1.c organiza os dados do código, processa um arquivo e nele contém diversas leituras no formato: <ID_SENSOR> , o programa também trata o arquivo indicado como argumento da linha de comando de execução do programa. Diz os diferentes sensores presentes 
Separa os dados em arquivos diferentes Em cada arquivo, ordena as leituras por timestamp, data e hora em ordem crescente. Exemplo de programa 1 para execução:
gcc processa_leituras.c -o processa_leituras

O programa prova2.c recebe a linha de comando o nome de um sensor e um dado e uma hora posta o programa localização usando busca binária, e sempre achando a busca mais próxima das horas colocadas no programa Exemplo de programa 2 para execução:
gcc busca_sensor.c -o busca_sensor -lm

O programa prova3.c gera um arquivo de teste gerando todas as horas, minutos, dias, segundos, meses e anos de acordo com o apresentado no aplicativo, caso seja colocada uma informação que não existe, o programa não será executado corretamente e ocorrerá erro o programa
utilizando 2000 leituras preliminares com valores numéricos. Exemplo de execução deste código ./sensor_generator.exe 01 01 2024 00 00 00 01 01 2025 23 59 59 S1 CONJ_Z S2 BINARIO S3 CONJ_Q S4 TEXTO
