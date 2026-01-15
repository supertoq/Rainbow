/* Copyright (c) 2025 - 2026 super-toq
 * LICENSE: BSD 2-Clause "Simplified"
 * 
 *
 * gcc $(pkg-config --cflags gtk4 libadwaita-1 dbus-1) -o rainbow main.c free.toq.rainbow.gresource.c $(pkg-config --libs gtk4 libadwaita-1 dbus-1)
 *
 * Please note:
 * DON'T USE ON OLED DISPLAY !
 * The Use of this code and execution of the applications is at your own risk, I accept no liability!
 * 
 * free.toq.rainbow basierend auf "Basis OLED-Saver 1.1.3" nur mit bunten Farben ;-) 
 */
#define APP_VERSION    "1.0.3"//_1
#define APP_ID         "free.toq.rainbow"
#define APP_NAME       "Rainbow"
#define APP_DOMAINNAME "toq-rainbow"
/* Fenstergröße Breite, Höche ( 370, 410) */
#define WIN_WIDTH      370
#define WIN_HEIGHT     410

#include <glib.h>
#include <gtk/gtk.h>
#include <adwaita.h>
#include "icon-gresource.h" // binäre Icons
#include <string.h>         // für strstr()
#include <unistd.h>         // POSIX-Header
#include <locale.h>         // für setlocale(LC_ALL, "")
#include <glib/gi18n.h>     // für _();

#include "time_stamp.h"     // Zeitstempel
#include "stby_prev.h"      // Standby Verhinderung

/* ----- Globale Strukturen ------------------------------------ */
typedef struct {
    GtkWidget *btn_1;
    GtkWidget *btn_2;
    GtkWidget *btn_3;
    GtkWidget *btn_4;
    int       interval_ms;
    GtkWidget *fullscreen_window;
} IntervalButtons;

typedef struct {
    AdwToastOverlay          *toast_overlay;      // Pointer auf die AdwToastOverlay
} ToastManager;
static ToastManager toast_manager = { NULL };

/* ----- Farben,Index im Array definieren ----- */
static guint colour_timer = 0;
static int current_colour_index = 0;
static const char *colours[] = {
                  "#000000",      // schwarz
                  "#FF0000",      // rot
                  "#00FF00",      // grün 
                  "#0000FF",      // blau 
                  "#FFFF00",      // gelb
                  "#FFFFFF"       // weiß
};

/* ----- Toast Mitteilungen ------------------------------------------ */
static void show_toast(const char *msg)
{
    if (!toast_manager.toast_overlay) // toast_manager siehe Strukt.
        return;

    AdwToast *toast = adw_toast_new(msg);
    adw_toast_set_timeout(toast, 2); // Sekunden
    adw_toast_set_priority(toast, ADW_TOAST_PRIORITY_HIGH);
    adw_toast_overlay_add_toast(toast_manager.toast_overlay, toast);
}

/* ----- Funktionen der Standby-Verhinderung ------------------------- */
// Ausgelagert!
// detect_desktop();
// start_gnome_inhibit();
// start_system_inhibit();
// stop_gnome_inhibit(error);
// stop_system_inhibit(error);

/* --- START-Wrapper --- ausgelöst in on_activate ------------------- */
static void start_standby_prevention(void)
{  // Noch umbauen mit Rückmeldung bei Erfolg, wie STOP !!

    /* Entsprechende Funktion zur Umgebung starten: */
    DesktopEnvironment de = detect_desktop();
    if (de == DESKTOP_GNOME) start_gnome_inhibit();
//    else
    start_system_inhibit(); // KDE, XFCE, MATE

    /* Toast-Message ausgeben: */
    show_toast(_("Standby wird nun verhindert!"));
}

