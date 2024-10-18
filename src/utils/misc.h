#ifndef LYDIASYFT_MISC_H
#define LYDIASYFT_MISC_H

#include <optional>
#include <string>

namespace Syft {

    std::optional<std::string> find_executable_using_which(const std::string& executable);

}


#endif //LYDIASYFT_MISC_H
