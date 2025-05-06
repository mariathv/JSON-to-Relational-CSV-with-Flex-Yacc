# JSON to Relational CSV Converter

A tool that converts JSON files into relational CSV tables using Flex and Yacc/Bison.

## Requirements

- Flex (lexer generator)
- Bison (parser generator)
- GCC (C compiler)
- Make

## Building

To build the project, run:

```bash
make
```

This will create the `json2relcsv` executable.

## Usage

Run the tool as:

```bash
./json2relcsv < input.json [--print-ast] [--out-dir DIR]
```

Options:
- `--print-ast`: Print the Abstract Syntax Tree to stdout
- `--out-dir DIR`: Specify output directory for CSV files (default: current directory)

## Features

- Handles any valid JSON up to 30 MiB
- Builds an AST that lasts until the program ends
- Streams CSV rows using conversion rules
- Assigns integer primary keys (id) and foreign keys
- Writes one .csv file per table
- Reports first error's line and column, exits non-zero on bad JSON

## Conversion Rules

1. Object → table row: Objects with same keys go in one table
2. Array of objects → child table: One row per element, with a foreign key to parent
3. Array of scalars → junction table: Columns parent_id, index, value
4. Scalars → columns: JSON null becomes empty
5. Every row gets an id. Foreign keys are <parent>_id
6. File name = table name + .csv; include header row

## Example

Input JSON:
```json
{
  "id": 1,
  "name": "Ali",
  "age": 19
}
```

Output (people.csv):
```
id,name,age
1,Ali,19
```

## Error Handling

The tool reports errors with line and column numbers:
```
Error: Unexpected character 'x' at line 2, column 5
```

## License

This project is licensed under the MIT License. 