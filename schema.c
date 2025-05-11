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
        int col_count = get_column_count(table);
        for (int i = 0; i < col_count; i++)
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
    if (!table)
    {
        fprintf(stderr, "Error: NULL table passed to add_row\n");
        return;
    }

    Row *row = malloc(sizeof(Row));
    if (!row)
    {
        fprintf(stderr, "Memory allocation failed for row\n");
        // Free the values that were passed in, as we won't be using them
        int col_count = get_column_count(table);
        for (int i = 0; i < col_count; i++)
        {
            if (values[i])
            {
                free(values[i]);
            }
        }
        free(values);
        return;
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

    fprintf(stderr, "Successfully added row to table '%s', now has %d rows\n",
            table->name, table->row_count);
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
        return strdup(node->value.str);
    case NODE_NUMBER:
        snprintf(buffer, sizeof(buffer), "%g", node->value.num);
        return strdup(buffer);
    case NODE_BOOLEAN:
        return strdup(node->value.boolean ? "true" : "false");
    case NODE_NULL:
        return strdup("null");
    default:
        return strdup("complex_value");
    }
}

// Forward declarations
void generate_schema_from_node(Node *node, Schema *schema, const char *parent_table);
void populate_data_from_node(Node *node, Schema *schema, const char *parent_table, int parent_id, int id);

// Schema generation
void process_ast(Node *root, const char *out_dir)
{
    if (root == NULL)
        return;

    Schema *schema = create_schema();

    // First pass: Generate schema structure
    generate_schema_from_node(root, schema, NULL);

    // Debug output
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

    // Second pass: Populate data
    populate_data_from_node(root, schema, NULL, -1, 1);

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
        // Create a new table for this object
        char *table_name = to_table_name(parent_table ? parent_table : "root");
        fprintf(stderr, "Creating/finding table: %s\n", table_name);

        Table *table = find_table(schema, table_name);
        if (table == NULL)
        {
            table = create_table(table_name);
            add_table(schema, table);
            fprintf(stderr, "Created new table: %s\n", table_name);

            // If this is a child table, add parent_id column for relationship
            if (parent_table != NULL)
            {
                char *parent_table_name = to_table_name(parent_table);
                char *parent_fk_name = malloc(strlen(parent_table_name) + 4); // +4 for "_id\0"
                sprintf(parent_fk_name, "%s_id", parent_table_name);
                add_column(table, parent_fk_name, "INTEGER"); // Foreign key to parent
                free(parent_fk_name);
                free(parent_table_name);
            }
        }

        // Process each key-value pair in the object
        Pair *pair = node->value.pairs;
        while (pair != NULL)
        {
            fprintf(stderr, "Processing key: %s\n", pair->key);

            if (pair->value->type == NODE_OBJECT)
            {
                // Nested object - create a new table with relationship
                char *child_table_name = to_table_name(pair->key);
                Table *child_table = find_table(schema, child_table_name);

                if (child_table == NULL)
                {
                    child_table = create_table(child_table_name);
                    add_table(schema, child_table);

                    // Add foreign key to parent table
                    char *parent_fk_name = malloc(strlen(table_name) + 4);
                    sprintf(parent_fk_name, "%s_id", table_name);
                    add_column(child_table, parent_fk_name, "INTEGER");
                    free(parent_fk_name);
                }

                // Create the nested table
                generate_schema_from_node(pair->value, schema, pair->key);
                free(child_table_name);
            }
            else if (pair->value->type == NODE_ARRAY)
            {
                // Create a new table for this array
                char *array_table_name = to_table_name(pair->key);
                Table *array_table = find_table(schema, array_table_name);

                if (array_table == NULL)
                {
                    array_table = create_table(array_table_name);
                    add_table(schema, array_table);

                    // Add foreign key to parent table
                    char *parent_fk_name = malloc(strlen(table_name) + 4);
                    sprintf(parent_fk_name, "%s_id", table_name);
                    add_column(array_table, parent_fk_name, "INTEGER");
                    free(parent_fk_name);
                }

                // Process array elements to determine columns
                Element *element = pair->value->value.elements;
                int element_index = 0;

                while (element != NULL)
                {
                    if (element->value->type == NODE_OBJECT)
                    {
                        // If array contains objects, add their fields as columns
                        Pair *obj_pair = element->value->value.pairs;
                        while (obj_pair != NULL)
                        {
                            const char *type = "TEXT";
                            if (obj_pair->value->type == NODE_NUMBER)
                                type = "REAL";
                            else if (obj_pair->value->type == NODE_BOOLEAN)
                                type = "INTEGER";

                            add_column(array_table, obj_pair->key, type);
                            obj_pair = obj_pair->next;
                        }
                    }
                    else
                    {
                        // If array contains primitives, add a value column
                        const char *type = "TEXT";
                        if (element->value->type == NODE_NUMBER)
                            type = "REAL";
                        else if (element->value->type == NODE_BOOLEAN)
                            type = "INTEGER";

                        add_column(array_table, "value", type);
                        break; // Only need to add this column once
                    }

                    element = element->next;
                    element_index++;
                }

                free(array_table_name);
            }
            else
            {
                // Regular scalar value - add as column
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

                fprintf(stderr, "About to add column '%s' to table '%s'\n", pair->key, table->name);
                add_column(table, pair->key, type);
            }

            pair = pair->next;
        }

        free(table_name);
        break;
    }

    case NODE_ARRAY:
        // Arrays are handled within the object case
        break;

    default:
        break;
    }
}

