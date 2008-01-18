#include "memleaks.h"

#ifdef _DEBUG

void *operator new(size_t size, const char *file, int line) {
return operator new(size, 1, file, line);
}

void operator delete(void *data, const char *file, int line) {
   operator delete(data, 1, file, line);
}

#endif 