/* 2025 - 2026 supertoq
 * LICENSE: BSD 2-Clause "Simplified"
 *
 * time_stamp.h
 *
 * Version 2026-01-02
 */
#pragma once

#include <glib.h>

/**
 * Lokale Zeit als String:
 * Format: "YYYY-MM-DD HH:MM:SS.mmm"
 * 
 * @return Pointer auf statischen String, nicht freigeben !!
 */
const char *time_stamp(void);
