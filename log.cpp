// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: Apache-2.0

#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/core/null_deleter.hpp>

#include "log.h"
#include <fstream>

using std::string;

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
using namespace boost::log::trivial;

void logInit(string const & logFilePath, string const &logLevel)
{
    logging::core::get()->remove_all_sinks();

    typedef sinks::synchronous_sink< sinks::text_ostream_backend > TextSink;
    auto sink = boost::make_shared< TextSink >();

    auto fileStream = boost::make_shared< std::ofstream >(logFilePath, std::ios::app);
    sink->locked_backend()->add_stream(fileStream);

    boost::shared_ptr< std::ostream > cLogStream(&std::clog, boost::null_deleter());
    sink->locked_backend()->add_stream(cLogStream);

    sink->locked_backend()->auto_flush(true);

    sink->set_formatter
    (
        expr::stream
            << "[" << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S,%f") << "]"
            << " <" << logging::trivial::severity << ">\t"
            << expr::smessage
    );
    logging::core::get()->add_sink(sink);
    std::istringstream sevLevel(logLevel);

    severity_level sevLevel2;
    sevLevel >> sevLevel2;

    sink->set_filter(severity >= sevLevel2);

    logging::add_common_attributes();

}
