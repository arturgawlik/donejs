#include <cstdio>
#include <v8.h>

#define ANSI_COLOR_RED "\x1b[31m"
// #define ANSI_COLOR_GREEN   "\x1b[32m"
// #define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
// #define ANSI_COLOR_MAGENTA "\x1b[35m"
// #define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

namespace done::internal::util {

void PrintfError(std::string msg) {
  printf(ANSI_COLOR_RED "[ERROR]: %s\n" ANSI_COLOR_RESET, msg.c_str());
}

void PrintfDebug(std::string msg) {
  printf(ANSI_COLOR_BLUE "[DEBUG]: %s\n" ANSI_COLOR_BLUE, msg.c_str());
}

} // namespace done::internal::util