/* --- STOP --- ausgelöst beim shutdown ---------------------------- */
static void stop_standby_prevention(GApplication *app, gpointer user_data)
{

    g_print("Stopping standby prevention\n"); 

    /* Standby-Inhibits beenden */
    stop_gnome_inhibit(NULL);
    stop_system_inhibit(NULL);

    /* Farb-Timer stoppen */
    if (colour_timer > 0) {
        g_source_remove(colour_timer);
        colour_timer = 0;
    }

}

/* ----- Callback: About-Dialog öffnen ------------------------------ */
static void show_about (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    AdwApplication *app = ADW_APPLICATION (user_data);
    /* About‑Dialog anlegen */
    AdwAboutDialog *about = ADW_ABOUT_DIALOG (adw_about_dialog_new ());
    adw_about_dialog_set_application_name (about, APP_NAME);
    adw_about_dialog_set_version (about, APP_VERSION);
    adw_about_dialog_set_developer_name (about, "super-toq");
    adw_about_dialog_set_website (about, "https://github.com/super-toq/rainbow");
    adw_about_dialog_set_comments(about, "<b>Caution:</b>\n"
                                         "Please read this regarding the protection "
                                         "of your hardware and your health!\n\n"
                                         "Do not use on OLED displays! \n"
                                         "Only use it if you know what you're doing. "
                                         "Find out beforehand about any damage to the display!\n"
                                         "The author provides no warranty and assumes no liability "
                                         "for any direct or indirect damages resulting "
                                         "from the use of this software. \n\n"
                                         "<b>Health warning:</b>\nRapid colour changes can cause headaches, "
                                         "dizziness, or light sensitivity in susceptible individuals. "
                                         "Do not look at the flashing image— it may trigger migraine or "
                                         "epilepsy in sensitive users. Look away and turn off the tool "
                                         "immediately if any discomfort occurs.");

    /* Lizenz – BSD2 wird als „custom“ angegeben */
    adw_about_dialog_set_license_type(about, GTK_LICENSE_BSD);
    adw_about_dialog_set_license(about,
                                        "Copyright © 2025, super-toq\n\n"
                                        "This program comes WITHOUT ANY WARRANTY.\n"
                                        "Follow the link to view the license details: "
                                        "<a href=\"https://opensource.org/license/BSD-2-Clause\"><b>BSD 2-Clause License</b></a>\n"
                                        "\n"
                                        "Application Icons by SVG Repo. \n"
                                        "<a href=\"https://www.svgrepo.com\">www.svgrepo.com</a> \n"
                                        "Thanks to SVG Repo for sharing their free icons, "
                                        "we appreciate your generosity and respect your work.\n"
                                        "The icons are licensed under the \n"
                                        "<a href=\"https://www.svgrepo.com/page/licensing/#CC%20Attribution\">"
                                        "Creative Commons Attribution License.</a> \n"
                                        "Colours, shapes, and sizes of the symbols (icons) have been slightly modified from the original, "
                                        "some symbols have been combined with each other.\n"
        );

//    adw_about_dialog_set_translator_credits (about, "toq: deutsch\n toq: englisch");
      adw_about_dialog_set_application_icon (about, "free.toq.rainbow");   //IconName

    /* Setze das Anwendungssymbol von GResource: */

    /* Dialog innerhalb (modal) des Haupt-Fensters anzeigen */
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(
    gtk_application_get_active_window(GTK_APPLICATION(app)) )));
    adw_dialog_present(ADW_DIALOG(about), GTK_WIDGET(parent));

} // Ende About-Dialog

/* ----- Mausbewegung beendet Fullscreen Fenster -------------------- */
     //  aktiviert von enable_mouse_exit_after_delay() 
