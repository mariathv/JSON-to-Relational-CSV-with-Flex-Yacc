#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

// Node creation functions
Node* create_object_node(Pair* pairs) {
    Node* node = malloc(sizeof(Node));
    if (!node) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    node->type = NODE_OBJECT;
    node->value.pairs = pairs;
    return node;
}

Node* create_array_node(Element* elements) {
    Node* node = malloc(sizeof(Node));
    if (!node) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    node->type = NODE_ARRAY;
    node->value.elements = elements;
    return node;
}

Node* create_pair_node(char* key, Node* value) {
    Node* node = malloc(sizeof(Node));
    node->type = NODE_PAIR;
    Pair* pair = malloc(sizeof(Pair));
    pair->key = key;
    pair->value = value;
    pair->next = NULL;
    node->value.pairs = pair;
    return node;
}

Node* create_string_node(char* str) {
    Node* node = malloc(sizeof(Node));
    if (!node) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    node->type = NODE_STRING;
    node->value.str = str;
    return node;
}

Node* create_number_node(double num) {
    Node* node = malloc(sizeof(Node));
    if (!node) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    node->type = NODE_NUMBER;
    node->value.num = num;
    return node;
}

Node* create_boolean_node(int boolean) {
    Node* node = malloc(sizeof(Node));
    if (!node) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    node->type = NODE_BOOLEAN;
    node->value.boolean = boolean;
    return node;
}

Node* create_null_node() {
    Node* node = malloc(sizeof(Node));
    if (!node) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    node->type = NODE_NULL;
    return node;
}

// Helper functions
Pair* append_pair(Pair* list, Pair* new_pair) {
    if (list == NULL) {
        return new_pair;
    }
    Pair* current = list;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_pair;
    return list;
}

Element* append_element(Element* list, Element* new_element) {
    if (list == NULL) {
        return new_element;
    }
    Element* current = list;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_element;
    return list;
}

// AST operations
void print_ast_node(Node* node, int indent) {
    if (node == NULL) return;

    for (int i = 0; i < indent; i++) {
        printf("  ");
    }

    switch (node->type) {
        case NODE_OBJECT:
            printf("OBJECT\n");
            Pair* pair = node->value.pairs;
            while (pair != NULL) {
                for (int i = 0; i < indent + 1; i++) {
                    printf("  ");
                }
                printf("%s:\n", pair->key);
                print_ast_node(pair->value, indent + 2);
                pair = pair->next;
            }
            break;
        case NODE_ARRAY:
            printf("ARRAY\n");
            Element* elem = node->value.elements;
            while (elem != NULL) {
                print_ast_node(elem->value, indent + 1);
                elem = elem->next;
            }
            break;
        case NODE_STRING:
            printf("STRING: %s\n", node->value.str);
            break;
        case NODE_NUMBER:
            printf("NUMBER: %g\n", node->value.num);
            break;
        case NODE_BOOLEAN:
            printf("BOOLEAN: %s\n", node->value.boolean ? "true" : "false");
            break;
        case NODE_NULL:
            printf("NULL\n");
            break;
        default:
            printf("UNKNOWN\n");
    }
}

void free_ast_node(Node* node) {
    if (node == NULL) return;

    switch (node->type) {
        case NODE_OBJECT:
            Pair* pair = node->value.pairs;
            while (pair != NULL) {
                Pair* next = pair->next;
                free(pair->key);
                free_ast_node(pair->value);
                free(pair);
                pair = next;
            }
            break;
        case NODE_ARRAY:
            Element* elem = node->value.elements;
            while (elem != NULL) {
                Element* next = elem->next;
                free_ast_node(elem->value);
                free(elem);
                elem = next;
            }
            break;
        case NODE_STRING:
            free(node->value.str);
            break;
        default:
            break;
    }
    free(node);
} 