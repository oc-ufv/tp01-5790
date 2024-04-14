#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definição de uma estrutura para representar uma instrução RISC-V
typedef struct {
    char opcode[8]; // Opcode da instrução (7 bits)
    int rs1;        // Registrador rs1 (5 bits)
    int rs2;        // Registrador rs2 (5 bits)
    int rd;         // Registrador rd (5 bits)
    int imm;        // Imediato (12 bits)
    int func3;      // Campo func3 (3 bits)
} RISC_V_Instruction;

// Função para converter um número decimal para binário com um determinado número de bits
char *to_binary(int num, int bits) {
    static char bin[17];
    bin[bits] = '\0';

    int i;
    for (i = bits - 1; i >= 0; i--) {
        bin[i] = (num & 1) + '0';
        num >>= 1;
    }
    return bin;
}

// Função para criar uma nova instrução RISC-V
RISC_V_Instruction *create_instruction(char *opcode, int rs1, int rs2, int rd, int imm, int func3) {
    RISC_V_Instruction *instruction = (RISC_V_Instruction *)malloc(sizeof(RISC_V_Instruction));
    if (instruction == NULL) {
        return NULL; // Erro de alocação de memória
    }

    strcpy(instruction->opcode, opcode);
    instruction->rs1 = rs1;
    instruction->rs2 = rs2;
    instruction->rd = rd;
    instruction->imm = imm;
    instruction->func3 = func3;

    return instruction;
}

// Função para montar uma instrução RISC-V em binário
char *assemble_instruction(RISC_V_Instruction *instruction) {
    char *binary_instruction = (char *)malloc(sizeof(char) * 33); // 32 bits + null terminator
    if (binary_instruction == NULL) {
        return NULL; // Erro de alocação de memória
    }

    sprintf(binary_instruction, "%s", instruction->opcode);
    
    // Formato I: opcode rs1 funct3 rd imm
    if (strcmp(instruction->opcode, "0000011") == 0 || strcmp(instruction->opcode, "0100011") == 0 || strcmp(instruction->opcode, "0010011") == 0 || strcmp(instruction->opcode, "1100011") == 0) {
        strcat(binary_instruction, to_binary(instruction->rs1, 5));
        strcat(binary_instruction, to_binary(instruction->func3, 3));
        strcat(binary_instruction, to_binary(instruction->rd, 5));
        strcat(binary_instruction, to_binary(instruction->imm, 12));
    } 
    // Formato R: opcode rs1 rs2 funct3 rd funct7
    else if (strcmp(instruction->opcode, "0110011") == 0) {
        strcat(binary_instruction, to_binary(instruction->rs1, 5));
        strcat(binary_instruction, to_binary(instruction->rs2, 5));
        strcat(binary_instruction, to_binary(instruction->func3, 3));
        strcat(binary_instruction, to_binary(instruction->rd, 5));
        strcat(binary_instruction, "0000000");
    }
    // Formato SB: opcode rs1 rs2 funct3 imm
    else if (strcmp(instruction->opcode, "1100011") == 0) {
        strcat(binary_instruction, to_binary(instruction->rs1, 5));
        strcat(binary_instruction, to_binary(instruction->rs2, 5));
        strcat(binary_instruction, to_binary(instruction->func3, 3));
        strcat(binary_instruction, to_binary(instruction->imm, 12));
    }

    return binary_instruction;
}

// Função para processar pseudo-instruções e converter em instruções RISC-V
void process_pseudo_instructions(char *line) {
    // Implementação de pseudo-instruções
    if (strstr(line, "li") != NULL) {
        int rd, imm;
        sscanf(line, "%*s x%d, %d", &rd, &imm);
        printf("addi x%d, x0, %d\n", rd, imm);
    }
}

// Função para converter um número de uma base para outra
int convert_base(char *str, int base) {
    return strtol(str, NULL, base);
}

