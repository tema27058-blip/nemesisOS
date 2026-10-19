#ifndef NEMESIS_EXCEPTIONS_H
#define NEMESIS_EXCEPTIONS_H

#include <nemesis/types.h>

void exceptions_init(void);
void exception_handler(u64 esr, u64 far);

#endif
