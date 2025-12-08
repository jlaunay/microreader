#ifndef CSS_STYLE_H
#define CSS_STYLE_H

#include <Arduino.h>

/**
 * Text alignment values supported by the reader
 */
enum class TextAlign {
  Left,    // Default left alignment
  Right,   // Right alignment
  Center,  // Center alignment
  Justify  // Justified text (both edges aligned)
};

/**
 * CssStyle - Represents supported CSS properties for a selector
 *
 * This structure holds the subset of CSS properties that the reader supports.
 * Currently supported:
 * - text-align: left, right, center, justify
 *
 * Properties can be extended in the future to support:
 * - font-style (italic)
 * - font-weight (bold)
 * - text-indent
 * - margin-top/bottom (for paragraph spacing)
 */
struct CssStyle {
  TextAlign textAlign = TextAlign::Left;
  bool hasTextAlign = false;  // True if text-align was explicitly set

  // Merge another style into this one (other style takes precedence)
  void merge(const CssStyle& other) {
    if (other.hasTextAlign) {
      textAlign = other.textAlign;
      hasTextAlign = true;
    }
  }

  // Reset to default values
  void reset() {
    textAlign = TextAlign::Left;
    hasTextAlign = false;
  }
};

/**
 * ActiveStyle - Tracks the currently active style during parsing
 *
 * This is used by the word provider to track what styles are in effect
 * as elements are entered and exited.
 */
struct ActiveStyle {
  CssStyle style;
  bool isBlockElement = false;  // True if this style came from a block element
};

#endif
