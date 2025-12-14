#include <Arduino.h>

#include "Bookerly26.h"
#include "Bookerly26Bold.h"
#include "Bookerly26BoldItalic.h"
#include "Bookerly26Italic.h"
#include "Font14.h"
#include "Font27.h"
#include "NotoSans26.h"
#include "NotoSans26Bold.h"
#include "NotoSans26BoldItalic.h"
#include "NotoSans26Italic.h"

// Font definitions
const SimpleGFXfont Font14 = {Font14Bitmaps, nullptr, nullptr, Font14Glyphs, 305, 16, "Font14", 14, FontStyle::REGULAR};
const SimpleGFXfont Font27 = {Font27Bitmaps, nullptr, nullptr, Font27Glyphs, 305, 29, "Font27", 27, FontStyle::REGULAR};

// Font families (group variants together)
FontFamily notoSansFamily = {
    "NotoSans",
    &NotoSans26,           // regular
    &NotoSans26Bold,       // bold
    &NotoSans26Italic,     // italic
    &NotoSans26BoldItalic  // boldItalic
};

FontFamily bookerlyFamily = {
    "Bookerly",
    &Bookerly26,           // regular
    &Bookerly26Bold,       // bold
    &Bookerly26Italic,     // italic
    &Bookerly26BoldItalic  // boldItalic
};

// Example: Font14 family
FontFamily font14Family = {"Font14", &Font14, nullptr, nullptr, nullptr};

// Example: Font27 family
FontFamily font27Family = {"Font27", &Font27, nullptr, nullptr, nullptr};