static gboolean exit_fullscreen(GtkEventControllerMotion *controller, gdouble x, gdouble y, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);

    /* 1. Timer sofort stoppen */
    if (colour_timer > 0) {
        g_source_remove(colour_timer);
        colour_timer = 0;

        // "ref" wieder freigeben: 
        // (welche beim Starten von g_object_ref(fullscreen_window) erstellt wurde)
        g_object_unref(window);
    }

    /* 2. IntervalButton-Referenz zurücksetzen */
    IntervalButtons *ib = g_object_get_data(G_OBJECT(window), "interval_buttons");
    if (ib)
        ib->fullscreen_window = NULL;

    /* 3. Motion-Handler von diesem Controller trennen. [r!] */
    g_signal_handlers_disconnect_by_func(controller, G_CALLBACK(exit_fullscreen), user_data);

    /* 4. Zerstörung, aus dem aktuellen Event-Stack heraus, sicher verschieben */
    g_idle_add((GSourceFunc)gtk_window_destroy, window);

    g_print("Mouse motion exits fullscreen mode\n");

    return TRUE;
}

/* ----- Callback Farben-Timer -------------------------------------- */
static gboolean change_background_colour(gpointer user_data)
{
    GtkWidget *c_widget = GTK_WIDGET(user_data);

    /* wenn Widget nicht mehr sichtbar, Referenz freigeben und Timer stoppen */
    if (!gtk_widget_is_visible(c_widget)) {
        g_object_unref(c_widget);    // Gegensatz zu g_object_ref beim Start
        return G_SOURCE_REMOVE;
    }

    const char *colour = colours[current_colour_index];
    char css[100];
    snprintf(css, sizeof(css),
             ".fullscreen-window { background-color: %s; }", colour);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider, css);
    gtk_style_context_add_provider_for_display(
        gtk_widget_get_display(c_widget),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    current_colour_index = (current_colour_index + 1) % G_N_ELEMENTS(colours);
    return G_SOURCE_CONTINUE;
}



/* ----- Motion-Handler-Wartezeit bis Exit gültig ist --------------- */
static gboolean enable_mouse_exit_after_delay(gpointer user_data)
{
    GtkEventController *motion = GTK_EVENT_CONTROLLER(user_data);
    
    /* Motion-Handler aktivieren */
    gtk_event_controller_set_propagation_phase(motion, GTK_PHASE_TARGET);


    return G_SOURCE_REMOVE;  // Motion-Timer nur einmal ausführen
}