// Função para implementar as instruções do Grupo 11: lh, sh, add, or, andi, sll, bne
void implement_group_11_instructions(char *line) {
    char opcode[8];
    int rs1, rs2, rd, imm;
    int func3;

    if (strstr(line, "lh") != NULL) {
        strcpy(opcode, "0000011");
        sscanf(line, "%*s x%d, %d(x%d)", &rd, &imm, &rs1);
        func3 = 2;
    } 
    else if (strstr(line, "sh") != NULL) {
        strcpy(opcode, "0100011");
        sscanf(line, "%*s x%d, %d(x%d)", &rs2, &imm, &rs1);
        func3 = 2;
    } 
    else if (strstr(line, "add") != NULL) {
        strcpy(opcode, "0110011");
        sscanf(line, "%*s x%d, x%d, x%d", &rd, &rs1, &rs2);
        func3 = 0;
    } 
    else if (strstr(line, "or") != NULL) {
        strcpy(opcode, "0110011");
        sscanf(line, "%*s x%d, x%d, x%d", &rd, &rs1, &rs2);
        func3 = 6;
    } 
    else if (strstr(line, "andi") != NULL) {
        strcpy(opcode, "0010011");
        sscanf(line, "%*s x%d, x%d, %d", &rd, &rs1, &imm);
        func3 = 7;
    } 
    else if (strstr(line, "sll") != NULL) {
        strcpy(opcode, "0110011");
        sscanf(line, "%*s x%d, x%d, %d", &rd, &rs1, &rs2);
        func3 = 1;
    } 
    else if (strstr(line, "bne") != NULL) {
        strcpy(opcode, "1100011");
        sscanf(line, "%*s x%d, x%d, %d", &rs1, &rs2, &imm);
        func3 = 1;
    } 
    else {
        printf("Instrução não suportada ou não implementada: %s\n", line);
        return;
    }

    RISC_V_Instruction *instruction = create_instruction(opcode, rs1, rs2, rd, imm, func3);
    if (instruction == NULL) {
        printf("Erro ao criar a instrução.\n");
        return;
    }

    char *binary_instruction = assemble_instruction(instruction);
    if (binary_instruction == NULL) {
        printf("Erro ao montar a instrução.\n");
        free(instruction);
        return;
    }

    printf("%s\n", binary_instruction);

    free(instruction);
    free(binary_instruction);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo.asm> [-o <arquivo_saida>]\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo de entrada.\n");
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0'; // Remove o caractere de nova linha

        // Processar pseudo-instruções
        process_pseudo_instructions(line);

        // Implementar as instruções do Grupo 11
        implement_group_11_instructions(line);
    }

    fclose(file);

    // Verificar se foi passado o argumento -o para salvar a saída em um arquivo
    if (argc >= 4 && strcmp(argv[2], "-o") == 0) {
        FILE *output_file = fopen(argv[3], "w");
        if (output_file == NULL) {
            printf("Erro ao criar o arquivo de saída.\n");
            return 1;
        }

        file = fopen(argv[1], "r");
        if (file == NULL) {
            printf("Erro ao abrir o arquivo de entrada novamente.\n");
            fclose(output_file);
            return 1;
        }

        while (fgets(line, sizeof(line), file)) {
            line[strcspn(line, "\n")] = '\0'; // Remove o caractere de nova linha

            // Implementar as instruções do Grupo 11
            implement_group_11_instructions(line);

            RISC_V_Instruction *instruction = create_instruction("0000000", 0, 0, 0, 0, 0); // Instrução vazia
            if (instruction == NULL) {
                printf("Erro ao criar a instrução.\n");
                fclose(file);
                fclose(output_file);
                return 1;
            }

            char *binary_instruction = assemble_instruction(instruction);
            if (binary_instruction == NULL) {
                printf("Erro ao montar a instrução.\n");
                free(instruction);
                fclose(file);
                fclose(output_file);
                return 1;
            }

            fprintf(output_file, "%s\n", binary_instruction);

            free(instruction);
            free(binary_instruction);
        }

        fclose(file);
        fclose(output_file);
    }

    return 0;
}
