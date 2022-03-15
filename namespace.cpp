#include "namespace.h"

#include <iostream>

// Report a failure
void fail ( beast::error_code ec, char const* what )
{
     std::cerr << what << ": " << ec.message() << "\n";
}
