#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <X11/Xlib.h>
#include "config.h"

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

/* X Server */
static Display *dpy;
static int screen;
static Window root;

/* Status */
char status[512];

int bat_perc;
char bat_status;

void battery() {
    int perc;
    FILE *fp;
    char status[12];
    char *path = malloc(strlen(BATTERY_NAME) + 35);

    sprintf(path, "/sys/class/power_supply/%s/capacity", BATTERY_NAME);
    fp = fopen(path, "r");
    fscanf(fp, "%d", &perc);
    fclose(fp);

    bat_perc = perc;

    sprintf(path, "/sys/class/power_supply/%s/status", BATTERY_NAME);
    fp = fopen(path, "r");
    fscanf(fp, "%s", status);
    fclose(fp);

    if (strcmp(status, "Discharging") == 0) { bat_status = DISCHARGING_SYMBOL; }
    if (strcmp(status, "Charging") == 0)    { bat_status = CHARGING_SYMBOL; }
    if (strcmp(status, "Full") == 0)        { bat_status = FULL_SYMBOL; }
}

void set_status() {
    Display *d = XOpenDisplay(NULL);
    if (d) { dpy = d; }

    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);
    XStoreName(dpy, root, status);
    XCloseDisplay(dpy);
}

int main() {
    static const char *battery_ev = "/sys/class/power_supply/BAT0/uevent";
    static const char *network_ev = "/proc/net/wireless";


    int fd, wd;
    ssize_t refresh_ev = 1;
    char buf[BUF_LEN];

    int debug_count = 0;

    fd = inotify_init();
    if (fd == -1) { printf("inotify: couldn't init\n"); }

    wd = inotify_add_watch(fd, battery_ev, IN_ALL_EVENTS);
    if (wd == -1) { printf("inotify: couldn't add watch\n"); }

    do {
        battery();
        sprintf(status, "Battery: %d%c%%", bat_perc, bat_status);
        printf("[%d]:: %s\n", debug_count, status);
        debug_count++;
        set_status();
        refresh_ev = read(fd, buf, BUF_LEN);
    } while(refresh_ev);

    return 0;
}
