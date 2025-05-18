#pragma once

#include <cstdlib>
#include <iostream>

#define ASSERT(condition)                                                                                    \
    do                                                                                                       \
    {                                                                                                        \
        if (!(condition))                                                                                    \
        {                                                                                                    \
            std::cerr << "JLANG ASSERT FAILED: " << #condition << " at " << __FILE__ << ":" << __LINE__      \
                      << "\r\n"
std::abort();
}
}
while (0)