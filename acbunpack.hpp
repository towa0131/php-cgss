#include <stdio.h>
#include <stdint.h>
#include <string.h>

#if defined(_WIN32) || defined(WIN32)

#include <windows.h>

#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "cgss_api.h"

using namespace cgss;

void MakeDirectories(const std::string &s);

void CopyStream(IStream *src, IStream *dst);