#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <sys/inotify.h>
#include <X11/Xlib.h>
  
#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

static Display *dpy;
static int screen;
static Window root;

char status[512];

int battery_perc() {
    int perc;
    FILE *fp;
    
    fp = fopen("/sys/class/power_supply/BAT0/capacity", "r");
    fscanf(fp, "%d", &perc);
    fclose(fp);
    return perc;
}

char battery_status() {
    char status[12];
    FILE *fp;

    fp = fopen("/sys/class/power_supply/BAT0/status", "r");
    fscanf(fp, "%s", status);
    fclose(fp);
    if (strcmp(status, "Discharging") == 0) { return '-'; }
    if (strcmp(status, "Charging") == 0)    { return '+'; }
    if (strcmp(status, "Full") == 0)        { return '='; }
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
    int bat_perc;
    char bat_status;

    char buf[BUF_LEN] __attribute__ ((aligned(8)));

    int fd, wd;
    ssize_t refresh = 1;
    int debug_count = 0;

    fd = inotify_init();
    if (fd == -1) { printf("inotify: couldn't init\n"); }

    wd = inotify_add_watch(fd, battery_ev, IN_ALL_EVENTS);
    if (wd == -1) { printf("inotify: couldn't add watch\n"); }

    bat_perc = battery_perc();

    while(refresh) {
        refresh = read(fd, buf, BUF_LEN);
        bat_perc = battery_perc();
        sprintf(status, "Battery: %d%%", bat_perc);
        printf("[%d]:: %s\n", debug_count, status);
        debug_count++;
        set_status();
    }   
    
    return 0;
}
