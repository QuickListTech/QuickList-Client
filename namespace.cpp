#include "namespace.h"
#include "log.h"

// Report a failure
void fail ( beast::error_code ec, char const* what )
{
     logger.err() << what << ": " << ec.message() << std::endl;
}