/* ----- Callback ActionRow Buttons --------------------------------- */
static void on_arow_button_clicked(GtkButton *btn, gpointer user_data)
{
    IntervalButtons *ib = user_data;
    if (!ib) return;

    // Reset der CSS-Klassen
    gtk_widget_remove_css_class(ib->btn_1, "suggested-action");
    gtk_widget_remove_css_class(ib->btn_2, "suggested-action");
    gtk_widget_remove_css_class(ib->btn_3, "suggested-action");
    gtk_widget_remove_css_class(ib->btn_4, "suggested-action");

    gtk_widget_add_css_class(GTK_WIDGET(btn), "suggested-action");

    // Intervalle bestimmen
    if      (GTK_WIDGET(btn) == ib->btn_1) ib->interval_ms = 60;
    else if (GTK_WIDGET(btn) == ib->btn_2) ib->interval_ms = 128;
    else if (GTK_WIDGET(btn) == ib->btn_3) ib->interval_ms = 1000;
    else if (GTK_WIDGET(btn) == ib->btn_4) ib->interval_ms = 90000;

    // alten Timer stoppen
    if (colour_timer > 0) {
        g_source_remove(colour_timer);
        colour_timer = 0;

        /* TIMER hatte die Referenz auf das Fullscreen-Window erhöht.
           Hier beim Stoppen diese Referenz wieder freigeben (falls vorhanden). */
        if (ib->fullscreen_window) {
            g_object_unref(ib->fullscreen_window);
        }
    }

    // Einen neuen Timer starten, falls das Fullscreen-Fenster existiert
    if (ib->fullscreen_window && gtk_widget_is_visible(ib->fullscreen_window)) {
         /* Referenz beim Start erhöhen, damit der Timer sich selbst absichert */
         g_object_ref(ib->fullscreen_window);
         colour_timer = g_timeout_add(ib->interval_ms, change_background_colour, ib->fullscreen_window);
    }
}

 
/* ---------- Callback Fullscreen-Button ---------------------------- */
static void on_fullscreen_button_clicked(GtkButton *button, gpointer user_data)
{
    GtkApplication *app = GTK_APPLICATION(user_data);

    /* 1. "ib"-Wert über Buttons holen ---- */
    IntervalButtons *ib = g_object_get_data(G_OBJECT(button), "interval_buttons");
    if (!ib) return;


    /* 2. Fullscreen-Fenster erzeugen ---- */
    GtkWidget *fullscreen_window = gtk_application_window_new(app);
    ib->fullscreen_window = fullscreen_window; // hier speichern!
    gtk_window_set_title(GTK_WINDOW(fullscreen_window), _("Vollbild"));
    gtk_widget_add_css_class(fullscreen_window, "fullscreen-window");
    gtk_window_fullscreen(GTK_WINDOW(fullscreen_window));
    gtk_window_present(GTK_WINDOW(fullscreen_window));

    /* 3. Motion‑Controller erstellen ---- */
    GtkEventController *motion = gtk_event_controller_motion_new();
    g_signal_connect(motion, "motion", G_CALLBACK(exit_fullscreen), fullscreen_window);
    gtk_widget_add_controller(fullscreen_window, motion);
    /* 3.1 Motion-Handler vorerst deaktivieren */
    gtk_event_controller_set_propagation_phase(motion, GTK_PHASE_NONE);

    /* 4. Timer für Farben auf 0 setzen ---- */
    if (colour_timer > 0) {
        g_source_remove(colour_timer);
        colour_timer = 0;
    }

    /* 5. Referenzwert auf das Window erhöhen, damit der Timer eine gültige Referenz hat */
    g_object_ref(fullscreen_window);
    colour_timer = g_timeout_add(ib->interval_ms, change_background_colour, fullscreen_window);

    /* 6. Verzögerung in Sekunden, bir zur Aktivierung des Motion-Handlers --- */
    g_timeout_add_seconds(1, enable_mouse_exit_after_delay, motion);
    // notwendig damit exit_fullscreen() auch auf ib zugreifen kann:
    g_object_set_data(G_OBJECT(fullscreen_window), "interval_buttons", ib);

    /* 7. Standby Prevention starten --- */
    start_standby_prevention();
}

/* ----- Callback Beenden-Button ------------------------------------ */
static void on_quitbutton_clicked (GtkButton *button, gpointer user_data)
{
    IntervalButtons *ib = user_data;
    // Debug:
    g_print("[DEBUG] ib memory address=%p\n", ib);

    //g_print("Received instruction to terminate...\n"); testen
    GtkWindow *win = GTK_WINDOW(user_data); 
    gtk_window_destroy(win);
}

