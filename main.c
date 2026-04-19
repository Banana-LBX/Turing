#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_TAPE 1000
#define MAX_RULES 100
#define RULE_LENGTH 64

typedef enum {
    ZERO = 0,
    ONE = 1,
    BLANK = 2
} Cell;

typedef struct {
    int state;
    Cell read;
    Cell write;
    int move; // L(-1), .(0), R(1)
    int next_state; // -1 for halt
} Rule;

void strip_spaces(char* str) {
    char *read = str, *write = str;
    while (*read) {
        if (*read != ' ') {
            *write++ = *read;
        }
        read++;
    }
    *write = '\0';
}

Cell parse_cell(char c) {
    if (c == '0') return ZERO;
    if (c == '1') return ONE;
    if (c == '_') return BLANK;

    fprintf(stderr, "Invalid cell symbol: %c\n", c);
    exit(1);
}

char cell_to_char(Cell c) {
    if (c == ZERO) return '0';
    if (c == ONE) return '1';
    return '_';
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <tape> <rules>\n", argv[0]);
        return 1;
    }

    // Read files into strings
    FILE *tape_ptr = fopen(argv[1], "r");
    if (tape_ptr == NULL) {
        perror("Error opening file");
        return 1;
    }

    FILE *rules_ptr = fopen(argv[2], "r");
    if (rules_ptr == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Get tape from file
    char tape_str[MAX_TAPE];
    char head_str[MAX_TAPE];

    fgets(tape_str, sizeof(tape_str), tape_ptr);
    fgets(head_str, sizeof(head_str), tape_ptr);

    tape_str[strcspn(tape_str, "\n")] = '\0';
    head_str[strcspn(head_str, "\n")] = '\0';

    // Get index of the head
    int head_offset = -1;

    for (int i = 0; i < strlen(head_str); i++) {
        if (head_str[i] == '^') {
            head_offset = i;
            break;
        }
    }

    if (strlen(head_str) > strlen(tape_str)) {
        fprintf(stderr, "Head line cannot exceed tape line\n");
        return 1;
    }

    if (head_offset == -1) {
        fprintf(stderr, "No ^ found in head line\n");
        return 1;
    }

    // Get rules from file
    char rules_str[MAX_RULES][RULE_LENGTH];
    int rule_count = 0;
    while (rule_count < MAX_RULES &&
        fgets(rules_str[rule_count], RULE_LENGTH, rules_ptr) != NULL) {

        rules_str[rule_count][strcspn(rules_str[rule_count], "\n")] = '\0'; // remove newline
        strip_spaces(rules_str[rule_count]);

        rule_count++;
    }

    // Convert input string to array fo cells
    int tape_size = 100; // initial size
    Cell *tape = malloc(tape_size * sizeof(Cell));

    if (!tape) {
        perror("malloc failed");
        exit(1);
    }

    for (int i = 0; i < tape_size; i++) {
        tape[i] = BLANK;
    }

    int start = MAX_TAPE / 2;
    int base = start - head_offset;

    for (int i = 0; i < strlen(tape_str); i++) {
        int pos = base + i;

        if (tape_str[i] == '0') tape[pos] = ZERO;
        else if (tape_str[i] == '1') tape[pos] = ONE;
        else if (tape_str[i] == '_') tape[pos] = BLANK;
        else {
            fprintf(stderr, "Invalid tape symbol\n");
            return 1;
        }
    }

    // Parse rules
    Rule rules[MAX_RULES];
    for (int i = 0; i < rule_count; ++i) {
        int state;
        char move;
        char next_state_str[10];
        char read_char, write_char;

        int matched = sscanf(rules_str[i], "q%d:%c/%c/%c/%s",
                            &state, &read_char, &write_char, &move, next_state_str);

        if (matched != 5) {
            fprintf(stderr, "Invalid rule format: %s\n", rules_str[i]);
            return 1;
        }

        rules[i].state = state;
        rules[i].read = parse_cell(read_char);
        rules[i].write = parse_cell(write_char);

        switch (tolower(move)) {
            case 'l': rules[i].move = -1; break;
            case 'r': rules[i].move = 1; break;
            case '.': rules[i].move = 0; break;
            default:
                fprintf(stderr, "Invalid move in rule: %s\n", rules_str[i]);
                return 1;
        }

        if (next_state_str[0] == 'q') {
            if (tolower(next_state_str[1]) == 'h') {
                rules[i].next_state = -1; // halt
            } else {
                rules[i].next_state = atoi(&next_state_str[1]);
            }
        } else {
            fprintf(stderr, "Invalid next state: %s\n", next_state_str);
            return 1;
        }
    }

    // Turing machine
    int head_position = start;
    int current_state = 0;

    while (1) {
        Cell current_cell = tape[head_position];
        bool found = false;

        for (int i = 0; i < rule_count; i++) {
            if (rules[i].state == current_state &&
                rules[i].read == current_cell) {

                // Apply rule
                tape[head_position] = rules[i].write;
                head_position += rules[i].move;
                current_state = rules[i].next_state;

                found = true;
                break;
            }
        }

        if (!found) {
            fprintf(stderr, "No matching rule found\n");
            break;
        }

        if (current_state == -1) {
            break; // halt
        }
    }

    // Print result
    int left = head_position - 10;
    int right = head_position + 10;

    if (left < 0) left = 0;
    if (right >= MAX_TAPE) right = MAX_TAPE - 1;

    for (int i = left; i <= right; i++) {
        printf("%c ", cell_to_char(tape[i]));
    }
    printf("\n");

    for (int i = left; i <= right; i++) {
        if (i == head_position)
            printf("^ ");
        else
            printf("  ");
    }
    printf("\n");

    fclose(tape_ptr);
    fclose(rules_ptr);
    return 0;
}
