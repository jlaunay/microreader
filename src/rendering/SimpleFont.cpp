#include "SimpleFont.h"

// Helper to initialize the glyph map for a font
void initFontGlyphMap(const SimpleGFXfont* font) {
  if (!font || font->glyphCount == 0) {
    return;
  }

  // Create the map if it doesn't exist
  if (!font->glyphMap) {
    font->glyphMap = new std::unordered_map<uint32_t, uint16_t>();
  }

  // Clear and rebuild
  font->glyphMap->clear();
  font->glyphMap->reserve(font->glyphCount);

  for (uint16_t i = 0; i < font->glyphCount; i++) {
    (*font->glyphMap)[font->glyph[i].codepoint] = i;
  }
}

// Helper to initialize glyph maps for all fonts in a family
void initFontFamilyGlyphMaps(const FontFamily* family) {
  if (!family)
    return;

  if (family->regular)
    initFontGlyphMap(family->regular);
  if (family->bold)
    initFontGlyphMap(family->bold);
  if (family->italic)
    initFontGlyphMap(family->italic);
  if (family->boldItalic)
    initFontGlyphMap(family->boldItalic);
}

// New: Helper to get a font variant from a family (returns nullptr if not available)
const SimpleGFXfont* getFontVariant(const FontFamily* family, FontStyle style) {
  if (!family) {
    return nullptr;
  }

  switch (style) {
    case FontStyle::REGULAR:
      return family->regular;
    case FontStyle::BOLD:
      return family->bold ? family->bold : family->regular;  // Fallback to regular
    case FontStyle::ITALIC:
      return family->italic ? family->italic : family->regular;
    case FontStyle::BOLD_ITALIC:
      return family->boldItalic ? family->boldItalic : (family->bold ? family->bold : family->regular);
    default:
      return family->regular;
  }
}