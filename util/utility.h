#ifndef _UTILITY_H_
#define _UTILITY_H_

#include "../lsc.h"
#include "entries.h"

void getColumnWidths(column_widths_t* column_widths, entry_info_t* entry_infos, size_t num_entries, options_t* options);
void getModeString(char* buffer, mode_t mode);
void getNiceSize(char* buffer, size_t size);

char stringNeedsQuotes(char* name);
void printWithQuotes(char* str, char needs_quotes);

#endif