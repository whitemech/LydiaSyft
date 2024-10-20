#include "misc.h"

#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>


namespace Syft {

    std::optional<std::string> find_executable_using_which(const std::string& executable) {
        // Build the command string
        std::string command = "which " + executable;  // No need for -a since we only want the first match

        // Open a pipe to the command
        std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);
        if (!pipe) {
            std::cerr << "popen() failed!" << std::endl;
            return std::nullopt;
        }

        // Read the output of the command (only the first line)
        char buffer[128];
        std::string result;
        if (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
            result = buffer;  // Capture the first result
        }

        // Remove trailing newline character from result, if present
        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }

        // if result is empty, return null
        if (result.empty()){
            return std::nullopt;
        }

        return result;
    }

}
