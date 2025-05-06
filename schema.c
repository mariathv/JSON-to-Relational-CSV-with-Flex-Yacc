#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "schema.h"

// Schema operations
Schema *create_schema()
{
    Schema *schema = malloc(sizeof(Schema));
    schema->tables = NULL;
    schema->table_count = 0;
    return schema;
}

void free_schema(Schema *schema)
{
    if (schema == NULL)
        return;

    Table *table = schema->tables;
    while (table != NULL)
    {
        Table *next = table->next;
        free_table(table);
        table = next;
    }
    free(schema);
}

void add_table(Schema *schema, Table *table)
{
    if (schema == NULL || table == NULL)
        return;

    // Add table at the end of the list to maintain insertion order
    if (schema->tables == NULL)
    {
        schema->tables = table;
    }
    else
    {
        Table *current = schema->tables;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = table;
    }
    schema->table_count++;
}

Table *find_table(Schema *schema, const char *name)
{
    if (schema == NULL || name == NULL)
        return NULL;

    Table *table = schema->tables;
    while (table != NULL)
    {
        if (strcmp(table->name, name) == 0)
        {
            return table;
        }
        table = table->next;
    }
    return NULL;
}

// Table operations
Table *create_table(const char *name)
{
    Table *table = malloc(sizeof(Table));
    table->name = strdup(name);
    table->columns = NULL;
    table->rows = NULL;
    table->next = NULL;
    table->row_count = 0;

    // Always add an 'id' column as primary key
    add_column(table, "id", "INTEGER");

    return table;
}

void free_table(Table *table)
{
    if (table == NULL)
        return;

    Column *column = table->columns;
    while (column != NULL)
    {
        Column *next = column->next;
        free(column->name);
        free(column->type);
        free(column);
        column = next;
    }

    // Free rows
    Row *row = table->rows;
    while (row != NULL)
    {
        Row *next_row = row->next;
        for (int i = 0; i < table->row_count; i++)
        {
            free(row->values[i]);
        }
        free(row->values);
        free(row);
        row = next_row;
    }

    free(table->name);
    free(table);
}

void add_column(Table *table, const char *name, const char *type)
{
    if (table == NULL || name == NULL || type == NULL)
        return;

    // Check if column already exists
    Column *existing = table->columns;
    while (existing != NULL)
    {
        if (strcmp(existing->name, name) == 0)
        {
            // Column already exists
            return;
        }
        existing = existing->next;
    }

    // Create new column
    Column *column = malloc(sizeof(Column));
    column->name = strdup(name);
    column->type = strdup(type);
    column->next = NULL;

    // Add column at the end of the list to maintain insertion order
    if (table->columns == NULL)
    {
        table->columns = column;
    }
    else
    {
        Column *current = table->columns;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = column;
    }
}

int get_column_count(Table *table)
{
    if (table == NULL)
        return 0;

    int count = 0;
    Column *column = table->columns;
    while (column != NULL)
    {
        count++;
        column = column->next;
    }
    return count;
}

void add_row(Table *table, char **values)
{
    Row *row = malloc(sizeof(Row));
    if (!row)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    row->values = values; // Array of strings for the row's values
    row->next = NULL;

    // Add row at the end of the list to maintain insertion order
    if (table->rows == NULL)
    {
        table->rows = row;
    }
    else
    {
        Row *current = table->rows;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = row;
    }
    table->row_count++;
}

Column *find_column(Table *table, const char *name)
{
    if (table == NULL || name == NULL)
        return NULL;

    Column *column = table->columns;
    while (column != NULL)
    {
        if (strcmp(column->name, name) == 0)
        {
            return column;
        }
        column = column->next;
    }
    return NULL;
}

// Helper function to convert a string to a valid table name
char *to_table_name(const char *str)
{
    if (str == NULL)
        return NULL;

    char *result = malloc(strlen(str) + 1);
    int j = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        unsigned char c = str[i];
        if (isalnum(c))
        {
            result[j++] = tolower(c);
        }
        else if (c == ' ' || c == '_')
        {
            result[j++] = '_';
        }
    }
    result[j] = '\0';
    return result;
}

