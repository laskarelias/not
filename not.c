#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <sys/inotify.h>
  
#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

void battery() {
    int capacity;
    FILE *fp;
    
    fp = fopen("/sys/class/power_supply/BAT0/capacity", "r");
    fscanf(fp, "%d", &capacity);
    printf("%d\n", capacity);
    fclose(fp);
}


int main() {
    battery();
    static const char *battery_ev = "/sys/class/power_supply/BAT0/uevent";

    char buf[BUF_LEN] __attribute__ ((aligned(8)));

    int fd, wd;
    ssize_t refresh = 1;

    fd = inotify_init();
    if (fd == -1) { printf("inotify init\n"); }

    wd = inotify_add_watch(fd, battery_ev, IN_ACCESS);
    if (wd == -1) { printf("inotify add watch\n"); }

    battery();

    while(refresh) {
        refresh = read(fd, buf, BUF_LEN);
        battery();
    }   

    return 0;
}
