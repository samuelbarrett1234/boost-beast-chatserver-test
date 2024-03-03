#ifndef SERVER_ERRORS_HPP
#define SERVER_ERRORS_HPP


#include <exception>


#define SERVER_UNREACHABLE throw std::exception();
#define SERVER_ASSERT(expr) if (!static_cast<bool>(expr)) throw std::exception();
#define SERVER_VALIDATE(expr) if (!static_cast<bool>(expr)) throw std::exception();
#define SERVER_VALIDATE_ERROR_CODE(ec) if (static_cast<bool>(ec)) throw std::exception();


#endif  // SERVER_ERRORS_HPP