/* ------------------------------------------------------------------ */
/*       Aktivierungshandler                                          */
/* ------------------------------------------------------------------ */
static void on_activate (AdwApplication *app, gpointer)
{
    /* ----- CSS-Provider für zusätzliche Anpassungen --------------- */
    // orange=#db9c4a , lightred=#ff8484 , grey=#c0bfbc
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
                            ".opaque.custom-suggested-action-button1 {"
                                         "  background-color: #c0bfbc;"
                                                      "  color: black;"
                                                                    "}"

                            ".opaque.custom-suggested-action-button2 {"
                                         "  background-color: #434347;"
                                                    "  color: #ff8484;"
                                                                    "}"

                                                                      );

    gtk_style_context_add_provider_for_display( gdk_display_get_default(),
    GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    /* ----- Adwaita-Fenster ---------------------------------------- */
    AdwApplicationWindow *adw_win = ADW_APPLICATION_WINDOW(adw_application_window_new(GTK_APPLICATION(app))); 

    /* ----- .devel-Klasse für Fensterrahmen ----- */
    //gtk_widget_add_css_class (GTK_WIDGET(adw_win), "devel");

    gtk_window_set_title(GTK_WINDOW(adw_win), APP_NAME);         // Fenstertitel
    gtk_window_set_default_size(GTK_WINDOW(adw_win), WIN_WIDTH, WIN_HEIGHT);
    gtk_window_set_resizable(GTK_WINDOW(adw_win), FALSE);       // Skalierung nicht erlauben

    /* --- NavigationView -------------------------------------------- */
    AdwNavigationView *nav_view = ADW_NAVIGATION_VIEW(adw_navigation_view_new());

    /* ----- ToolbarView --------------------------------------------- */
    AdwToolbarView *toolbar_view = ADW_TOOLBAR_VIEW (adw_toolbar_view_new());

    /* ----- HeaderBar mit TitelWidget ------------------------------ */
    AdwHeaderBar *header = ADW_HEADER_BAR (adw_header_bar_new());
    /* Label mit Pango-Markup erzeugen */
    GtkLabel *title_label = GTK_LABEL(gtk_label_new (NULL));
    gtk_label_set_markup(title_label, "<b>Rainbow</b>");               // Fenstertitel in Markup
    gtk_label_set_use_markup(title_label, TRUE);                       // Markup‑Parsing aktivieren
    adw_header_bar_set_title_widget(header, GTK_WIDGET(title_label)); // Label als Title-Widget einsetzen
    adw_toolbar_view_add_top_bar(toolbar_view, GTK_WIDGET(header));   // HeaderBar zur ToolbarView hinzuf

    /* --- Hauptseite in NavigationView ------------------------------ */
    AdwNavigationPage *main_page = adw_navigation_page_new(GTK_WIDGET(toolbar_view), APP_NAME);
    adw_navigation_view_push(nav_view, main_page);                         // NavView als MainPage

    /* ----- ToastOverlay -------------------------------------------- */
    //toast_overlay = ADW_TOAST_OVERLAY(adw_toast_overlay_new());
    //adw_toast_overlay_set_child(toast_overlay, GTK_WIDGET(nav_view));
    toast_manager.toast_overlay = ADW_TOAST_OVERLAY(adw_toast_overlay_new()); // ToastOverlay in Strukt.
    adw_toast_overlay_set_child(toast_manager.toast_overlay, GTK_WIDGET(nav_view));
    /* ----- ToastOverlay in das Fenster einfügen  ------------------- */
    adw_application_window_set_content(adw_win, GTK_WIDGET(toast_manager.toast_overlay)); // ToastOverl. in AdwWin / Strukt.

    /* --- Hamburger-Button innerhalb der Headerbar ----------------- */
    GtkMenuButton *menu_btn = GTK_MENU_BUTTON(gtk_menu_button_new());
    gtk_menu_button_set_icon_name(menu_btn, "open-menu-symbolic");
    adw_header_bar_pack_start(header, GTK_WIDGET (menu_btn)); // Link in Headerbar

    /* --- Popover-Menu im Hamburger -------------------------------- */
    GMenu *menu = g_menu_new();
    g_menu_append(menu, _("Infos zu Rainbow"), "app.show-about");
    GtkPopoverMenu *menu_popover = GTK_POPOVER_MENU(
               gtk_popover_menu_new_from_model(G_MENU_MODEL(menu)));
    gtk_menu_button_set_popover(menu_btn, GTK_WIDGET(menu_popover));

    /* --- Aktion die den About‑Dialog öffnet ----------------------- */
    const GActionEntry entries[] = { { "show-about", show_about, NULL, NULL, NULL } };

    /* Registrierung der im Array about_entry definierten Aktionen */ 
    g_action_map_add_action_entries(G_ACTION_MAP(app), entries, G_N_ELEMENTS(entries), app);

    /* ---- Haupt-Box ----------------------------------------------- */
    GtkBox *main_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 1));
    gtk_widget_set_margin_top   (GTK_WIDGET(main_box),    12);    // Rand unterhalb Headerbar
    gtk_widget_set_margin_bottom(GTK_WIDGET(main_box),    12);    // unterer Rand unteh. der Buttons
    gtk_widget_set_margin_start (GTK_WIDGET(main_box),    12);    // links
    gtk_widget_set_margin_end   (GTK_WIDGET(main_box),    12);    // rechts

    /* ----- Label1 ------------------------------------------------- */
    GtkWidget *label1 = gtk_label_new(_("Der Pixel Refresher\n"));
    gtk_widget_set_halign(label1, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(label1, GTK_ALIGN_CENTER);
    gtk_box_append(main_box, label1);

    /* ----- Icon --------------------------------------------------- */
    GtkWidget *icon = gtk_image_new_from_resource("/free/toq/rainbow/icon1"); //alias in xml !
    gtk_widget_set_halign(icon, GTK_ALIGN_CENTER);                 // Icon horizontal zentrieren
    gtk_image_set_pixel_size(GTK_IMAGE(icon), 174);
    gtk_box_append(main_box, icon);

    /* ----- Hilfe-Button-BOX  -------------------------------------- */
    GtkWidget *hb_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_halign(hb_box, GTK_ALIGN_CENTER);

    /* ----- Hilfe-Button ------------------------------------------- */
    GtkWidget *help_button = gtk_menu_button_new();
    gtk_menu_button_set_icon_name(GTK_MENU_BUTTON(help_button), "help-about-symbolic");
    gtk_widget_add_css_class(help_button, "circular");
    gtk_widget_add_css_class(help_button, "flat");
    gtk_box_append(GTK_BOX(hb_box), help_button);

    /* ---- Hilfe-Button-Popover ------------------------------------ */
    GtkWidget *help_popover = gtk_popover_new();
    /* Scroll-Container und Content-BOX erstellen */
    GtkWidget *scrolled = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled), 280);
    GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_size_request(content_box, 300, -1); // Breite
    /* Popover erstellen und einsetzen */
    GtkWidget *content_title = gtk_label_new(_("Farbintervall"));
    gtk_widget_add_css_class(content_title, "title-4");
    gtk_label_set_xalign(GTK_LABEL(content_title), 0.0);
    gtk_box_append(GTK_BOX(content_box), content_title);
    /* Hilfe-Texte */
    GtkWidget *help_text = gtk_label_new(
      _("• <b>60ms (≈16Hz)</b>\n"
        "Schnelle Pixel‑Revitalisierung: \n"
        "Kann festsitzende Pixel entladen, bei nicht zu hoher Display-Belastung. \n\n"
        "• <b>128ms (≈7,8Hz)</b>\n"
        "Hartnäckige Revitalisierung: \n"
        "Gibt Kristallen mehr Zeit zur vollständige Entladung, "
        "kann bei festsitzenden Pixeln wirksamer sein. \n\n"
        "• <b>1sec (1Hz)</b>\n"
        "Burn-In Prävention und leichte Ghosting-Reduktion:\n"
        "Fast keine Belastung für das Display, geeignet für Sichtung und Diagnose. \n\n"
        "• <b>1min (60s)</b>\n"
        "Maximale Flüssigkristall-Entspannung, gleichmäßige Belastung: \n"
        "Ideal für Diagnose sowie Mess- und Kalibrierprozesse.\n\n"
        "(Alle Angaben sind ohne Gewähr!)")
        );
    gtk_label_set_use_markup(GTK_LABEL(help_text), TRUE); // Markup anwenden
    gtk_label_set_wrap(GTK_LABEL(help_text), TRUE);
    gtk_label_set_wrap_mode(GTK_LABEL(help_text), PANGO_WRAP_WORD_CHAR);
    gtk_label_set_xalign(GTK_LABEL(help_text), 0.0);
    gtk_label_set_max_width_chars(GTK_LABEL(help_text), 50);
    gtk_widget_set_hexpand(help_text, TRUE); // Breite
    gtk_box_append(GTK_BOX(content_box), help_text);
    /* Content-BOX Abstände */
    gtk_widget_set_margin_top   (content_box, 12);
    gtk_widget_set_margin_bottom(content_box, 12);
    gtk_widget_set_margin_start (content_box, 12);
    gtk_widget_set_margin_end   (content_box, 12);
    /* Zusammensetzen */
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), content_box);
    gtk_popover_set_child(GTK_POPOVER(help_popover), scrolled);
    /* Popover an Button binden */
    gtk_menu_button_set_popover(GTK_MENU_BUTTON(help_button), help_popover);

    /* ----- Label2-BOX  -------------------------------------------- */
    GtkWidget *l2_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_halign(l2_box, GTK_ALIGN_CENTER);

    /* ----- Label2 ------------------------------------------------- */
    GtkWidget *label2 = gtk_label_new(_("Farbintervall auswählen:  "));
    gtk_box_append(GTK_BOX(l2_box), label2);
    gtk_box_append(GTK_BOX(main_box), l2_box);

    /* ----- ListBox für die ActionRows ----------------------------- */
    GtkListBox *actionrow_listbox = GTK_LIST_BOX(gtk_list_box_new()); // notwendig für action_row's
    gtk_list_box_set_selection_mode(actionrow_listbox, GTK_SELECTION_NONE);
    gtk_widget_set_margin_top(GTK_WIDGET(actionrow_listbox), 6); // Listbox 6px unterhalb von l2_box beginnen

    /* ----- ActionRow-4-Buttons ------------------------------------ */
    AdwActionRow *action_row1 = ADW_ACTION_ROW(adw_action_row_new());
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(action_row1), "");

    GtkWidget *btn_1  = gtk_button_new_with_label("60ms");
    GtkWidget *btn_2  = gtk_button_new_with_label("128ms");
    GtkWidget *btn_3  = gtk_button_new_with_label("1sec");
    GtkWidget *btn_4  = gtk_button_new_with_label("1min");

    adw_action_row_add_suffix(action_row1, btn_1);
    adw_action_row_add_suffix(action_row1, btn_2);
    adw_action_row_add_suffix(action_row1, btn_3);
    adw_action_row_add_suffix(action_row1, btn_4);

    IntervalButtons *ib = g_new0(IntervalButtons, 1);
         ib->btn_1 = btn_1;
         ib->btn_2 = btn_2;
         ib->btn_3 = btn_3;
         ib->btn_4 = btn_4;
         ib->interval_ms = 1000;     // Vorgabewert A
    g_signal_connect(btn_1, "clicked", G_CALLBACK(on_arow_button_clicked), ib);
    g_signal_connect(btn_2, "clicked", G_CALLBACK(on_arow_button_clicked), ib);
    g_signal_connect(btn_3, "clicked", G_CALLBACK(on_arow_button_clicked), ib);
    g_signal_connect(btn_4, "clicked", G_CALLBACK(on_arow_button_clicked), ib);

    gtk_widget_add_css_class(btn_3, "suggested-action"); // Vorgabewert B
    gtk_widget_set_halign (GTK_WIDGET(action_row1), GTK_ALIGN_CENTER);

    gtk_list_box_append(actionrow_listbox, GTK_WIDGET(action_row1));
    gtk_box_append(GTK_BOX(main_box), GTK_WIDGET(actionrow_listbox));

    /* ----- Hilfe-Button-BOX + Label2-BOX einfügen ----------------- */
    gtk_box_append(GTK_BOX(main_box), hb_box);

    /* ----- Shutdown-Handler mit den ib verbinden ------------------ */
    g_signal_connect(app, "shutdown", G_CALLBACK(stop_standby_prevention), ib); // testen !! 30.12.25

    /* ----- untere Button-BOX -------------------------------------- */
    GtkBox *button_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12));
    gtk_widget_set_valign (GTK_WIDGET(button_box), GTK_ALIGN_END);
    gtk_widget_set_hexpand(GTK_WIDGET(button_box), TRUE);
    gtk_widget_set_vexpand(GTK_WIDGET(button_box), TRUE);
    gtk_widget_set_halign (GTK_WIDGET(button_box), GTK_ALIGN_CENTER);
    gtk_widget_set_margin_bottom(GTK_WIDGET(button_box), 12);

    /* ----- Fullscreen-Button ------------------------------------- */
    GtkWidget *setfullscreen_button = gtk_button_new_with_label(_(" Starten "));
    gtk_widget_set_halign(setfullscreen_button, GTK_ALIGN_CENTER);
    gtk_widget_set_size_request(setfullscreen_button, 140, 48); // Button Größe
    g_signal_connect(setfullscreen_button, "clicked", G_CALLBACK(on_fullscreen_button_clicked), app);
