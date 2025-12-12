/**
 * XhtmlToTxtConversionTest.cpp - XHTML to Plain Text Conversion Test
 *
 * Tests the EpubWordProvider's XHTML to plain text conversion logic.
 * Uses a test HTML file to verify correct handling of:
 * - Block elements (div, p, etc.)
 * - Empty block elements
 * - &nbsp; for intentional blank lines
 * - <br/> handling
 * - Whitespace normalization
 */

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "content/providers/EpubWordProvider.h"
#include "content/xml/SimpleXmlParser.h"
#include "test_globals.h"
#include "test_utils.h"

namespace fs = std::filesystem;

// Path to test HTML file
const char* TEST_HTML_PATH = "C:/Users/Patrick/Desktop/microreader/resources/books/test.html";

/**
 * Read entire file contents as string
 */
std::string readFileContents(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    return "";
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

/**
 * Print string with visible whitespace markers
 */
void printWithMarkers(const std::string& text) {
  std::cout << "--- Output (with markers) ---\n";
  for (size_t i = 0; i < text.length(); i++) {
    char c = text[i];
    if (c == '\n') {
      std::cout << "\\n\n";  // Show newline marker then actual newline
    } else if (c == ' ') {
      std::cout << "\xB7";  // Middle dot for space
    } else if (c == '\t') {
      std::cout << "\\t";
    } else {
      std::cout << c;
    }
  }
  std::cout << "\n--- End Output ---\n";
}

/**
 * Count occurrences of a substring
 */
int countOccurrences(const std::string& text, const std::string& substr) {
  int count = 0;
  size_t pos = 0;
  while ((pos = text.find(substr, pos)) != std::string::npos) {
    count++;
    pos += substr.length();
  }
  return count;
}

/**
 * Test: Convert test.html to plain text
 */
void testConversion(TestUtils::TestRunner& runner) {
  std::cout << "\n=== Test: XHTML to TXT Conversion ===\n";

  // Check test file exists
  if (!fs::exists(TEST_HTML_PATH)) {
    std::cout << "ERROR: Test file not found: " << TEST_HTML_PATH << "\n";
    runner.expectTrue(false, "Test file should exist");
    return;
  }

  std::cout << "Input file: " << TEST_HTML_PATH << "\n";

  // Create EpubWordProvider with the test HTML (direct XHTML mode)
  EpubWordProvider provider(TEST_HTML_PATH);

  if (!provider.isValid()) {
    std::cout << "ERROR: Failed to create EpubWordProvider\n";
    runner.expectTrue(false, "Provider should be valid");
    return;
  }

  std::cout << "Provider created successfully\n";

  // The provider creates a .txt file next to the input
  std::string expectedTxtPath = TEST_HTML_PATH;
  size_t dotPos = expectedTxtPath.rfind('.');
  if (dotPos != std::string::npos) {
    expectedTxtPath = expectedTxtPath.substr(0, dotPos) + ".txt";
  }

  std::cout << "Expected output: " << expectedTxtPath << "\n";

  // Read the converted output
  std::string output = readFileContents(expectedTxtPath);

  if (output.empty()) {
    std::cout << "ERROR: Output file is empty or not found\n";
    runner.expectTrue(false, "Output should not be empty");
    return;
  }

  std::cout << "\n--- Raw Output ---\n";
  std::cout << output;
  std::cout << "\n--- End Raw Output ---\n\n";

  printWithMarkers(output);

  // ========== VALIDATION ==========
  std::cout << "\n=== Validation ===\n";

  // 1. Should contain "Das Buch"
  bool hasDasBuch = output.find("Das Buch") != std::string::npos;
  std::cout << "Contains 'Das Buch': " << (hasDasBuch ? "YES" : "NO") << "\n";
  runner.expectTrue(hasDasBuch, "Output should contain 'Das Buch'");

  // 2. Should contain "Los Angeles"
  bool hasLosAngeles = output.find("Los Angeles") != std::string::npos;
  std::cout << "Contains 'Los Angeles': " << (hasLosAngeles ? "YES" : "NO") << "\n";
  runner.expectTrue(hasLosAngeles, "Output should contain 'Los Angeles'");

  // 3. Should contain "Neal Stephenson"
  bool hasNealStephenson = output.find("Neal Stephenson") != std::string::npos;
  std::cout << "Contains 'Neal Stephenson': " << (hasNealStephenson ? "YES" : "NO") << "\n";
  runner.expectTrue(hasNealStephenson, "Output should contain 'Neal Stephenson'");

  // 4. Should NOT contain content from <head> or <style>
  bool hasStyleContent = output.find("margin-bottom") != std::string::npos;
  std::cout << "Contains style content: " << (hasStyleContent ? "YES (BAD)" : "NO (GOOD)") << "\n";
  runner.expectTrue(!hasStyleContent, "Output should NOT contain style content");

  // 5. Count newlines to verify structure
  int newlineCount = countOccurrences(output, "\n");
  std::cout << "Newline count: " << newlineCount << "\n";

  // 6. Check for blank line between "Das Buch" and "Los Angeles" (from &nbsp; div)
  // Pattern: "Das Buch\n\nLos Angeles" (two newlines = one blank line)
  // But with current test.html: Das Buch -> empty div with nbsp -> Los Angeles div
  // So we expect: "Das Buch\n" + blank line + "Los Angeles..."

  // Find positions
  size_t dasBuchPos = output.find("Das Buch");
  size_t losAngelesPos = output.find("Los Angeles");

  if (dasBuchPos != std::string::npos && losAngelesPos != std::string::npos) {
    std::string between = output.substr(dasBuchPos + 8, losAngelesPos - dasBuchPos - 8);
    int newlinesBetween = countOccurrences(between, "\n");
    std::cout << "Newlines between 'Das Buch' and 'Los Angeles': " << newlinesBetween << "\n";
    std::cout << "Text between (escaped): '";
    for (char c : between) {
      if (c == '\n')
        std::cout << "\\n";
      else if (c == ' ')
        std::cout << " ";
      else
        std::cout << c;
    }
    std::cout << "'\n";

    // Should have 2 newlines (blank line = div close + nbsp div close)
    runner.expectTrue(newlinesBetween == 2, "Should have blank line (2 newlines) between Das Buch and Los Angeles");
  }

  // 7. Check for blank line between Los Angeles paragraph and Neal Stephenson paragraph
  size_t nealPos = output.find("Neal Stephenson");
  size_t drohtPos = output.find("droht.");  // End of Los Angeles paragraph

  if (drohtPos != std::string::npos && nealPos != std::string::npos) {
    std::string between2 = output.substr(drohtPos + 6, nealPos - drohtPos - 6);
    int newlinesBetween2 = countOccurrences(between2, "\n");
    std::cout << "Newlines between paragraphs: " << newlinesBetween2 << "\n";
    std::cout << "Text between (escaped): '";
    for (char c : between2) {
      if (c == '\n')
        std::cout << "\\n";
      else if (c == ' ')
        std::cout << " ";
      else
        std::cout << c;
    }
    std::cout << "'\n";

    // Should have 2 newlines (blank line from &nbsp;<br/> div)
    runner.expectTrue(newlinesBetween2 == 2, "Should have blank line (2 newlines) between paragraphs");
  }

  // 8. Should NOT have trailing empty lines at the end (from empty mbppagebreak div)
  if (!output.empty()) {
    // Count trailing newlines
    int trailingNewlines = 0;
    for (int i = output.length() - 1; i >= 0 && output[i] == '\n'; i--) {
      trailingNewlines++;
    }
    std::cout << "Trailing newlines: " << trailingNewlines << "\n";
    runner.expectTrue(trailingNewlines <= 1, "Should have at most 1 trailing newline");
  }
}

int main() {
  std::cout << "========================================\n";
  std::cout << "XHTML to TXT Conversion Test\n";
  std::cout << "========================================\n";

  TestUtils::TestRunner runner("XhtmlToTxtConversion");

  testConversion(runner);

  std::cout << "\n========================================\n";
  runner.printSummary();
  std::cout << "========================================\n";

  return runner.allPassed() ? 0 : 1;
}
