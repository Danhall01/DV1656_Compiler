#pragma once

#include <stdint.h>

#include "CFG.h"
#include "SymbolTable.h"

int GenerateJavaBytecode(const char* savePath, CFG_s* CFG, SymbolTable_s* ST);
