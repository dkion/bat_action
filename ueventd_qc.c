#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define UEVENT_BUFFER_SIZE 2048
#define POWER_SUPPLY "power_supply"

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_nl addr;
    char buf[UEVENT_BUFFER_SIZE];
    int sync_bms = 0, sync_battery = 0;

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = 1; // KOBJECT_UEVENT_GROUP

    sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sock);
        return 1;
    }

    while (1) {
        ssize_t len = recv(sock, buf, sizeof(buf), 0);
        if (len < 0) continue;

        sync_bms = 0;
        sync_battery = 0;

        char *ptr = buf;
        while (ptr < buf + len) {
            if (strstr(ptr, POWER_SUPPLY)) {
                if (strstr(ptr, "bms")) sync_bms = 1;
                if (strstr(ptr, "battery")) sync_battery = 1;
            }
            ptr += strlen(ptr) + 1;
        }

        if (sync_bms || sync_battery) {
            if (sync_bms)
                system("mount --bind /data/adb/modules/battery_fake/fake/bms "
                       "/sys/class/power_supply/bms/uevent");
            if (sync_battery)
                system("mount --bind /data/adb/modules/battery_fake/fake/battery "
                       "/sys/class/power_supply/battery/uevent");
        }
    }

    close(sock);
    return 0;
}
