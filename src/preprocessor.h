#ifndef MINI_C_PREPROCESSOR_H
#define MINI_C_PREPROCESSOR_H

#include "common.h"

char *preprocess_source(const char *source, const char *filename, bool *had_error);

#endif
