/* 2025 - 2026 super-toq
 * LICENSE: BSD 2-Clause "Simplified"
 *
 * stby_prev.h – Schnittstelle für Standby‑Prevention 
 * Version 2026-01-14
 *
 */
#ifndef STBY_PREV_H
#define STBY_PREV_H

#include <glib.h>
#include <gio/gio.h>   // für GError

/* ----- Globale Strukture ------------------------------------ */
typedef enum {
    DESKTOP_UNKNOWN,
    DESKTOP_GNOME,
    DESKTOP_KDE,
    DESKTOP_XFCE,
    DESKTOP_MATE
} DesktopEnvironment;

/* Umgebung feststellen */
DesktopEnvironment detect_desktop(void);

/* GNOME inhibit */
void   start_gnome_inhibit(void);
gboolean stop_gnome_inhibit(GError **error);

/* Systemd/KDE inhibit */
void   start_system_inhibit(void);
gboolean stop_system_inhibit(GError **error);

#endif /* STBY_PREV_H */
