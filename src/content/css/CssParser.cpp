#include "CssParser.h"

CssParser::CssParser() {}

CssParser::~CssParser() {}

bool CssParser::parseFile(const char* filepath) {
  File file = SD.open(filepath);
  if (!file) {
    Serial.printf("CssParser: Failed to open %s\n", filepath);
    return false;
  }

  // Read entire file into string (CSS files are usually small)
  String cssContent;
  cssContent.reserve(file.size());
  while (file.available()) {
    cssContent += (char)file.read();
  }
  file.close();

  return parseString(cssContent);
}

bool CssParser::parseString(const String& cssContent) {
  size_t pos = 0;
  size_t len = cssContent.length();

  while (pos < len) {
    // Skip whitespace and comments
    pos = skipWhitespaceAndComments(cssContent, pos);
    if (pos >= len)
      break;

    // Skip @rules (like @page, @media) - we don't support them yet
    if (cssContent.charAt(pos) == '@') {
      // Find the end of the @rule
      int braceCount = 0;
      bool foundBrace = false;
      while (pos < len) {
        char c = cssContent.charAt(pos);
        if (c == '{') {
          braceCount++;
          foundBrace = true;
        } else if (c == '}') {
          braceCount--;
          if (braceCount == 0 && foundBrace) {
            pos++;
            break;
          }
        } else if (c == ';' && !foundBrace) {
          // Simple @rule like @import
          pos++;
          break;
        }
        pos++;
      }
      continue;
    }

    // Find selector(s)
    size_t selectorEnd = findSelectorEnd(cssContent, pos);
    if (selectorEnd >= len || selectorEnd <= pos)
      break;

    String selector = cssContent.substring(pos, selectorEnd);
    selector.trim();

    // Skip the '{'
    pos = selectorEnd + 1;

    // Find the end of the rule block
    size_t ruleEnd = findRuleEnd(cssContent, pos);
    if (ruleEnd >= len)
      break;

    String properties = cssContent.substring(pos, ruleEnd);
    properties.trim();

    // Parse the rule
    if (selector.length() > 0 && properties.length() > 0) {
      parseRule(selector, properties);
    }

    // Skip the '}'
    pos = ruleEnd + 1;
  }

  Serial.printf("CssParser: Loaded %d style rules\n", styleMap_.size());
  return true;
}

const CssStyle* CssParser::getStyleForClass(const String& className) const {
  auto it = styleMap_.find(className);
  if (it != styleMap_.end()) {
    return &it->second;
  }
  return nullptr;
}

CssStyle CssParser::getCombinedStyle(const String& classNames) const {
  CssStyle combined;

  // Split class names by whitespace
  int start = 0;
  int len = classNames.length();

  while (start < len) {
    // Skip leading whitespace
    while (start < len &&
           (classNames.charAt(start) == ' ' || classNames.charAt(start) == '\t' || classNames.charAt(start) == '\n')) {
      start++;
    }
    if (start >= len)
      break;

    // Find end of class name
    int end = start;
    while (end < len && classNames.charAt(end) != ' ' && classNames.charAt(end) != '\t' &&
           classNames.charAt(end) != '\n') {
      end++;
    }

    if (end > start) {
      String className = classNames.substring(start, end);
      const CssStyle* style = getStyleForClass(className);
      if (style) {
        combined.merge(*style);
      }
    }

    start = end;
  }

  return combined;
}

void CssParser::parseRule(const String& selector, const String& properties) {
  // Parse the selector - handle comma-separated selectors
  int start = 0;
  int len = selector.length();

  while (start < len) {
    // Find next comma or end
    int end = selector.indexOf(',', start);
    if (end < 0)
      end = len;

    String singleSelector = selector.substring(start, end);
    singleSelector.trim();

    if (singleSelector.length() > 0) {
      // Extract class name from selector
      String className = extractClassName(singleSelector);

      if (className.length() > 0) {
        // Parse properties
        CssStyle style;

        // Split properties by semicolon
        int propStart = 0;
        int propLen = properties.length();

        while (propStart < propLen) {
          int propEnd = properties.indexOf(';', propStart);
          if (propEnd < 0)
            propEnd = propLen;

          String prop = properties.substring(propStart, propEnd);
          prop.trim();

          if (prop.length() > 0) {
            // Split property into name and value
            int colonPos = prop.indexOf(':');
            if (colonPos > 0) {
              String propName = prop.substring(0, colonPos);
              String propValue = prop.substring(colonPos + 1);
              propName.trim();
              propValue.trim();

              // Convert to lowercase for comparison
              propName.toLowerCase();

              parseProperty(propName, propValue, style);
            }
          }

          propStart = propEnd + 1;
        }

        // Store style if it has any supported properties
        if (style.hasTextAlign) {
          // Merge with existing style if present
          auto it = styleMap_.find(className);
          if (it != styleMap_.end()) {
            it->second.merge(style);
          } else {
            styleMap_[className] = style;
          }
        }
      }
    }

    start = end + 1;
  }
}

void CssParser::parseProperty(const String& name, const String& value, CssStyle& style) {
  if (name == "text-align") {
    style.textAlign = parseTextAlign(value);
    style.hasTextAlign = true;
  }
  // Add more property parsing here as needed:
  // else if (name == "font-style") { ... }
  // else if (name == "font-weight") { ... }
}

TextAlign CssParser::parseTextAlign(const String& value) {
  String v = value;
  v.toLowerCase();
  v.trim();

  if (v == "left" || v == "start") {
    return TextAlign::Left;
  } else if (v == "right" || v == "end") {
    return TextAlign::Right;
  } else if (v == "center") {
    return TextAlign::Center;
  } else if (v == "justify") {
    return TextAlign::Justify;
  }

  // Default to left
  return TextAlign::Left;
}

size_t CssParser::skipWhitespaceAndComments(const String& css, size_t pos) {
  size_t len = css.length();

  while (pos < len) {
    char c = css.charAt(pos);

    // Skip whitespace
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
      pos++;
      continue;
    }

    // Skip comments /* ... */
    if (c == '/' && pos + 1 < len && css.charAt(pos + 1) == '*') {
      pos += 2;
      while (pos + 1 < len) {
        if (css.charAt(pos) == '*' && css.charAt(pos + 1) == '/') {
          pos += 2;
          break;
        }
        pos++;
      }
      continue;
    }

    // Not whitespace or comment
    break;
  }

  return pos;
}

size_t CssParser::findSelectorEnd(const String& css, size_t pos) {
  size_t len = css.length();

  while (pos < len) {
    char c = css.charAt(pos);
    if (c == '{') {
      return pos;
    }
    pos++;
  }

  return len;
}

size_t CssParser::findRuleEnd(const String& css, size_t pos) {
  size_t len = css.length();
  int braceCount = 1;  // We're already inside one '{'

  while (pos < len) {
    char c = css.charAt(pos);
    if (c == '{') {
      braceCount++;
    } else if (c == '}') {
      braceCount--;
      if (braceCount == 0) {
        return pos;
      }
    }
    pos++;
  }

  return len;
}

String CssParser::extractClassName(const String& selector) {
  // Find the class selector (starts with '.')
  int dotPos = selector.indexOf('.');
  if (dotPos < 0) {
    return String("");  // No class selector
  }

  // Extract class name (everything after '.' until end or special char)
  int start = dotPos + 1;
  int end = start;
  int len = selector.length();

  while (end < len) {
    char c = selector.charAt(end);
    // Class name can contain letters, digits, hyphens, underscores
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_') {
      end++;
    } else {
      break;
    }
  }

  if (end > start) {
    return selector.substring(start, end);
  }

  return String("");
}
