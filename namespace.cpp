#include "namespace.h"
#include "log.h"

// Report a failure
void fail ( beast::error_code ec, char const* what )
{
     BOOST_LOG_TRIVIAL(error) << what << ": " << ec.message();
}
