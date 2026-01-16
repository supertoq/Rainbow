/* 2025 - 2026 supertoq
 * LICENSE: BSD 2-Clause "Simplified"
 *
 * detect_desktop();
 * start_gnome_inhibit();
 * start_system_inhibit();
 * stop_gnome_inhibit(error);
 * stop_system_inhibit(error);
 * 
 * Standby-Prevention-Logik
 * stby_prev.c
 * Version 2.1 2026-01-14
 */
#define APP_ID         "io.github.supertoq.rainbow"
#define APP_NAME       "supertoq-rainbow"

#include <dbus/dbus.h>            // für DBusConnection,DBusMessage (dbus_* , sqrt )
#include <stdint.h>               // Ganzzahl-Typen
#include <unistd.h>               // POSIX-Header
#include <string.h>               // für strstr() in Desktopumgebung;

#include "stby_prev.h"            // die öffentliche Schnittstelle hierfür
//#include "config.h"               // für g_cfg. (für spätere Implementierungen
#include "time_stamp.h"           // Zeitstempel
#include "log_file.h"             // g_print logging

/* --- Globale Variablen für Inhibit --- */
static uint32_t gnome_cookie = 0; // GNOME-Inhibit uint32-Cookie, geliefert von org.freedesktop.ScreenSaver.Inhibit;
static int system_fd = -1;        // systemd/KDE-Inhibit (fd = File Descriptor/Verbindung zu einem Systemdienst)
                                  // geliefert von org.freedesktop.login1.Manager.Inhibit;


/* ----- Desktop‑Erkennung ---------------------------------------- */
DesktopEnvironment detect_desktop(void)
{
    const char *desktop = g_getenv("XDG_CURRENT_DESKTOP");
    if (!desktop) desktop = g_getenv("DESKTOP_SESSION");
    if (!desktop) return DESKTOP_UNKNOWN;

    g_autofree gchar *upper = g_ascii_strup(desktop, -1);
    if (strstr(upper, "GNOME")) return DESKTOP_GNOME;
    if (strstr(upper, "KDE"))   return DESKTOP_KDE;
    if (strstr(upper, "XFCE"))  return DESKTOP_XFCE;
    if (strstr(upper, "MATE"))  return DESKTOP_MATE;
    return DESKTOP_UNKNOWN;
}

/* ----- GNOME ScreenSaver Inhibit ---------------------------------- */
void start_gnome_inhibit(void)
{
    DBusError err = DBUS_ERROR_INIT;
    /* Session-Bus */
    DBusConnection *conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (!conn || dbus_error_is_set(&err)) {
        g_warning("[GNOME‑INHIBIT] DBus error: %s\n", err.message);
        dbus_error_free(&err);
        return;
    }

    /* DBus-Auffruf vorbereiten */
    DBusMessage *msg = dbus_message_new_method_call(
        "org.freedesktop.ScreenSaver",
        "/ScreenSaver",
        "org.freedesktop.ScreenSaver",
        "Inhibit"
    );

    if (!msg) 
    {
        g_warning("[GNOME-INHIBIT] Failed to allocate DBus message (1)\n");
        return;
    }

    const char *app    = APP_NAME;
    const char *reason = "Prevent Standby";
    DBusMessageIter args;
    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &app);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &reason);

    /* Nachricht senden */
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
    if (!reply) 
    {
        g_warning("[GNOME-INHIBIT] No reply: %s\n", err.message);
        dbus_error_free(&err);
        dbus_message_unref(msg);
        return;
    }

    /* Antwort auslesen (COOKIE als uint32) */
    DBusMessageIter iter;
    if (dbus_message_iter_init(reply, &iter) &&
        dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_UINT32) {
        dbus_message_iter_get_basic(&iter, &gnome_cookie);
        g_print("[%s] [GNOME-INHIBIT] Activated (cookie=%u)\n", time_stamp(), gnome_cookie);
    } else {
        g_warning("[GNOME-INHIBIT] Invalid reply (no cookie)\n");
    }

    dbus_message_unref(msg);
    dbus_message_unref(reply);
}


/* ----- Stop Gnome Inhibit ----------------------------------------- */
gboolean stop_gnome_inhibit(GError **error)
{ (void)error; // bewusst inaktiv

    if (!gnome_cookie) return TRUE;   /* nichts zu beenden */

    DBusError err = DBUS_ERROR_INIT;
    DBusConnection *conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (!conn || dbus_error_is_set(&err)) 
    {
        g_warning("[GNOME-INHIBIT] DBus error: %s\n", err.message);
        dbus_error_free(&err);
        return FALSE;
    }

    DBusMessage *msg = dbus_message_new_method_call(
        "org.freedesktop.ScreenSaver",
        "/ScreenSaver",
        "org.freedesktop.ScreenSaver",
        "UnInhibit"
    );

    if (!msg) {
        g_warning("[GNOME-INHIBIT] Failed to allocate DBus message (2)\n");
        return FALSE;
    }

    DBusMessageIter args;
    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT32, &gnome_cookie);

    dbus_connection_send(conn, msg, NULL);
    dbus_message_unref(msg);

    g_print("[%s] [GNOME-INHIBIT] Deactivated (cookie=%u)\n", time_stamp(), gnome_cookie);
    gnome_cookie = 0;
    return TRUE;
}

/* ----- systemd/KDE login1.Manager Inhibit ------------------------- */
void start_system_inhibit(void)
{
    DBusError err = DBUS_ERROR_INIT;
    DBusConnection *conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (!conn || dbus_error_is_set(&err)) 
    {
        g_warning("[SYSTEM-INHIBIT] DBus error: %s\n", err.message);
        dbus_error_free(&err);
        return;
    }

    DBusMessage *msg = dbus_message_new_method_call(
        "org.freedesktop.login1",
        "/org/freedesktop/login1",
        "org.freedesktop.login1.Manager",
        "Inhibit");
    if (!msg) 
    {
        g_warning("[SYSTEM-INHIBIT] Failed to allocate DBus message\n");
        return;
    }

    const char *what = "sleep:idle:shutdown:handle-lid-switch:handle-suspend-key";
    const char *who  = "Rainbow";
    const char *why  = "Prevent Standby";
    const char *mode = "block";

    DBusMessageIter args;
    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &what);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &who);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &why);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &mode);

    /* Methode senden und Antwort empfangen */
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
    if (!reply) 
    {
        g_warning("[SYSTEM-INHIBIT] Inhibit failed: %s\n", err.message);
        dbus_error_free(&err);
        dbus_message_unref(msg);
        return;
    }

    DBusMessageIter iter;
    if (dbus_message_iter_init(reply, &iter) &&
        dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_UNIX_FD) 
    {
        dbus_message_iter_get_basic(&iter, &system_fd);
        g_print("[%s] [SYSTEM-INHIBIT] Activated (fd=%d)\n", time_stamp(), system_fd);
    } else {
        g_warning("[SYSTEM-INHIBIT] Invalid reply (no fd)\n");
    }
    /* Aufräumen */
    dbus_message_unref(msg);
    dbus_message_unref(reply);
}

/* ----- Stop System Inhibit ---------------------------------------- */
gboolean stop_system_inhibit(GError **error)
{
    (void)error;   /* Parameter bleibt aus Kompatibilitäts‑Gründen */

    if (system_fd < 0) return FALSE;   /* kein gültiger FD */

    close(system_fd);
    g_print("[%s] [SYSTEM-INHIBIT] Deactivated (fd=%d)\n",
            time_stamp(), system_fd);
    system_fd = -1;
    return TRUE;
}

