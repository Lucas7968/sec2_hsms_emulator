#ifndef HSMS_COMMON_HPP
#define HSMS_COMMON_HPP

#include <string>

namespace hsms {

/// Returns the library version string for verification.
[[nodiscard]] std::string get_version();

} // namespace hsms

#endif // HSMS_COMMON_HPP