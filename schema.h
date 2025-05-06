#ifndef SCHEMA_H
#define SCHEMA_H

#include "ast.h"

typedef struct Column Column;
typedef struct Table Table;
typedef struct Schema Schema;
typedef struct Row Row;

struct Column
{
    char *name;
    char *type;
    Column *next;
};

struct Row
{
    char **values; // Array of strings (each corresponding to a column's value)
    Row *next;
};

struct Table
{
    char *name;
    Column *columns;
    Row *rows; // A list of rows in this table
    Table *next;
    int row_count;
};

struct Schema
{
    Table *tables;
    int table_count;
};

// Schema operations
Schema *create_schema();
void free_schema(Schema *schema);
void add_table(Schema *schema, Table *table);
Table *find_table(Schema *schema, const char *name);

// Table operations
Table *create_table(const char *name);
void free_table(Table *table);
void add_column(Table *table, const char *name, const char *type);
void add_row(Table *table, char **values);
Column *find_column(Table *table, const char *name);
int get_column_count(Table *table);
void debug_print_table(Table *table);
int find_column_index(Table *table, const char *column_name);

// String helpers
char *to_table_name(const char *str);
char *node_to_string(Node *node);

// Schema generation
void process_ast(Node *root, const char *out_dir);
void generate_schema_from_node(Node *node, Schema *schema, const char *parent_table);
void populate_data_from_node(Node *node, Schema *schema, const char *parent_table, int parent_id, int id);
void write_schema_to_csv(Schema *schema, const char *out_dir);

#endif // SCHEMA_H