
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

void allocation_failed()
{
  write_to_log("Error: allocation failed\n");
  exit(1);
}

void addr_alignment_incorrect()
{
  write_to_log("Error: address is not a multiple of 4.\n");
}

void name_already_exists(const char *name)
{
  write_to_log("Error: name '%s' already exists in table.\n", name);
}

void write_symbol(FILE *output, uint32_t addr, const char *name)
{
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
SymbolTable *create_table(int mode)
{
  // allocate memory for SymbolTable
  SymbolTable *symtbl_ptr = (SymbolTable *)malloc(sizeof(SymbolTable));

  // check if mem alloc failed
  if (symtbl_ptr == NULL)
    allocation_failed();

  // initialize values
  symtbl_ptr->len = 0;
  symtbl_ptr->cap = SYMBOL_TABLE_MAX_DEFS; // 8
  symtbl_ptr->mode = mode;

  // allocate memory for symbol defs
  symtbl_ptr->tbl = (Symbol *)malloc(symtbl_ptr->cap * sizeof(Symbol));

  // check malloc failure
  if (symtbl_ptr->tbl == NULL)
  {
    free(symtbl_ptr);
    allocation_failed();
  }

  // return pointer to new table
  return symtbl_ptr;
}

/* Frees the given SymbolTable and all associated memory. */
void free_table(SymbolTable *table)
{
  if (!table)
    return free(table->tbl);
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
int add_to_table(SymbolTable *table, const char *name, uint32_t addr)
{
  if (!table)
    allocation_failed();

  // check address is word-aligned
  if (addr % 4 != 0)
  {
    addr_alignment_incorrect();
    return -1;
  }

  // if we're capped out on number of symbols stored, allocate more memory
  if (table->len >= table->cap)
  {
    uint32_t new_cap = (table->cap) + SYMBOL_TABLE_MAX_DEFS;
    uint32_t new_size = new_cap * sizeof(Symbol);

    Symbol *temp = (Symbol *)malloc(new_size);

    if (temp == NULL)
    {
      free(temp);
      allocation_failed();
    }

    for (int i = 0; i < table->len; i++)
      temp[i] = (table->tbl)[i];

    free(table->tbl);
    table->tbl = temp;
    table->cap = new_cap;
  }

  // Create new Symbol
  Symbol *current = table->tbl;
  if (!current)
    allocation_failed();
  else
  {
    // check for name if mode is uique
    if (table->mode == SYMTBL_UNIQUE_NAME)
    {
      for (int i = 0; i < table->len; i++)
      {
        current = (table->tbl) + i;
        if (strcmp(current->name, name) == 0)
        {
          name_already_exists(name);
          return -1;
        }
      }
    }
  }

  // NAME may point to a temporary array, so copy for safety
  char *name_cpy = (char *)malloc(sizeof(name) + 1);
  strcpy(name_cpy, name);

  // assigne and add to table
  Symbol new_sym = {name_cpy, addr};
  *(table->tbl + table->len) = new_sym;

  // increment length
  table->len += 1;

  return 0;
}

/* Returns the address (byte offset) of the given symbol. If a symbol with name
   NAME is not present in TABLE, return -1.
 */
int64_t get_addr_for_symbol(SymbolTable *table, const char *name)
{
  if (!table)
    return -1;
  else
  {
    Symbol *current;
    // Check for already assigned names
    for (int i = 0; i < table->len; i++)
    {
      current = (table->tbl) + i;
      if (strcmp(current->name, name) == 0)
        return current->addr;
    }
  }
  return -1;
}

/* Writes the SymbolTable TABLE to OUTPUT. You should use write_symbol() to
   perform the write. Do not print any additional whitespace or characters.
 */
void write_table(SymbolTable *table, FILE *output)
{
  if (!table)
    return;

  Symbol *current = table->tbl;

  if (!current)
    return;
  else
  {
    for (int i = 0; i < table->len; i++)
    {
      current = (table->tbl) + i;
      write_symbol(output, current->addr, current->name);
    }
  }
}