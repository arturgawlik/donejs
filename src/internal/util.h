#include <string>
namespace done::internal::util {
/*
    Initializes JS `process` buildins.
*/
void PrintfError(std::string msg);
void PrintfDebug(std::string msg);
} // namespace done::internal::util
