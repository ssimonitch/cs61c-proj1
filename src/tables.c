
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "tables.h"

const int SYMTBL_NON_UNIQUE = 0;
const int SYMTBL_UNIQUE_NAME = 1;

/*******************************
 * Helper Functions
 *******************************/

void allocation_failed() {
    write_to_log("Error: allocation failed\n");
    exit(1);
}

void addr_alignment_incorrect() {
    write_to_log("Error: address is not a multiple of 4.\n");
}

void name_already_exists(const char* name) {
    write_to_log("Error: name '%s' already exists in table.\n", name);
}

void write_symbol(FILE* output, uint32_t addr, const char* name) {
    fprintf(output, "%u\t%s\n", addr, name);
}

/*******************************
 * Symbol Table Functions
 *******************************/

/* Creates a new SymbolTable containg 0 elements and returns a pointer to that
   table. Multiple SymbolTables may exist at the same time. 
   If memory allocation fails, you should call allocation_failed(). 
   Mode will be either SYMTBL_NON_UNIQUE or SYMTBL_UNIQUE_NAME. You will need
   to store this value for use during add_to_table().
 */
SymbolTable* create_table(int mode) {
    // allocate memory for SymbolTable
    SymbolTable* symbol_table = malloc(sizeof(SymbolTable));
    // check if mem alloc failed
    if (symbol_table != NULL) {
      // initialize values
      symbol_table->len  = 0;
      symbol_table->cap  = SYMBOL_TABLE_MAX_DEFS;
      symbol_table->mode = mode;
      // allocate & check memory for symbol defs
      void* symbol_defs = malloc(symbol_table->cap * sizeof(Symbol*));
      symbol_table->tbl = (Symbol*) symbol_defs;
      if (symbol_table->tbl == NULL) {
        free(symbol_table);
        allocation_failed();
      }
    } else 
      allocation_failed();
    // return pointer to new table
    return symbol_table;
}

/* Frees the given SymbolTable and all associated memory. */
void free_table(SymbolTable* table) {
  free(table->tbl);
  free(table);
}

/* Adds a new symbol and its address to the SymbolTable pointed to by TABLE. 
   ADDR is given as the byte offset from the first instruction. The SymbolTable
   must be able to resize itself as more elements are added. 

   Note that NAME may point to a temporary array, so it is not safe to simply
   store the NAME pointer. You must store a copy of the given string.

   If ADDR is not word-aligned, you should call addr_alignment_incorrect() and
   return -1. If the table's mode is SYMTBL_UNIQUE_NAME and NAME already exists 
   in the table, you should call name_already_exists() and return -1. If memory
   allocation fails, you should call allocation_failed(). 

   Otherwise, you should store the symbol name and address and return 0.
 */
int add_to_table(SymbolTable* table, const char* name, uint32_t addr) {
    uint32_t n_symbols = table->len;
    printf("\nadding name %s at addr %d\n", name, addr);

    // check address is word-aligned
    if (addr % 4) {
      addr_alignment_incorrect();
      return -1;
    }

    // check for name if mode is uique
    if (
      table->mode == SYMTBL_UNIQUE_NAME && 
      is_name_in_table(table, name)
    ) {
      name_already_exists(name);
      return -1;
    }

    // if we're capped out on number of symbols stored, allocate more memory
    if (n_symbols >= table->cap) {
      uint32_t new_cap = table->cap + SYMBOL_TABLE_MAX_DEFS;
      void* tmp = realloc(table->tbl, (new_cap * sizeof(Symbol*)));
      if (tmp == NULL) {
        free_table(table);
        allocation_failed();
      } else {
        printf("\nrealloc successful. new cap: %d\n", new_cap);
        table->cap = new_cap;
        table->tbl = (Symbol*) tmp;
      }
    }
    
    // create new Symbol
    char* name_cpy = malloc(sizeof(name));
    strcpy(name_cpy, name); // NAME may point to a temporary array, so copy for safety
    Symbol symbol = { name_cpy, addr };


    // add new symbol to table
    uint32_t symbol_offset = sizeof(Symbol*) * n_symbols;
    *(table->tbl + symbol_offset) = symbol;
    table->len = table->len + 1;

    return 0;
}

/* Returns the address (byte offset) of the given symbol. If a symbol with name
   NAME is not present in TABLE, return -1.
 */
int64_t get_addr_for_symbol(SymbolTable* table, const char* name) {
    uint32_t symbol_offset = get_symbol_offset_by_name(table, name);
    if (symbol_offset == -1)
      return -1;
    else {
      Symbol symbol = *(table->tbl + symbol_offset);
      return symbol.addr;
    }
}

/* Writes the SymbolTable TABLE to OUTPUT. You should use write_symbol() to
   perform the write. Do not print any additional whitespace or characters.
 */
void write_table(SymbolTable* table, FILE* output) {
    uint32_t n_symbols = table->len;

    for (int i = 0; i < n_symbols; i++) {
      uint32_t symbol_ptr_offset = sizeof(Symbol*) * i;
      Symbol symbol_i = *(table->tbl + symbol_ptr_offset);
      write_symbol(output, symbol_i.addr, symbol_i.name);
    }
}

int is_name_in_table(SymbolTable* table, const char* name) {
  uint32_t n_symbols = table->len;
  if (n_symbols == 0)
    return 0;

  for (int i = 0; i < n_symbols; i++) {
    uint32_t symbol_ptr_offset = sizeof(Symbol*) * i;
    printf("\nchecking at offset %d\n", symbol_ptr_offset);
    Symbol symbol_i = *(table->tbl + symbol_ptr_offset);
    if (strcmp(symbol_i.name, name) == 0) {
      return 1;
    }
  }
  printf("\nis name in table? - no\n");
  return 0;
}

uint32_t get_symbol_offset_by_name(SymbolTable* table, const char* name) {
  uint32_t n_symbols = table->len;
  if (n_symbols == 0)
    return -1;

  for (int i = 0; i < n_symbols; i++) {
    uint32_t symbol_ptr_offset = sizeof(Symbol*) * i;
    Symbol symbol_i = *(table->tbl + symbol_ptr_offset);
    if (strcmp(symbol_i.name, name) == 0) {
      return symbol_ptr_offset;
    }
  }
  return -1;
}