//!!  g_object_set_data(G_OBJECT(setfullscreen_button), "set1_check", set1_check);
    g_object_set_data(G_OBJECT(setfullscreen_button), "interval_buttons", ib);

    /* ----- Beenden-Button ----------------------------------------- */
    GtkWidget *quit_button = gtk_button_new_with_label(_(" Beenden "));
    gtk_widget_set_halign(quit_button, GTK_ALIGN_CENTER);
    gtk_widget_set_size_request(quit_button, 140, 48); // Button Größe
    g_signal_connect(quit_button, "clicked", G_CALLBACK(on_quitbutton_clicked), adw_win);

    gtk_box_set_spacing(GTK_BOX(button_box), 12); // Abstand zwischen den Schaltflächen
    gtk_box_append(button_box, quit_button);
    gtk_box_append(button_box, setfullscreen_button);
    gtk_box_append(main_box, GTK_WIDGET(button_box));
    /* -----  Haupt-Box zur ToolbarView hinzufügen ------------------ */
    adw_toolbar_view_set_content(toolbar_view, GTK_WIDGET(main_box));

    /* ----- Haupt-Fenster desktop‑konform anzeigen ----------------- */
    gtk_window_present(GTK_WINDOW(adw_win));
}

/* --------------------------------------------------------------------------- */
/* Anwendungshauptteil, main()                                                 */
/* --------------------------------------------------------------------------- */
int main (int argc, char **argv)
{
    /* Arbeitsverzeichnis ermitteln */
    const char *app_dir = g_get_current_dir();  
    const char *locale_path = NULL;
    const char *flatpak_id = getenv("FLATPAK_ID"); //flatpak string 

    /* Resource-Bundle registrieren um den Inhalt verfügbar zu machen */
    g_resources_register (resources_get_resource ()); 


    /* ----- Localiziation-Pfad ----- */
    setlocale(LC_ALL, "");               // ruft die aktuelle Locale des Prozesses ab
//    setlocale(LC_ALL, "en_US.UTF-8"); // testen!!
    textdomain             (APP_DOMAINNAME);      // textdomain festlegen
    bind_textdomain_codeset(APP_DOMAINNAME, "UTF-8"); 
    if (flatpak_id != NULL && flatpak_id[0] != '\0')
    {
        locale_path = "/app/share/locale";
    } else {
        locale_path = "/usr/share/locale";
    }
    bindtextdomain         (APP_DOMAINNAME, locale_path);
    //g_print("Localization files in: %s \n", locale_path); // testen!!


    g_autoptr(AdwApplication) app =      // Instanz erstellen + App-ID + Default-Flags;
                        adw_application_new(APP_ID, G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "activate", G_CALLBACK(on_activate),             NULL);
    /* --- Anwendung starten u. auf Ereignis wartet --- */
    return g_application_run(G_APPLICATION (app), argc, argv);
}
