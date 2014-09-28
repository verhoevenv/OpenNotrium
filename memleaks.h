#ifndef __MEMLEAKS_H__
#define __MEMLEAKS_H__

#ifdef _DEBUG
   #include <crtdbg.h>

   #define DEBUG_NEW new(THIS_FILE, __LINE__)

   void *operator new(size_t size, const char *file, int line);
   void operator delete(void *data, const char *file, int line);
#endif

#endif /* memleaks.h */ 