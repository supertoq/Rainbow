/* 2025 - 2026 supertoq
 * LICENSE: BSD 2-Clause "Simplified"
 *
 * time_stamp.c
 *
 * Format: 
 * "YYYY-MM-DD HH:MM:SS.mmm"
 *
 * g_print("[%s] text\n", time_stamp());
 *
 * Version 2026-01-02
 */
#include "time_stamp.h"
#include <glib.h>

const char *time_stamp(void)
{
    static char buf[32]; // Puffer für Rückgabe

    /* aktuelle lokale Zeit */
    GDateTime *now = g_date_time_new_now_local();

    /* Formatierung in "buf" */
    gchar *time_format = g_date_time_format(now, "%Y-%m-%d %H:%M:%S"); // ohne Millisekunden
    g_strlcpy(buf, time_format, sizeof(buf));               // in statischen Puffer kopieren

/* Info:  g_strlcpy - Kopiert so viele Zeichen wie passen und setzt Nullterminator "\0" am Ende;
    gsize g_strlcpy(char *Ziel, const char *Quelle, gsize Zielpuffergröße);  */

    /* freigeben */
    g_free(time_format);
    g_date_time_unref(now);

    return buf;
}