// Helper function to find column index by name
int find_column_index(Table *table, const char *column_name)
{
    if (!table || !column_name)
        return -1;

    int index = 0;
    Column *col = table->columns;

    while (col)
    {
        if (strcmp(col->name, column_name) == 0)
            return index;
        col = col->next;
        index++;
    }

    return -1;
}

void populate_data_from_node(Node *node, Schema *schema, const char *parent_table, int parent_id, int id)
{
    if (node == NULL || schema == NULL)
    {
        fprintf(stderr, "Warning: NULL node or schema in populate_data_from_node\n");
        return;
    }

    switch (node->type)
    {
    case NODE_OBJECT:
    {
        char *table_name = to_table_name(parent_table ? parent_table : "root");
        if (!table_name)
        {
            fprintf(stderr, "Error: Failed to create table name\n");
            return;
        }

        Table *table = find_table(schema, table_name);
        if (!table)
        {
            fprintf(stderr, "Error: Table '%s' not found\n", table_name);
            free(table_name);
            return;
        }

        // Count columns
        int col_count = get_column_count(table);
        fprintf(stderr, "Table '%s' has %d columns\n", table_name, col_count);

        // Allocate space for values
        char **values = malloc(col_count * sizeof(char *));
        if (!values)
        {
            fprintf(stderr, "Error: Failed to allocate memory for row values\n");
            free(table_name);
            return;
        }

        for (int i = 0; i < col_count; i++)
        {
            values[i] = strdup("");
            if (!values[i])
            {
                fprintf(stderr, "Error: Failed to allocate memory for value\n");
                // Clean up already allocated values
                for (int j = 0; j < i; j++)
                {
                    free(values[j]);
                }
                free(values);
                free(table_name);
                return;
            }
        }

        // Set the ID value
        char id_str[32];
        snprintf(id_str, sizeof(id_str), "%d", id);

        int id_index = find_column_index(table, "id");
        if (id_index >= 0 && id_index < col_count)
        {
            free(values[id_index]);
            values[id_index] = strdup(id_str);
            if (!values[id_index])
            {
                fprintf(stderr, "Error: Failed to allocate memory for ID value\n");
                for (int i = 0; i < col_count; i++)
                {
                    free(values[i]);
                }
                free(values);
                free(table_name);
                return;
            }
            fprintf(stderr, "Set ID column to %s\n", id_str);
        }

        // Handle parent ID reference if applicable
        if (parent_id >= 0 && parent_table && strcmp(parent_table, "root") != 0)
        {
            char *parent_table_name = to_table_name("root");
            if (!parent_table_name)
            {
                fprintf(stderr, "Error: Failed to create parent table name\n");
                for (int i = 0; i < col_count; i++)
                {
                    free(values[i]);
                }
                free(values);
                free(table_name);
                return;
            }

            char *parent_fk_name = malloc(strlen(parent_table_name) + 4);
            if (!parent_fk_name)
            {
                fprintf(stderr, "Error: Failed to allocate memory for parent FK name\n");
                free(parent_table_name);
                for (int i = 0; i < col_count; i++)
                {
                    free(values[i]);
                }
                free(values);
                free(table_name);
                return;
            }

            sprintf(parent_fk_name, "%s_id", parent_table_name);

            int parent_id_index = find_column_index(table, parent_fk_name);
            if (parent_id_index >= 0 && parent_id_index < col_count)
            {
                char parent_id_str[32];
                snprintf(parent_id_str, sizeof(parent_id_str), "%d", parent_id);
                free(values[parent_id_index]);
                values[parent_id_index] = strdup(parent_id_str);
                if (!values[parent_id_index])
                {
                    fprintf(stderr, "Error: Failed to allocate memory for parent ID value\n");
                    free(parent_fk_name);
                    free(parent_table_name);
                    for (int i = 0; i < col_count; i++)
                    {
                        free(values[i]);
                    }
                    free(values);
                    free(table_name);
                    return;
                }
                fprintf(stderr, "Set parent ID column %s to %s\n", parent_fk_name, parent_id_str);
            }

            free(parent_fk_name);
            free(parent_table_name);
        }

        // Process each pair in the object
        Pair *pair = node->value.pairs;
        while (pair != NULL)
        {
            if (!pair->key)
            {
                fprintf(stderr, "Warning: NULL key in pair\n");
                pair = pair->next;
                continue;
            }

            if (!pair->value)
            {
                fprintf(stderr, "Warning: NULL value for key '%s'\n", pair->key);
                pair = pair->next;
                continue;
            }

            if (pair->value->type != NODE_OBJECT && pair->value->type != NODE_ARRAY)
            {
                // Regular value - set in current table
                int col_index = find_column_index(table, pair->key);

                if (col_index >= 0 && col_index < col_count)
                {
                    char *value_str = node_to_string(pair->value);
                    if (!value_str)
                    {
                        fprintf(stderr, "Warning: Failed to convert value to string for column '%s'\n",
                                pair->key);
                        pair = pair->next;
                        continue;
                    }

                    fprintf(stderr, "Setting value '%s' for column '%s' at index %d\n",
                            value_str, pair->key, col_index);

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
                // Nested object - recursively populate it
                char *child_table_name = to_table_name(pair->key);
                if (!child_table_name)
                {
                    fprintf(stderr, "Error: Failed to create child table name for '%s'\n", pair->key);
                    pair = pair->next;
                    continue;
                }

                int child_id = table->row_count + 1;
                populate_data_from_node(pair->value, schema, pair->key, id, child_id);
                free(child_table_name);
            }
            else if (pair->value->type == NODE_ARRAY)
            {
                // Handle array values
                char *array_table_name = to_table_name(pair->key);
                if (!array_table_name)
                {
                    fprintf(stderr, "Error: Failed to create array table name for '%s'\n", pair->key);
                    pair = pair->next;
                    continue;
                }

                Table *array_table = find_table(schema, array_table_name);
                if (array_table)
                {
                    fprintf(stderr, "Processing array '%s' in table '%s'\n", pair->key, table_name);

                    // First, count the number of elements in the array
                    Element *element = pair->value->value.elements;
                    int element_count = 0;
                    while (element != NULL)
                    {
                        element_count++;
                        element = element->next;
                    }
                    fprintf(stderr, "Array '%s' has %d elements\n", pair->key, element_count);

                    // Store elements in a separate array for correct ordering
                    element = pair->value->value.elements;
                    for (int elem_idx = 0; elem_idx < element_count && element != NULL; elem_idx++)
                    {
                        fprintf(stderr, "Processing array element %d of %d\n", elem_idx + 1, element_count);

                        int array_col_count = get_column_count(array_table);
                        fprintf(stderr, "Array table '%s' has %d columns\n", array_table_name, array_col_count);

                        // Prepare row values for this array element
                        char **array_values = malloc(array_col_count * sizeof(char *));
                        if (!array_values)
                        {
                            fprintf(stderr, "Error: Failed to allocate memory for array values\n");
                            element = element->next;
                            continue;
                        }

                        // Initialize all values to empty strings
                        for (int i = 0; i < array_col_count; i++)
                        {
                            array_values[i] = strdup("");
                            if (!array_values[i])
                            {
                                fprintf(stderr, "Error: Failed to allocate memory for array value\n");
                                // Clean up already allocated values
                                for (int j = 0; j < i; j++)
                                {
                                    free(array_values[j]);
                                }
                                free(array_values);
                                element = element->next;
                                continue;
                            }
                        }

                        // Set ID value for this array row
                        int array_id = elem_idx + 1;
                        char array_id_str[32];
                        snprintf(array_id_str, sizeof(array_id_str), "%d", array_id);

                        int array_id_index = find_column_index(array_table, "id");
                        if (array_id_index >= 0 && array_id_index < array_col_count)
                        {
                            free(array_values[array_id_index]);
                            array_values[array_id_index] = strdup(array_id_str);
                            if (!array_values[array_id_index])
                            {
                                fprintf(stderr, "Error: Failed to allocate memory for array ID value\n");
                                for (int i = 0; i < array_col_count; i++)
                                {
                                    free(array_values[i]);
                                }
                                free(array_values);
                                element = element->next;
                                continue;
                            }
                        }

                        // Set foreign key to parent table
                        char *parent_fk_name = malloc(strlen(table_name) + 4);
                        if (!parent_fk_name)
                        {
                            fprintf(stderr, "Error: Failed to allocate memory for parent FK name\n");
                            for (int i = 0; i < array_col_count; i++)
                            {
                                free(array_values[i]);
                            }
                            free(array_values);
                            element = element->next;
                            continue;
                        }

                        sprintf(parent_fk_name, "%s_id", table_name);
                        int parent_fk_index = find_column_index(array_table, parent_fk_name);

                        if (parent_fk_index >= 0 && parent_fk_index < array_col_count)
                        {
                            free(array_values[parent_fk_index]);
                            array_values[parent_fk_index] = strdup(id_str);
                            if (!array_values[parent_fk_index])
                            {
                                fprintf(stderr, "Error: Failed to allocate memory for parent FK value\n");
                                free(parent_fk_name);
                                for (int i = 0; i < array_col_count; i++)
                                {
                                    free(array_values[i]);
                                }
                                free(array_values);
                                element = element->next;
                                continue;
                            }
                        }
                        free(parent_fk_name);

                        if (element->value->type == NODE_OBJECT)
                        {
                            // For object elements, set column values
                            Pair *obj_pair = element->value->value.pairs;
                            while (obj_pair != NULL)
                            {
                                int obj_col_index = find_column_index(array_table, obj_pair->key);
                                if (obj_col_index >= 0 && obj_col_index < array_col_count)
                                {
                                    char *value_str = node_to_string(obj_pair->value);
                                    if (value_str)
                                    {
                                        fprintf(stderr, "Setting array value '%s' for column '%s' at index %d\n",
                                                value_str, obj_pair->key, obj_col_index);
                                        free(array_values[obj_col_index]);
                                        array_values[obj_col_index] = value_str;
                                    }
                                }
                                obj_pair = obj_pair->next;
                            }
                        }
                        else
                        {
                            // For primitive elements, set the value column
                            int value_col_index = find_column_index(array_table, "value");
                            if (value_col_index >= 0 && value_col_index < array_col_count)
                            {
                                char *value_str = node_to_string(element->value);
                                if (value_str)
                                {
                                    free(array_values[value_col_index]);
                                    array_values[value_col_index] = value_str;
                                }
                            }
                        }

                        // Log the values we're about to add
                        fprintf(stderr, "Adding array row with values: ");
                        for (int i = 0; i < array_col_count; i++)
                        {
                            fprintf(stderr, "[%s] ", array_values[i] ? array_values[i] : "NULL");
                        }
                        fprintf(stderr, "\n");

                        // Add the row
                        add_row(array_table, array_values);

                        // Move to next element
                        element = element->next;
                    }
                }
                else
                {
                    fprintf(stderr, "Warning: Array table '%s' not found\n", array_table_name);
                }

                free(array_table_name);
            }

            pair = pair->next;
        }

        // Validate before adding row
        fprintf(stderr, "Adding row with values: ");
        for (int i = 0; i < col_count; i++)
        {
            fprintf(stderr, "[%s] ", values[i] ? values[i] : "NULL");
        }
        fprintf(stderr, "\n");

        // Add row to table
        add_row(table, values);

        free(table_name);
        break;
    }

    default:
        fprintf(stderr, "Ignoring node of type %d\n", node->type);
        break;
    }
}
void write_schema_to_csv(Schema *schema, const char *out_dir)
{
    if (schema == NULL || out_dir == NULL)
    {
        fprintf(stderr, "Error: NULL schema or output directory\n");
        return;
    }

    Table *table = schema->tables;
    while (table != NULL)
    {
        if (!table->name)
        {
            fprintf(stderr, "Error: Table with NULL name encountered\n");
            table = table->next;
            continue;
        }

        char filename[256];
        snprintf(filename, sizeof(filename), "%s/%s.csv", out_dir, table->name);

        FILE *file = fopen(filename, "w");
        if (file == NULL)
        {
            fprintf(stderr, "Error: Could not open file %s for writing\n", filename);
            table = table->next;
            continue;
        }

        int col_count = get_column_count(table);
        fprintf(stderr, "Writing table '%s' with %d columns to CSV\n", table->name, col_count);

        // Write header
        Column *column = table->columns;
        fprintf(stderr, "Writing headers: ");
        while (column != NULL)
        {
            if (column->name)
            {
                fprintf(file, "%s", column->name);
                fprintf(stderr, "%s ", column->name);
            }
            else
            {
                fprintf(file, "unnamed_column");
                fprintf(stderr, "unnamed_column ");
            }

            if (column->next != NULL)
            {
                fprintf(file, ",");
            }
            column = column->next;
        }
        fprintf(stderr, "\n");
        fprintf(file, "\n");

        // Write data rows
        fprintf(stderr, "Table '%s' has %d rows\n", table->name, table->row_count);
        Row *row = table->rows;
        int row_count = 0;

        while (row != NULL)
        {
            row_count++;
            fprintf(stderr, "Writing row %d/%d: ", row_count, table->row_count);

            // Check if row values are valid
            if (!row->values)
            {
                fprintf(stderr, "Warning: NULL row values for row %d\n", row_count);
                fprintf(file, "\n");
                row = row->next;
                continue;
            }

            column = table->columns;
            int col_index = 0;
            while (column != NULL)
            {
                if (col_index < col_count && row->values[col_index])
                {
                    // CSV escaping: if value contains comma, quote it
                    char *value = row->values[col_index];
                    if (strchr(value, ',') || strchr(value, '"') || strchr(value, '\n'))
                    {
                        fprintf(file, "\"%s\"", value);
                    }
                    else
                    {
                        fprintf(file, "%s", value);
                    }

                    fprintf(stderr, "[%s=%s] ",
                            column->name ? column->name : "unnamed",
                            row->values[col_index]);
                }
                else
                {
                    fprintf(file, "");
                    fprintf(stderr, "[%s=EMPTY] ", column->name ? column->name : "unnamed");
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

            // Save previous row in case next causes problems
            Row *prev_row = row;
            row = row->next;

            // Safety check
            if (row == prev_row)
            {
                fprintf(stderr, "Error: Circular reference detected in row list\n");
                break;
            }
        }

        fclose(file);
        table = table->next;
    }
}
