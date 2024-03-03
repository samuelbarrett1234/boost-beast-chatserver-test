#ifndef SERVER_ERRORS_HPP
#define SERVER_ERRORS_HPP


#include <exception>
#include <boost/beast.hpp>


struct BeastError :
    public std::exception
{
    BeastError(boost::beast::error_code ec);

    boost::beast::error_code ec;
    std::string msg;

    virtual inline const char* what() const override
    {
        return msg.c_str();
    }
};


#define SERVER_UNREACHABLE throw std::exception();
#define SERVER_ASSERT(expr) if (!static_cast<bool>(expr)) throw std::exception();
#define SERVER_VALIDATE(expr) if (!static_cast<bool>(expr)) throw std::exception();
#define SERVER_VALIDATE_ERROR_CODE(ec) if (static_cast<bool>(ec)) throw BeastError(ec);


#endif  // SERVER_ERRORS_HPP
