#include "random_dialog.h"
#include <iostream>
#include <sstream>
#include <limits>

namespace datapainter {

std::optional<int> RandomDialog::prompt_int(const std::string& prompt, int default_val) {
    std::cout << prompt << " [" << default_val << "]: ";
    std::cout.flush();

    std::string line;
    std::getline(std::cin, line);

    // Trim whitespace
    line.erase(0, line.find_first_not_of(" \t\n\r"));
    line.erase(line.find_last_not_of(" \t\n\r") + 1);

    if (line.empty()) {
        return default_val;
    }

    if (line == "c" || line == "cancel") {
        return std::nullopt;
    }

    std::istringstream iss(line);
    int value;
    if (iss >> value) {
        return value;
    }

    std::cout << "Invalid input, using default: " << default_val << std::endl;
    return default_val;
}

std::optional<double> RandomDialog::prompt_double(const std::string& prompt, double default_val) {
    std::cout << prompt << " [" << default_val << "]: ";
    std::cout.flush();

    std::string line;
    std::getline(std::cin, line);

    // Trim whitespace
    line.erase(0, line.find_first_not_of(" \t\n\r"));
    line.erase(line.find_last_not_of(" \t\n\r") + 1);

    if (line.empty()) {
        return default_val;
    }

    if (line == "c" || line == "cancel") {
        return std::nullopt;
    }

    std::istringstream iss(line);
    double value;
    if (iss >> value) {
        return value;
    }

    std::cout << "Invalid input, using default: " << default_val << std::endl;
    return default_val;
}

bool RandomDialog::prompt_yes_no(const std::string& prompt, bool default_val) {
    std::cout << prompt << " [" << (default_val ? "y" : "n") << "]: ";
    std::cout.flush();

    std::string line;
    std::getline(std::cin, line);

    // Trim whitespace
    line.erase(0, line.find_first_not_of(" \t\n\r"));
    line.erase(line.find_last_not_of(" \t\n\r") + 1);

    if (line.empty()) {
        return default_val;
    }

    if (line == "y" || line == "Y" || line == "yes" || line == "Yes") {
        return true;
    }

    if (line == "n" || line == "N" || line == "no" || line == "No") {
        return false;
    }

    return default_val;
}

std::optional<std::string> RandomDialog::prompt_target(const std::string& x_meaning,
                                                        const std::string& o_meaning) {
    std::cout << "Target value:" << std::endl;
    std::cout << "  1 - " << x_meaning << " (x)" << std::endl;
    std::cout << "  2 - " << o_meaning << " (o)" << std::endl;
    std::cout << "Choice [1]: ";
    std::cout.flush();

    std::string line;
    std::getline(std::cin, line);

    // Trim whitespace
    line.erase(0, line.find_first_not_of(" \t\n\r"));
    line.erase(line.find_last_not_of(" \t\n\r") + 1);

    if (line.empty() || line == "1") {
        return x_meaning;
    }

    if (line == "2") {
        return o_meaning;
    }

    if (line == "c" || line == "cancel") {
        return std::nullopt;
    }

    // Default to x_meaning
    return x_meaning;
}

std::optional<RandomDialogResult> RandomDialog::show(const std::string& x_meaning,
                                                      const std::string& o_meaning) {
    RandomDialogResult result;
    result.cancelled = false;

    // Clear screen and show dialog
    std::cout << "\033[2J\033[H";  // Clear screen
    std::cout << "=== Random Point Generation ===" << std::endl;
    std::cout << std::endl;
    std::cout << "Generate random points with configurable distribution." << std::endl;
    std::cout << "Type 'cancel' at any prompt to abort." << std::endl;
    std::cout << std::endl;

    // Prompt for count
    auto count = prompt_int("Number of points", 100);
    if (!count.has_value()) {
        result.cancelled = true;
        return result;
    }
    result.count = count.value();

    // Prompt for target
    auto target = prompt_target(x_meaning, o_meaning);
    if (!target.has_value()) {
        result.cancelled = true;
        return result;
    }
    result.target = target.value();

    // Prompt for distribution type
    std::cout << std::endl;
    result.use_normal_dist = prompt_yes_no("Use normal (Gaussian) distribution? (n for uniform)", true);

    if (result.use_normal_dist) {
        // Prompt for standard deviation
        auto std_dev = prompt_double("Standard deviation", 1.0);
        if (!std_dev.has_value()) {
            result.cancelled = true;
            return result;
        }
        result.std_dev = std_dev.value();
        result.range = 0.0;  // Not used for normal
    } else {
        // Prompt for range
        auto range = prompt_double("Range (±)", 5.0);
        if (!range.has_value()) {
            result.cancelled = true;
            return result;
        }
        result.range = range.value();
        result.std_dev = 0.0;  // Not used for uniform
    }

    std::cout << std::endl;
    std::cout << "Generating " << result.count << " random points..." << std::endl;

    return result;
}

}  // namespace datapainter
