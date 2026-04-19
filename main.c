#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>

#define MAX_TAPE 1000
#define MAX_RULES 100
#define RULE_LENGTH 64

typedef enum {
    ZERO = 0,
    ONE = 1,
    BLANK = 2
} Cell;

typedef struct Node {
    Cell value;
    struct Node *left;
    struct Node *right;
} Node;

typedef struct {
    int state;
    Cell read;
    Cell write;
    int move; // L(-1), .(0), R(1)
    int next_state; // -1 for halt
} Rule;

int is_blank(const char *s) {
    while (*s) {
        if (!isspace((unsigned char)*s)) return 0;
        s++;
    }
    return 1;
}

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

Node* new_node() {
    Node *n = malloc(sizeof(Node));
    if (!n) {
        perror("malloc failed");
        exit(1);
    }
    n->value = BLANK;
    n->left = NULL;
    n->right = NULL;
    return n;
}

Node* move_right(Node *head) {
    if (head->right == NULL) {
        Node *n = new_node();
        n->left = head;
        head->right = n;
        return n;
    }
    return head->right;
}

Node* move_left(Node *head) {
    if (head->left == NULL) {
        Node *n = new_node();
        n->right = head;
        head->left = n;
        return n;
    }
    return head->left;
}

void print_tape(Node *tape_head, Rule rule) {
    Node *left = tape_head;
    Node *right = tape_head;

    // go far left
    while (left->left) left = left->left;

    // go far right
    while (right->right) right = right->right;

    // find first non-blank from left
    while (left && left->value == BLANK && left != tape_head) {
        left = left->right;
    }

    // find first non-blank from right
    while (right && right->value == BLANK && right != tape_head) {
        right = right->left;
    }

    // optional padding
    int padding = 3;

    for (int i = 0; i < padding; i++) {
        if (left->left) left = left->left;
        if (right->right) right = right->right;
    }

    // print tape
    Node *tmp = left;
    while (tmp) {
        printf("%c ", cell_to_char(tmp->value));
        if (tmp == right) break;
        tmp = tmp->right;
    }
    printf("\n");

    // print head
    tmp = left;
    while (tmp) {
        if (tmp == tape_head)
            printf("^(q%d)", rule.state);
        else
            printf("  ");

        if (tmp == right) break;
        tmp = tmp->right;
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s [flags] <tape> <rules>\n", argv[0]);
        return 1;
    }

    bool animate = false;
    int delay = 0;

    int arg_index = 1;

    // Check for flag
    if (argc > 1 && (strcmp(argv[arg_index], "--animate") == 0 || strcmp(argv[arg_index], "-a") == 0)) {
        animate = true;

        if (argc <= arg_index + 1) {
            fprintf(stderr, "Missing delay value\n");
            return 1;
        }

        delay = 1000000 * atof(argv[arg_index + 1]);
        arg_index += 2;
    }

    // Read files
    if (argc < arg_index + 2) {
        fprintf(stderr, "Usage: %s [flags] <tape> <rules>\n", argv[0]);
        return 1;
    }

    FILE *tape_ptr = fopen(argv[arg_index], "r");
    FILE *rules_ptr = fopen(argv[arg_index + 1], "r");

    if (tape_ptr == NULL) {
        perror("Error opening file");
        return 1;
    }
    if (rules_ptr == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Get rules from file
    char rules_str[MAX_RULES][RULE_LENGTH];
    int rule_count = 0;
    while (rule_count < MAX_RULES &&
        fgets(rules_str[rule_count], RULE_LENGTH, rules_ptr) != NULL) {

        // remove newline
        rules_str[rule_count][strcspn(rules_str[rule_count], "\n")] = '\0';

        // trim leading whitespace
        char *line = rules_str[rule_count];
        while (isspace((unsigned char)*line)) line++;

        // skip comments and blank lines
        char *comment = strchr(line, '#');
        if (comment) *comment = '\0';
        if (*line == '\0' || *line == '#') continue;

        // remove spaces inside rule
        strip_spaces(line);

        // move cleaned line back if skipped leading spaces
        if (line != rules_str[rule_count]) {
            memmove(rules_str[rule_count], line, strlen(line) + 1);
        }

        rule_count++;
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

    char *tape_line = NULL;
    char *head_line = NULL;
    size_t len = 0;

    // Read tape line
    ssize_t nread = getline(&tape_line, &len, tape_ptr);
    if (nread == -1) {
        perror("failed to read tape line");
        return 1;
    }
    tape_line[strcspn(tape_line, "\n")] = '\0';

    // Read head line
    len = 0;
    ssize_t hread = getline(&head_line, &len, tape_ptr);
    if (hread == -1) {
        perror("failed to read head line");
        return 1;
    }
    head_line[strcspn(head_line, "\n")] = '\0';

    // Initialize tape
    Node *head = new_node();
    Node *current = head;

    for (int i = 0; i < strlen(tape_line); i++) {
        current->value = parse_cell(tape_line[i]);

        if (i < strlen(tape_line) - 1) {
            Node *n = new_node();
            current->right = n;
            n->left = current;
            current = n;
        }
    }

    // Head placement
    Node *head_ptr = head;
    int pos = 0;

    for (int i = 0; i < strlen(head_line); i++) {
        if (head_line[i] == '^') break;
        pos++;
    }

    for (int i = 0; i < pos; i++) {
        head_ptr = move_right(head_ptr);
    }

    // Cleanup
    free(tape_line);
    free(head_line);

    // Turing machine
    Node *tape_head = head_ptr;
    int current_state = 0;

    if (animate) {
        print_tape(tape_head, rules[0]);
        usleep(delay);
    }

    int i;
    while (1) {
        Cell current_cell = tape_head->value;
        bool found = false;

        for (i = 0; i < rule_count; i++) {
            if (rules[i].state == current_state &&
                rules[i].read == current_cell) {

                // write
                tape_head->value = rules[i].write;

                // move
                if (rules[i].move == -1) {
                    tape_head = move_left(tape_head);
                } else if (rules[i].move == 1) {
                    tape_head = move_right(tape_head);
                }

                // state transition
                current_state = rules[i].next_state;

                // animate
                if (animate) {
                    print_tape(tape_head, rules[i]);
                    usleep(delay);
                }

                found = true;
                break;
            }
        }

        if (!found) {
            fprintf(stderr, "No matching rule found\n");
            break;
        }

        if (current_state == -1) {
            break;
        }
    }

    print_tape(tape_head, rules[i]);

    // Memory cleanup
    Node *cur = head;
    while (cur->left) cur = cur->left; // go to far left

    while (cur) {
        Node *next = cur->right;
        free(cur);
        cur = next;
    }

    fclose(tape_ptr);
    fclose(rules_ptr);

    return 0;
}
