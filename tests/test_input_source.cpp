#include <gtest/gtest.h>
#include "input_source.h"
#include "terminal.h"
#include <fstream>
#include <cstdio>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#define close _close
#else
#include <unistd.h>
#endif

using namespace datapainter;

class InputSourceTest : public ::testing::Test {
protected:
    void TearDown() override {
        // Clean up any temporary test files
        for (const auto& file : temp_files_) {
            std::remove(file.c_str());
        }
    }

    // Helper to create a temporary keystroke file
    std::string create_temp_file(const std::string& content) {
#ifdef _WIN32
        // Windows: use tmpnam_s for temporary file name
        char temp_name[L_tmpnam_s];
        if (tmpnam_s(temp_name, L_tmpnam_s) != 0) {
            return "";
        }
        std::string filename(temp_name);
#else
        // Unix/Linux/macOS: use mkstemp
        char temp_name[] = "/tmp/datapainter_test_XXXXXX";
        int fd = mkstemp(temp_name);
        if (fd == -1) {
            return "";
        }
        std::string filename(temp_name);
        close(fd);
#endif

        std::ofstream file(filename);
        file << content;
        file.close();

        temp_files_.push_back(filename);
        return filename;
    }

    std::vector<std::string> temp_files_;
};

// Test: Read keystroke file and parse each line
TEST_F(InputSourceTest, ReadKeystrokeFileAndParseEachLine) {
    std::string content = "x\no\n+\n";
    std::string filename = create_temp_file(content);

    FileInputSource source(filename);
    EXPECT_TRUE(source.has_input());

    EXPECT_EQ(source.read_key(), 'x');
    EXPECT_EQ(source.read_key(), 'o');
    EXPECT_EQ(source.read_key(), '+');
    EXPECT_FALSE(source.has_input());
    EXPECT_EQ(source.read_key(), -1);  // EOF
}

// Test: Parse regular character keystrokes
TEST_F(InputSourceTest, ParseRegularCharacterKeystrokes) {
    std::string content = "a\nb\nc\n1\n2\n";
    std::string filename = create_temp_file(content);

    FileInputSource source(filename);
    EXPECT_EQ(source.read_key(), 'a');
    EXPECT_EQ(source.read_key(), 'b');
    EXPECT_EQ(source.read_key(), 'c');
    EXPECT_EQ(source.read_key(), '1');
    EXPECT_EQ(source.read_key(), '2');
}

// Test: Parse special key names (<up>, <down>, etc.)
TEST_F(InputSourceTest, ParseSpecialKeyNames) {
    std::string content = "<up>\n<down>\n<left>\n<right>\n";
    std::string filename = create_temp_file(content);

    FileInputSource source(filename);
    EXPECT_EQ(source.read_key(), Terminal::KEY_UP_ARROW);
    EXPECT_EQ(source.read_key(), Terminal::KEY_DOWN_ARROW);
    EXPECT_EQ(source.read_key(), Terminal::KEY_LEFT_ARROW);
    EXPECT_EQ(source.read_key(), Terminal::KEY_RIGHT_ARROW);
}

// Test: Parse <space>, <tab>, <enter>, <esc>
TEST_F(InputSourceTest, ParseCommonSpecialKeys) {
    std::string content = "<space>\n<tab>\n<enter>\n<esc>\n";
    std::string filename = create_temp_file(content);

    FileInputSource source(filename);
    EXPECT_EQ(source.read_key(), ' ');
    EXPECT_EQ(source.read_key(), '\t');
    EXPECT_EQ(source.read_key(), '\n');
    EXPECT_EQ(source.read_key(), 27);  // ESC
}

// Test: Ignore comment lines starting with #
TEST_F(InputSourceTest, IgnoreCommentLines) {
    std::string content = "# This is a comment\nx\n# Another comment\no\n";
    std::string filename = create_temp_file(content);

    FileInputSource source(filename);
    EXPECT_EQ(source.read_key(), 'x');
    EXPECT_EQ(source.read_key(), 'o');
    EXPECT_FALSE(source.has_input());
}

