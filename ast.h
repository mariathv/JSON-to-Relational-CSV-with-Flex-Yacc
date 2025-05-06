#ifndef AST_H
#define AST_H

typedef enum {
    NODE_OBJECT,
    NODE_ARRAY,
    NODE_PAIR,
    NODE_STRING,
    NODE_NUMBER,
    NODE_BOOLEAN,
    NODE_NULL
} NodeType;

typedef struct Node Node;
typedef struct Pair Pair;
typedef struct Element Element;

struct Pair {
    char* key;
    Node* value;
    Pair* next;
};

struct Element {
    Node* value;
    Element* next;
};

struct Node {
    NodeType type;
    union {
        Pair* pairs;        // For OBJECT
        Element* elements;  // For ARRAY
        char* str;         // For STRING
        double num;        // For NUMBER
        int boolean;       // For BOOLEAN
    } value;
};

// Node creation functions
Node* create_object_node(Pair* pairs);
Node* create_array_node(Element* elements);
Node* create_string_node(char* str);
Node* create_number_node(double num);
Node* create_boolean_node(int boolean);
Node* create_null_node();

// Helper functions
Pair* append_pair(Pair* list, Pair* new_pair);
Element* append_element(Element* list, Element* new_element);

// AST operations
void print_ast_node(Node* node, int indent);
void free_ast_node(Node* node);

#endif // AST_H 