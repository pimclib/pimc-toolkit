#pragma once

#ifdef __APPLE__
#define IF_INDEX(v) (v)
#endif

#ifdef __linux__
#define IF_INDEX(v) static_cast<unsigned>(v)
#endif
