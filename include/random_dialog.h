#pragma once

#include <string>
#include <optional>

namespace datapainter {

// Configuration result from random point generation dialog
struct RandomDialogResult {
    bool cancelled;             // True if user cancelled
    int count;                  // Number of points to generate
    std::string target;         // Target value ("x" or "o")
    bool use_normal_dist;       // True for normal, false for uniform
    double std_dev;             // Standard deviation (for normal)
    double range;               // Range (for uniform)
};

// Simple text-based dialog for random point generation
// Exits raw mode, prompts user, returns to raw mode
class RandomDialog {
public:
    // Show dialog and get user configuration
    // x_meaning and o_meaning are the valid target values
    // Returns nullopt if user cancels
    static std::optional<RandomDialogResult> show(const std::string& x_meaning,
                                                   const std::string& o_meaning);

private:
    // Helper: Prompt for integer with default
    static std::optional<int> prompt_int(const std::string& prompt, int default_val);

    // Helper: Prompt for double with default
    static std::optional<double> prompt_double(const std::string& prompt, double default_val);

    // Helper: Prompt for choice (y/n)
    static bool prompt_yes_no(const std::string& prompt, bool default_val);

    // Helper: Prompt for target choice (x or o)
    static std::optional<std::string> prompt_target(const std::string& x_meaning,
                                                     const std::string& o_meaning);
};

}  // namespace datapainter
