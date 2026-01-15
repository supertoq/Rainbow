/* 2025 - 2026 super-toq
 * LICENSE: BSD 2-Clause "Simplified"
 *
 * log_file.h
 *
 * Version 2026-01-05 created in Allstedt
 */
#pragma once

#include <glib.h>

/* Initialisiert das Logging-System */
void log_file_init(const gchar *app_name);

/* Logfolder immer erstellen */
void log_folder_init(void);

/* Beendet Logging, Datei schließen */
void log_file_shutdown(void);
