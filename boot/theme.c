/*
 * theme.c
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#include "theme.h"
#include "compat.h"
#include "localization.h"

static struct BootMenuTheme current_theme = {
    .background_color = 0x1A1A2E,
    .header_color = 0x2D2D4F,
    .highlight_color = 0x4A4A8A,
    .text_color = 0xCCCCCC,
    .selected_text_color = 0xFFFFFF,
    .footer_color = 0x8888AA,
    .background_image = NULL
};

void SetBootMenuTheme(const struct BootMenuTheme* theme) {
    memcpy(&current_theme, theme, sizeof(struct BootMenuTheme));
}
const struct BootMenuTheme* GetBootMenuTheme(void) {
    return &current_theme;
}

/**
 * Load theme and language configuration from config files
 */
void LoadThemeAndLanguageFromConfig(void) {
    // Language is applied by main via SetLanguage(config.language)
    // Theme defaults are already set in current_theme. Extend here if future
    // theme keys are parsed from central configuration.
}