// Convert node value to string
// Convert node value to string
char *node_to_string(Node *node)
{
    if (node == NULL)
    {
        return strdup("NULL");
    }

    char buffer[256];

    switch (node->type)
    {
    case NODE_STRING:
        // Access the string value based on your Node structure
        return strdup((char *)node->value.str);
    case NODE_NUMBER:
        // Access the number value based on your Node structure
        snprintf(buffer, sizeof(buffer), "%g", node->value.num);
        return strdup(buffer);
    case NODE_BOOLEAN:
        // Access the boolean value based on your Node structure
        return strdup(node->value.boolean ? "true" : "false");
    case NODE_NULL:
        return strdup("null");
    default:
        return strdup("complex_value");
    }
}

// Forward declaration
void populate_data_from_node(Node *node, Schema *schema, const char *parent_table, int *parent_id, int id);

// Schema generation
void process_ast(Node *root, const char *out_dir)
{
    if (root == NULL)
        return;

    Schema *schema = create_schema();
    generate_schema_from_node(root, schema, NULL);

    // Extract data from the AST and populate rows
    populate_data_from_node(root, schema, NULL, NULL, 1);

    write_schema_to_csv(schema, out_dir);
    free_schema(schema);
}

void generate_schema_from_node(Node *node, Schema *schema, const char *parent_table)
{
    if (node == NULL || schema == NULL)
        return;

    switch (node->type)
    {
    case NODE_OBJECT:
    {
        // Create a new table for this object (or use the provided parent_table)
        char *table_name = to_table_name(parent_table ? parent_table : "root");
        Table *table = find_table(schema, table_name);
        if (table == NULL)
        {
            table = create_table(table_name);
            add_table(schema, table);
        }

        // Add columns for each pair
        Pair *pair = node->value.pairs;
        while (pair != NULL)
        {
            // Handle different types of values in pairs (object, array, scalar)
            if (pair->value->type == NODE_OBJECT)
            {
                // Nested object - create a new table for this nested object
                char *child_table_name = to_table_name(pair->key);
                char *fk_name = malloc(strlen(pair->key) + 4); // +4 for "_id\0"
                sprintf(fk_name, "%s_id", pair->key);
                add_column(table, fk_name, "INTEGER"); // Foreign key
                free(fk_name);

                generate_schema_from_node(pair->value, schema, pair->key);
                free(child_table_name);
            }
            else if (pair->value->type == NODE_ARRAY)
            {
                // Handle arrays if necessary
                if (pair->value->value.elements != NULL)
                {
                    // Get the first element from the array
                    Element *element = pair->value->value.elements;
                    Node *first_elem = element->value;

                    if (first_elem->type == NODE_OBJECT)
                    {
                        // Array of objects - create a new table
                        char *array_table_name = malloc(strlen(table_name) + strlen(pair->key) + 2);
                        sprintf(array_table_name, "%s_%s", table_name, pair->key);
                        Table *array_table = create_table(array_table_name);

                        // Add columns based on the array's object structure
                        Pair *array_pair = first_elem->value.pairs;
                        while (array_pair != NULL)
                        {
                            const char *type = "TEXT";
                            if (array_pair->value->type == NODE_NUMBER)
                            {
                                type = "REAL";
                            }
                            else if (array_pair->value->type == NODE_BOOLEAN)
                            {
                                type = "INTEGER";
                            }
                            add_column(array_table, array_pair->key, type);
                            array_pair = array_pair->next;
                        }

                        // Add foreign key reference back to the parent table
                        char *fk_name = malloc(strlen(table_name) + 4); // +4 for "_id\0"
                        sprintf(fk_name, "%s_id", table_name);
                        add_column(array_table, fk_name, "INTEGER");
                        free(fk_name);

                        add_table(schema, array_table);
                        free(array_table_name);
                    }
                    else
                    {
                        // Array of scalars - handle as simple scalar array
                        char *array_table_name = malloc(strlen(table_name) + strlen(pair->key) + 2);
                        sprintf(array_table_name, "%s_%s", table_name, pair->key);
                        Table *array_table = create_table(array_table_name);

                        char *fk_name = malloc(strlen(table_name) + 4); // +4 for "_id\0"
                        sprintf(fk_name, "%s_id", table_name);
                        add_column(array_table, fk_name, "INTEGER");
                        free(fk_name);

                        add_column(array_table, "index", "INTEGER");
                        add_column(array_table, "value", "TEXT");

                        add_table(schema, array_table);
                        free(array_table_name);
                    }
                }
            }
            else
            {
                // Scalar value - add as a regular column
                const char *type = "TEXT";
                if (pair->value->type == NODE_NUMBER)
                {
                    type = "REAL";
                }
                else if (pair->value->type == NODE_BOOLEAN)
                {
                    type = "INTEGER";
                }
                add_column(table, pair->key, type); // Add the column for this key-value pair
            }

            pair = pair->next;
        }

        free(table_name);
        break;
    }

    case NODE_ARRAY:
        // Arrays are handled in the object case (through elements)
        break;

    default:
        break;
    }
}

