#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h> // access()

#define DEEP_NIGHT_TEMP 1500
#define EVENING_TEMP 2500
#define DAY_TEMP 4000

pid_t hyprsunset_pid = 0;

void send_notification(const char *message, const char *icon) {
        char command[256];
        snprintf(command, sizeof(command), "dunstify -a 'System' -i '%s' -r 2593 -u low '%s' -t 3500", icon, message);
        system(command);
}

void on_button_on_clicked(GtkWidget *widget, gpointer data) {
        int current_hour = localtime(&(time_t){time(NULL)})->tm_hour;
        int sunset_value;

        if (current_hour >= 0 && current_hour < 8) {
                sunset_value = DEEP_NIGHT_TEMP;
        } else if (current_hour >= 18 && current_hour < 24) {
                sunset_value = EVENING_TEMP;
        } else {
                sunset_value = DAY_TEMP;
        }

        pid_t pid = fork();
        if (pid == 0) { // Child process
                send_notification("Night Mode On", "/usr/share/icons/ePapirus-Dark/32x32/categories/tomato.svg");
                char command[256];
                snprintf(command, sizeof(command), "hyprsunset -t %d", sunset_value);
                execl("/bin/sh", "sh", "-c", command, NULL);
                perror("execl");
                exit(1); // Exit child process on error
        } else if (pid < 0) { // Fork failed
                perror("fork");
                send_notification("Error starting hyprsunset", "/usr/share/icons/ePapirus-Dark/32x32/categories/error.svg");
        } 
        gtk_main_quit(); // Moved here. Now it won't block.
}

int is_hyprsunset_running() {
        FILE *fp;
        char path[1024];

        fp = popen("pidof hyprsunset", "r");
        if (fp == NULL) {
                perror("popen");
                return 0; // Assume not running on error
        }

        if (fgets(path, sizeof(path), fp) != NULL) {
                pclose(fp);
                return 1; // hyprsunset is running
        }

        pclose(fp);
        return 0; // hyprsunset is not running
}

void on_button_off_clicked(GtkWidget *widget, gpointer data) {
        if (is_hyprsunset_running()) {
                int ret = system("killall hyprsunset"); // Kill Hyprsunset
                if (ret == 0) {
                        send_notification("Night Mode Off", "/usr/share/icons/ePapirus-Dark/32x32/categories/Temps.svg");
                } else {
                        send_notification("Error turning off Night Mode!", "/usr/share/icons/ePapirus-Dark/32x32/categories/system-error.svg");
                }
        } else {
                send_notification("Hyprsunset not running!", "/usr/share/icons/ePapirus-Dark/32x32/categories/system-error.svg");
        }
        gtk_main_quit();
}

// Function to set rounded corners for a button
void set_rounded_corners(GtkWidget *button, int radius) {
        GtkStyleContext *context = gtk_widget_get_style_context(button);
        gtk_style_context_add_class(context, "rounded-button");

        char css[256];
        snprintf(css, sizeof(css), ".rounded-button { border-radius: %dpx; padding: 10px; }", radius); // Added padding
        GtkCssProvider *provider = gtk_css_provider_new();
        gtk_css_provider_load_from_data(provider, css, -1, NULL);
        gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(provider);
}


GtkWidget* create_night_mode_window() {
        GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(window), "Night Mode");
        gtk_window_set_default_size(GTK_WINDOW(window), 150, 100); // Adjusted size
        gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
        gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

        gtk_window_set_icon_from_file(GTK_WINDOW(window), "/usr/share/icons/ePapirus-Dark/32x32/apps/nightsky.svg", NULL);



        GtkWidget *grid = gtk_grid_new();
        gtk_grid_set_row_spacing(GTK_GRID(grid), 5); // Row spacing
        gtk_grid_set_column_spacing(GTK_GRID(grid), 5); // Column spacing
        gtk_container_set_border_width(GTK_CONTAINER(window), 20);
        gtk_container_add(GTK_CONTAINER(window), grid);


        GtkWidget *button_on = gtk_button_new_with_label("Turn On");
        set_rounded_corners(button_on, 15);
        g_signal_connect(button_on, "clicked", G_CALLBACK(on_button_on_clicked), NULL);
        gtk_grid_attach(GTK_GRID(grid), button_on, 0, 0, 1, 1);

        GtkWidget *button_off = gtk_button_new_with_label("Turn Off");
        set_rounded_corners(button_off, 15);
        g_signal_connect(button_off, "clicked", G_CALLBACK(on_button_off_clicked), NULL);
        gtk_grid_attach(GTK_GRID(grid), button_off, 1, 0, 1, 1);

        GtkWidget *button_quit = gtk_button_new_with_label("Quit");
        set_rounded_corners(button_quit, 15);
        g_signal_connect_swapped(button_quit, "clicked", G_CALLBACK(gtk_widget_destroy), window);
        gtk_grid_attach(GTK_GRID(grid), button_quit, 0, 1, 2, 1);

        gtk_widget_show_all(window);
        return window;

}

int main(int argc, char *argv[]) {
        gtk_init(&argc, &argv);

        GtkWidget *window = create_night_mode_window();

        gtk_main();

        return 0;
}
