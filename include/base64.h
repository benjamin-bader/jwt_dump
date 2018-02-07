#ifndef JWT_BASE64_H
#define JWT_BASE64_H

#include <string>

namespace jwt {

std::string base64_urlsafe_decode(const std::string& data);
std::string base64_decode(const std::string& data);

} // namespace jwt

#endif // JWT_BASE64_H