// Test: Ignore empty lines
TEST_F(InputSourceTest, IgnoreEmptyLines) {
    std::string content = "x\n\n\no\n\n";
    std::string filename = create_temp_file(content);

    FileInputSource source(filename);
    EXPECT_EQ(source.read_key(), 'x');
    EXPECT_EQ(source.read_key(), 'o');
    EXPECT_FALSE(source.has_input());
}

// Test: Handle file not found error
TEST_F(InputSourceTest, HandleFileNotFoundError) {
    FileInputSource source("/nonexistent/file/path.txt");
    EXPECT_FALSE(source.has_input());
    EXPECT_FALSE(source.get_error().empty());
    EXPECT_NE(source.get_error().find("Could not open file"), std::string::npos);
}

// Test: Return ordered sequence of keystrokes
TEST_F(InputSourceTest, ReturnOrderedSequenceOfKeystrokes) {
    std::string content = "x\n<up>\no\n<down>\n+\n";
    std::string filename = create_temp_file(content);

    FileInputSource source(filename);

    std::vector<int> expected = {'x', Terminal::KEY_UP_ARROW, 'o', Terminal::KEY_DOWN_ARROW, '+'};
    std::vector<int> actual;

    while (source.has_input()) {
        actual.push_back(source.read_key());
    }

    EXPECT_EQ(actual, expected);
}

// Test: FileInputSource returns EOF when sequence exhausted
TEST_F(InputSourceTest, FileInputSourceReturnsEOF) {
    std::string content = "x\no\n";
    std::string filename = create_temp_file(content);

    FileInputSource source(filename);
    EXPECT_EQ(source.read_key(), 'x');
    EXPECT_EQ(source.read_key(), 'o');

    // After reading all keystrokes
    EXPECT_FALSE(source.has_input());
    EXPECT_EQ(source.read_key(), -1);
    EXPECT_EQ(source.read_key(), -1);  // Multiple calls still return -1
}

// Test: Case-insensitive special key parsing
TEST_F(InputSourceTest, CaseInsensitiveSpecialKeyParsing) {
    std::string content = "<UP>\n<Down>\n<LEFT>\n<Right>\n";
    std::string filename = create_temp_file(content);

    FileInputSource source(filename);
    EXPECT_EQ(source.read_key(), Terminal::KEY_UP_ARROW);
    EXPECT_EQ(source.read_key(), Terminal::KEY_DOWN_ARROW);
    EXPECT_EQ(source.read_key(), Terminal::KEY_LEFT_ARROW);
    EXPECT_EQ(source.read_key(), Terminal::KEY_RIGHT_ARROW);
}

// Test: Escape sequences (\n, \t, \\)
TEST_F(InputSourceTest, ParseEscapeSequences) {
    // Note: In the file, we write literal \n as two characters '\' and 'n'
    std::string content = "\\n\n\\t\n\\\\\n";
    std::string filename = create_temp_file(content);

    FileInputSource source(filename);
    EXPECT_EQ(source.read_key(), '\n');
    EXPECT_EQ(source.read_key(), '\t');
    EXPECT_EQ(source.read_key(), '\\');
}

// Test: Empty file contains no valid keystrokes
TEST_F(InputSourceTest, EmptyFileError) {
    std::string content = "";
    std::string filename = create_temp_file(content);

    FileInputSource source(filename);
    EXPECT_FALSE(source.has_input());
    EXPECT_FALSE(source.get_error().empty());
    EXPECT_NE(source.get_error().find("no valid keystrokes"), std::string::npos);
}

// Test: File with only comments and empty lines
TEST_F(InputSourceTest, FileWithOnlyCommentsError) {
    std::string content = "# Comment 1\n\n# Comment 2\n\n";
    std::string filename = create_temp_file(content);

    FileInputSource source(filename);
    EXPECT_FALSE(source.has_input());
    EXPECT_FALSE(source.get_error().empty());
}