// Helper function to populate data from AST into schema
void populate_data_from_node(Node *node, Schema *schema, const char *parent_table, int *parent_id, int id)
{
    if (node == NULL || schema == NULL)
        return;

    switch (node->type)
    {
    case NODE_OBJECT:
    {
        char *table_name = to_table_name(parent_table ? parent_table : "root");
        Table *table = find_table(schema, table_name);

        if (table)
        {
            // Count columns
            int col_count = get_column_count(table);

            // Allocate space for values
            char **values = malloc(col_count * sizeof(char *));
            for (int i = 0; i < col_count; i++)
            {
                values[i] = strdup("");
            }

            // Set the ID value
            char id_str[20];
            sprintf(id_str, "%d", id);

            // Find id column index
            int id_index = 0;
            Column *col = table->columns;
            while (col && strcmp(col->name, "id") != 0)
            {
                id_index++;
                col = col->next;
            }

            if (values[id_index])
                free(values[id_index]);
            values[id_index] = strdup(id_str);

            // Set parent ID if applicable
            if (parent_id && parent_table)
            {
                char *fk_name = malloc(strlen(parent_table) + 4); // +4 for "_id\0"
                sprintf(fk_name, "%s_id", parent_table);

                // Find FK column index
                int fk_index = 0;
                Column *fk_col = table->columns;
                while (fk_col && strcmp(fk_col->name, fk_name) != 0)
                {
                    fk_index++;
                    fk_col = fk_col->next;
                }

                if (fk_col)
                {
                    char parent_id_str[20];
                    sprintf(parent_id_str, "%d", *parent_id);
                    if (values[fk_index])
                        free(values[fk_index]);
                    values[fk_index] = strdup(parent_id_str);
                }

                free(fk_name);
            }

            // Process each pair in the object
            Pair *pair = node->value.pairs;
            while (pair != NULL)
            {
                if (pair->value->type != NODE_OBJECT && pair->value->type != NODE_ARRAY)
                {
                    // Find column index
                    int col_index = 0;
                    Column *column = table->columns;
                    while (column && strcmp(column->name, pair->key) != 0)
                    {
                        col_index++;
                        column = column->next;
                    }

                    if (column)
                    {
                        char *value_str = node_to_string(pair->value);
                        if (values[col_index])
                            free(values[col_index]);
                        values[col_index] = value_str;
                    }
                }
                else if (pair->value->type == NODE_OBJECT)
                {
                    // Handle nested objects with recursion
                    populate_data_from_node(pair->value, schema, pair->key, &id, id + 1);
                }
                else if (pair->value->type == NODE_ARRAY)
                {
                    // Handle arrays of objects (simplified)
                    Element *element = pair->value->value.elements;
                    int array_index = 0;

                    while (element)
                    {
                        if (element->value->type == NODE_OBJECT)
                        {
                            // Array of objects - recursively process
                            char *array_table_name = malloc(strlen(table_name) + strlen(pair->key) + 2);
                            sprintf(array_table_name, "%s_%s", table_name, pair->key);

                            populate_data_from_node(element->value, schema, array_table_name, &id, id + array_index + 1);
                            free(array_table_name);
                        }
                        else
                        {
                            // Array of scalars - handle as simple values
                            char *array_table_name = malloc(strlen(table_name) + strlen(pair->key) + 2);
                            sprintf(array_table_name, "%s_%s", table_name, pair->key);

                            Table *array_table = find_table(schema, array_table_name);
                            if (array_table)
                            {
                                // Create a row for this array element
                                int arr_col_count = get_column_count(array_table);
                                char **arr_values = malloc(arr_col_count * sizeof(char *));

                                for (int i = 0; i < arr_col_count; i++)
                                {
                                    arr_values[i] = strdup("");
                                }

                                // Set ID
                                char elem_id_str[20];
                                sprintf(elem_id_str, "%d", id + array_index + 1);

                                // Find id column index
                                int arr_id_index = 0;
                                Column *arr_col = array_table->columns;
                                while (arr_col && strcmp(arr_col->name, "id") != 0)
                                {
                                    arr_id_index++;
                                    arr_col = arr_col->next;
                                }

                                if (arr_values[arr_id_index])
                                    free(arr_values[arr_id_index]);
                                arr_values[arr_id_index] = strdup(elem_id_str);

                                // Set parent ID
                                char *fk_name = malloc(strlen(table_name) + 4);
                                sprintf(fk_name, "%s_id", table_name);

                                int fk_index = 0;
                                Column *fk_col = array_table->columns;
                                while (fk_col && strcmp(fk_col->name, fk_name) != 0)
                                {
                                    fk_index++;
                                    fk_col = fk_col->next;
                                }

                                if (fk_col)
                                {
                                    if (arr_values[fk_index])
                                        free(arr_values[fk_index]);
                                    arr_values[fk_index] = strdup(id_str);
                                }

                                // Set index
                                int idx_index = 0;
                                Column *idx_col = array_table->columns;
                                while (idx_col && strcmp(idx_col->name, "index") != 0)
                                {
                                    idx_index++;
                                    idx_col = idx_col->next;
                                }

                                if (idx_col)
                                {
                                    char idx_str[20];
                                    sprintf(idx_str, "%d", array_index);
                                    if (arr_values[idx_index])
                                        free(arr_values[idx_index]);
                                    arr_values[idx_index] = strdup(idx_str);
                                }

                                // Set value
                                int val_index = 0;
                                Column *val_col = array_table->columns;
                                while (val_col && strcmp(val_col->name, "value") != 0)
                                {
                                    val_index++;
                                    val_col = val_col->next;
                                }

                                if (val_col)
                                {
                                    char *val_str = node_to_string(element->value);
                                    if (arr_values[val_index])
                                        free(arr_values[val_index]);
                                    arr_values[val_index] = val_str;
                                }

                                // Add row to array table
                                add_row(array_table, arr_values);
                                free(fk_name);
                            }

                            free(array_table_name);
                        }

                        array_index++;
                        element = element->next;
                    }
                }

                pair = pair->next;
            }

            // Add row to table
            add_row(table, values);
        }

        free(table_name);
        break;
    }

    default:
        break;
    }
}

