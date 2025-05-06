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

    // Debug output
    fprintf(stderr, "Adding column '%s' of type '%s' to table '%s'\n", name, type, table->name);

    // Check if column already exists
    Column *existing = table->columns;
    while (existing != NULL)
    {
        if (strcmp(existing->name, name) == 0)
        {
            // Column already exists
            fprintf(stderr, "Column '%s' already exists in table '%s', skipping\n", name, table->name);
            return;
        }
        existing = existing->next;
    }

    // Create new column
    Column *column = malloc(sizeof(Column));
    if (!column)
    {
        fprintf(stderr, "Memory allocation failed for column\n");
        return;
    }

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

    fprintf(stderr, "Successfully added column '%s' to table '%s'\n", name, table->name);
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
    fprintf(stderr, "Table '%s' has %d columns\n", table->name, count);
    return count;
}

void debug_print_table(Table *table)
{
    if (!table)
    {
        fprintf(stderr, "NULL table pointer!\n");
        return;
    }

    fprintf(stderr, "Table '%s' structure:\n", table->name);
    fprintf(stderr, "  Columns:\n");

    Column *col = table->columns;
    int count = 0;
    while (col)
    {
        fprintf(stderr, "    %d. %s (%s)\n", ++count, col->name, col->type);
        col = col->next;
    }

    fprintf(stderr, "  Total columns: %d\n", count);
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
        return strdup(node->value.str); // Use str as per your AST definition
    case NODE_NUMBER:
        snprintf(buffer, sizeof(buffer), "%g", node->value.num); // Use num as per your AST definition
        return strdup(buffer);
    case NODE_BOOLEAN:
        return strdup(node->value.boolean ? "true" : "false"); // Use boolean as per your AST definition
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

    // Debug output to show schema structure
    fprintf(stderr, "Schema structure after generation:\n");
    Table *debug_table = schema->tables;
    while (debug_table != NULL)
    {
        fprintf(stderr, "Table: %s\n", debug_table->name);
        fprintf(stderr, "Columns: ");
        Column *debug_col = debug_table->columns;
        while (debug_col != NULL)
        {
            fprintf(stderr, "%s (%s), ", debug_col->name, debug_col->type);
            debug_col = debug_col->next;
        }
        fprintf(stderr, "\n");
        debug_table = debug_table->next;
    }

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
        fprintf(stderr, "Creating/finding table: %s\n", table_name);

        Table *table = find_table(schema, table_name);
        if (table == NULL)
        {
            table = create_table(table_name);
            add_table(schema, table);
            fprintf(stderr, "Created new table: %s\n", table_name);
        }

        // Add columns for each pair - CRITICAL PART, KEY FIELDS GET PROCESSED HERE
        Pair *pair = node->value.pairs;
        while (pair != NULL)
        {
            fprintf(stderr, "Processing key: %s\n", pair->key);

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
                // Handle arrays code remains unchanged
                // ...
            }
            else
            {
                // Scalar value - add as a regular column
                const char *type = "TEXT";
                if (pair->value->type == NODE_NUMBER)
                {
                    type = "REAL";
                    fprintf(stderr, "Adding NUMBER column: %s as %s\n", pair->key, type);
                }
                else if (pair->value->type == NODE_BOOLEAN)
                {
                    type = "INTEGER";
                    fprintf(stderr, "Adding BOOLEAN column: %s as %s\n", pair->key, type);
                }
                else
                {
                    fprintf(stderr, "Adding TEXT column: %s as %s\n", pair->key, type);
                }

                // ADD DEBUG HERE TO TRACE THE COLUMN ADDITION
                fprintf(stderr, "About to add column '%s' to table '%s'\n", pair->key, table->name);
                add_column(table, pair->key, type); // Add the column for this key-value pair

                // Verify the column was added
                Column *check_col = find_column(table, pair->key);
                if (check_col)
                {
                    fprintf(stderr, "Successfully verified column '%s' exists\n", pair->key);
                }
                else
                {
                    fprintf(stderr, "WARNING: Failed to find column '%s' after adding!\n", pair->key);
                }
            }

            pair = pair->next;
        }

        // DEBUG: Print current table structure after all columns added
        fprintf(stderr, "After processing, table %s has columns: ", table->name);
        Column *debug_col = table->columns;
        while (debug_col)
        {
            fprintf(stderr, "%s, ", debug_col->name);
            debug_col = debug_col->next;
        }
        fprintf(stderr, "\n");

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
            fprintf(stderr, "Table '%s' has %d columns\n", table_name, col_count);

            // Allocate space for values - initialize all to empty strings
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

            if (col && id_index < col_count)
            {
                if (values[id_index])
                    free(values[id_index]);
                values[id_index] = strdup(id_str);
                fprintf(stderr, "Set ID column to %s\n", id_str);
            }

            // Handle parent ID reference if applicable
            if (parent_id && parent_table)
            {
                // Parent ID handling code...
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

                    if (column && col_index < col_count)
                    {
                        // Make sure we set values for all columns
                        char *value_str = node_to_string(pair->value);
                        fprintf(stderr, "Setting value '%s' for column '%s' at index %d\n",
                                value_str, pair->key, col_index);

                        if (values[col_index])
                            free(values[col_index]);
                        values[col_index] = value_str;
                    }
                    else
                    {
                        fprintf(stderr, "Column not found for key: %s\n", pair->key);
                    }
                }
                else if (pair->value->type == NODE_OBJECT)
                {
                    // Nested object handling...
                }
                else if (pair->value->type == NODE_ARRAY)
                {
                    // Array handling...
                }

                pair = pair->next;
            }

            // Validate before adding row
            fprintf(stderr, "Adding row with values: ");
            for (int i = 0; i < col_count; i++)
            {
                fprintf(stderr, "[%s] ", values[i]);
            }
            fprintf(stderr, "\n");

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

        int col_count = get_column_count(table);
        fprintf(stderr, "Writing table '%s' with %d columns to CSV\n", table->name, col_count);

        // Write header
        Column *column = table->columns;
        fprintf(stderr, "Writing headers: ");
        while (column != NULL)
        {
            fprintf(file, "%s", column->name);
            fprintf(stderr, "%s ", column->name);

            if (column->next != NULL)
            {
                fprintf(file, ",");
            }
            column = column->next;
        }
        fprintf(stderr, "\n");
        fprintf(file, "\n");

        // Write data rows
        Row *row = table->rows;
        while (row != NULL)
        {
            fprintf(stderr, "Writing row values: ");

            column = table->columns;
            int col_index = 0;
            while (column != NULL)
            {
                if (row->values && col_index < col_count)
                {
                    fprintf(file, "%s", row->values[col_index] ? row->values[col_index] : "");
                    fprintf(stderr, "[%s=%s] ", column->name, row->values[col_index] ? row->values[col_index] : "");
                }
                else
                {
                    fprintf(file, " ");
                    fprintf(stderr, "[%s=EMPTY] ", column->name);
                }

                if (column->next != NULL)
                {
                    fprintf(file, ",");
                }
                column = column->next;
                col_index++;
            }
            fprintf(stderr, "\n");
            fprintf(file, "\n");
            row = row->next;
        }

        fclose(file);
        table = table->next;
    }
}