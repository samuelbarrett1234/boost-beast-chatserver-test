#ifndef SERVER_ERRORS_HPP
#define SERVER_ERRORS_HPP


#include <exception>


#define SERVER_UNREACHABLE throw std::runtime_error();
#define SERVER_ASSERT(expr) if (!static_cast<bool>(expr)) throw std::runtime_error();
#define SERVER_VALIDATE(expr) if (!static_cast<bool>(expr)) throw std::runtime_error();
#define SERVER_VALIDATE_ERROR_CODE(ec) if (static_cast<bool>(ec)) throw std::runtime_error();


#endif  // SERVER_ERRORS_HPP
