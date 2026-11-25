#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <filesystem>
#include <vector>
#include "FileManager.h"

namespace fs = std::filesystem;

static int g_passCount = 0;
static int g_failCount = 0;

void EXPECT(bool condition, const std::string& caseName) {
    if (condition) {
        std::cout << "[PASS] " << caseName << std::endl;
        g_passCount++;
    } else {
        std::cerr << "[FAIL] " << caseName << std::endl;
        g_failCount++;
    }
}

std::string readFileContent(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return "";
    return std::string((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
}

int main() {
    fs::path tempDir = fs::temp_directory_path() / "mmedit_test_save";
    fs::create_directories(tempDir);

    std::cout << "Running saveFile tests in temp dir: " << tempDir.string() << std::endl;

    // Test 1: Basic UTF-8 content with Chinese & emoji
    {
        fs::path p = tempDir / "test_utf8.md";
        std::string content = "# 标题\n这是中文测试内容，包含特殊符号：⭐🎉！\nEnd.";
        bool ok = saveFile(p, content);
        EXPECT(ok, "Save UTF-8 and Chinese content");
        EXPECT(fs::exists(p), "UTF-8 file exists");
        EXPECT(readFileContent(p) == content, "UTF-8 content exactly matches");
    }

    // Test 2: Empty string
    {
        fs::path p = tempDir / "test_empty.md";
        bool ok = saveFile(p, "");
        EXPECT(ok, "Save empty content");
        EXPECT(fs::exists(p), "Empty file exists");
        EXPECT(fs::file_size(p) == 0, "Empty file size is 0");
    }

    // Test 3: Overwrite with shorter content (truncation check)
    {
        fs::path p = tempDir / "test_overwrite.md";
        std::string longContent = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
        saveFile(p, longContent);
        std::string shortContent = "BBB";
        bool ok = saveFile(p, shortContent);
        EXPECT(ok, "Overwrite with shorter content");
        EXPECT(fs::file_size(p) == shortContent.size(), "Overwritten file size matches new content size");
        EXPECT(readFileContent(p) == shortContent, "Overwritten file content strictly matches");
    }

    // Test 4: Binary preservation (CRLF and LF)
    {
        fs::path p = tempDir / "test_crlf.md";
        std::string mixedLineEndings = "Line1\r\nLine2\nLine3\r\nLine4";
        bool ok = saveFile(p, mixedLineEndings);
        EXPECT(ok, "Save binary line endings (CRLF + LF)");
        EXPECT(readFileContent(p) == mixedLineEndings, "Line endings preserved without unwanted conversion");
    }

    // Test 5: Large file content (1MB)
    {
        fs::path p = tempDir / "test_large.md";
        std::string large(1024 * 1024, 'X');
        bool ok = saveFile(p, large);
        EXPECT(ok, "Save 1MB content");
        EXPECT(fs::file_size(p) == 1024 * 1024, "1MB file size matches");
        EXPECT(readFileContent(p) == large, "1MB content verified");
    }

    // Test 6: Invalid directory (non-existent folder)
    {
        fs::path p = tempDir / "not_exist_subfolder_123" / "sub" / "file.md";
        bool ok = saveFile(p, "data");
        EXPECT(!ok, "Save to non-existent directory returns false");
    }

    // Test 7: Unicode path / folder with spaces & Chinese
    {
        fs::path unicodeDir = tempDir / "测试 目录 带空格";
        fs::create_directories(unicodeDir);
        fs::path p = unicodeDir / "测试笔记.md";
        std::string content = "Unicode path test passed";
        bool ok = saveFile(p, content);
        EXPECT(ok, "Save to Unicode/Chinese path with spaces");
        EXPECT(readFileContent(p) == content, "Unicode path content matches");
    }

    // Cleanup
    std::error_code ec;
    fs::remove_all(tempDir, ec);

    std::cout << "\n================ Summary ================" << std::endl;
    std::cout << "Total passed: " << g_passCount << std::endl;
    std::cout << "Total failed: " << g_failCount << std::endl;

    return g_failCount == 0 ? 0 : 1;
}