void write_schema_to_csv(Schema *schema, const char *out_dir)
{
    if (schema == NULL || out_dir == NULL)
        return;

    Table *table = schema->tables;
    while (table != NULL)
    {
        char filename[256];
        snprintf(filename, sizeof(filename), "%s/%s.csv", out_dir, table->name);

        FILE *file = fopen(filename, "w");
        if (file == NULL)
        {
            fprintf(stderr, "Error: Could not open file %s for writing\n", filename);
            return;
        }

        // Write header
        Column *column = table->columns;
        while (column != NULL)
        {
            fprintf(file, "%s", column->name);
            if (column->next != NULL)
            {
                fprintf(file, ",");
            }
            column = column->next;
        }
        fprintf(file, "\n");

        // Write data rows
        Row *row = table->rows;
        while (row != NULL)
        {
            column = table->columns;
            int col_index = 0;
            while (column != NULL)
            {
                if (row->values && col_index < get_column_count(table))
                {
                    fprintf(file, "%s", row->values[col_index] ? row->values[col_index] : "");
                }
                else
                {
                    fprintf(file, " ");
                }

                if (column->next != NULL)
                {
                    fprintf(file, ",");
                }
                column = column->next;
                col_index++;
            }
            fprintf(file, "\n");
            row = row->next;
        }

        fclose(file);
        table = table->next;
    }
}