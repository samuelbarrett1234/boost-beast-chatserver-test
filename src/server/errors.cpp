#include "errors.hpp"
#include <boost/stacktrace.hpp>
#include <sstream>


BeastError::BeastError(boost::beast::error_code ec) :
    ec(ec)
{
    std::stringstream s;
    s << ec.what() << std::endl << boost::stacktrace::stacktrace();
    msg = s.str();
}
