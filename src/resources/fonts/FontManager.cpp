#include "FontManager.h"
#include "FontDefinitions.h"

static FontFamily* currentFamily = &bookerlyFamily;

FontFamily* getCurrentFontFamily() {
    return currentFamily;
}

void setCurrentFontFamily(FontFamily* family) {
    if (family)
        currentFamily = family;
}
