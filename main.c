#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "ast.h"
#include "schema.h"
#include "parser.h"

extern Node *root;
extern int yyparse(void);
extern int yycolumn;

void print_usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s < input.json [--print-ast] [--out-dir DIR]\n", program_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  --print-ast    Print the Abstract Syntax Tree to stdout\n");
    fprintf(stderr, "  --out-dir DIR  Specify output directory for CSV files (default: current directory)\n");
    exit(1);
}

int main(int argc, char **argv)
{
    int print_ast = 0;
    char *out_dir = ".";

    // Parse command line arguments
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--print-ast") == 0)
        {
            print_ast = 1;
        }
        else if (strcmp(argv[i], "--out-dir") == 0)
        {
            if (i + 1 < argc)
            {
                out_dir = argv[++i];
                // Create output directory if it doesn't exist
                if (access(out_dir, F_OK) != 0)
                {
                    if (mkdir(out_dir, 0755) != 0)
                    {
                        fprintf(stderr, "Error: Could not create directory %s\n", out_dir);
                        return 1;
                    }
                }
            }
            else
            {
                print_usage(argv[0]);
            }
        }
        else
        {
            print_usage(argv[0]);
        }
    }

    // Parse JSON input
    yyparse();

    // Print AST if requested
    if (print_ast)
    {
        print_ast_node(root, 0);
    }

    // Process AST and generate CSV files
    process_ast(root, out_dir);

    // Cleanup
    free_ast_node(root);
    return 0;
}