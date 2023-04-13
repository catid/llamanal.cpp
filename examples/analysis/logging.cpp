#include "logging.hpp"

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/thread/thread.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/named_scope.hpp>

namespace analysis {

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;


//------------------------------------------------------------------------------
// File Logging

using file_sink_t = sinks::asynchronous_sink<sinks::text_file_backend>;
static boost::shared_ptr<file_sink_t> m_file_sink;

void init_file_logging()
{
    boost::shared_ptr<logging::core> core = logging::core::get();

    // Create a rotating file sink up to 10 MB
    boost::shared_ptr<sinks::text_file_backend> backend = boost::make_shared<sinks::text_file_backend>(
        keywords::file_name = "analysis_%N.log",
        keywords::rotation_size = 10 * 1024 * 1024
    );

    // Create an async frontend and attach it to the backend
    m_file_sink = boost::make_shared<file_sink_t>(backend);

    // Set format
    auto fmtTimeStamp = boost::log::expressions::
        format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f");
    auto fmtSeverity = boost::log::expressions::
        attr<boost::log::trivial::severity_level>("Severity");
    boost::log::formatter logFmt =
        boost::log::expressions::format("[%1%] [%2%] %3%")
        % fmtTimeStamp % fmtSeverity % boost::log::expressions::smessage;
    m_file_sink->set_formatter(logFmt);

    // Create a background thread for logging
    m_file_sink->locked_backend()->auto_flush(true);

    // Always log at debug level to disk
    m_file_sink->set_filter(logging::trivial::severity >= logging::trivial::debug);

    // Add the sink to the logging core
    core->add_sink(m_file_sink);

    BOOST_LOG_TRIVIAL(debug) << "Logging to: analysis_%N.log";
}

void stop_file_logging()
{
    if (m_file_sink) {
        m_file_sink->stop();
        m_file_sink->flush();
        m_file_sink.reset();
    }
}


//------------------------------------------------------------------------------
// Console Logging

using console_sink_t = sinks::asynchronous_sink<sinks::text_ostream_backend>;
static boost::shared_ptr<console_sink_t> m_console_sink;

void init_console_logging()
{
    boost::shared_ptr<logging::core> core = logging::core::get();

    // Create a backend and initialize it with a stream
    auto backend = boost::make_shared<sinks::text_ostream_backend>();
    backend->add_stream(boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));

    // Wrap it into the frontend and register in the core
    m_console_sink = boost::make_shared<console_sink_t>(backend);

    // Set format
    auto fmtTimeStamp = boost::log::expressions::
        format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f");
    auto fmtSeverity = boost::log::expressions::
        attr<boost::log::trivial::severity_level>("Severity");
    boost::log::formatter logFmt =
        boost::log::expressions::format("[%1%] [%2%] %3%")
        % fmtTimeStamp % fmtSeverity % boost::log::expressions::smessage;
    m_console_sink->set_formatter(logFmt);

    // Add the sink to the logging core
    core->add_sink(m_console_sink);
}

void stop_console_logging()
{
    if (m_console_sink) {
        m_console_sink->stop();
        m_console_sink->flush();
        m_console_sink.reset();
    }
}


//------------------------------------------------------------------------------
// Logging Initialization

void init_logging(bool verbose)
{
    boost::shared_ptr<logging::core> core = logging::core::get();

    // This is required for timestamps to work
    boost::log::add_common_attributes();

    // Remove default console sink so we can set our own format
    core->remove_all_sinks();

    if (verbose) {
        core->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);
    } else {
        core->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
    }

    init_console_logging();
    init_file_logging();

    if (verbose) {
        BOOST_LOG_TRIVIAL(info) << "Verbose logging enabled.";
    }
}

void stop_logging()
{
    boost::shared_ptr<logging::core> core = logging::core::get();

    core->flush();
    core->remove_all_sinks();

    stop_console_logging();
    stop_file_logging();
}


} // namespace analysis
