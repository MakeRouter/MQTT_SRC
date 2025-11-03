#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <mosquitto.h>

#define LEASE_FILE "/var/lib/misc/dnsmasq.leases"
#define TARGET_MAC "****"   // 확인할 MAC 주소
#define IFACE "wlan0"                    // arping 인터페이스
#define MQTT_BROKER "192.168.50.1"       // 브로커 IP (라즈베리파이)
#define MQTT_PORT 1883
#define TOPIC "router/alive"
#define MQTT_USER "pi_mqtt"
#define MQTT_PASS "00000011"
#define CHECK_INTERVAL 5   // 주기 (초 단위)

volatile int keep_running = 1;

// Ctrl+C 핸들러
void handle_sigint(int sig) {
    (void)sig;
    keep_running = 0;
    printf("\n[INFO] Ctrl+C detected, stopping...\n");
}

int main(void) {
    signal(SIGINT, handle_sigint);

    // MQTT 초기화
    mosquitto_lib_init();
    struct mosquitto *mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        fprintf(stderr, "MQTT client create failed\n");
        return 1;
    }

    if (mosquitto_username_pw_set(mosq, MQTT_USER, MQTT_PASS) != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "MQTT username/password set failed\n");
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 1;
    }

    if (mosquitto_connect(mosq, MQTT_BROKER, MQTT_PORT, 60) != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "MQTT connect fail\n");
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 1;
    }

    printf("[INFO] Monitoring started... (press Ctrl+C to stop)\n");

    while (keep_running) {
        FILE *fp = fopen(LEASE_FILE, "r");
        if (!fp) {
            perror("fopen");
            sleep(CHECK_INTERVAL);
            continue;
        }

        char line[512];
        int found = 0;

        while (fgets(line, sizeof(line), fp)) {
            char ts[32], mac[32], ip[32], name[64], cid[64];
            int n = sscanf(line, "%31s %31s %31s %63s %63s", ts, mac, ip, name, cid);

            // 필드 4개 이상이면 정상적인 데이터
            if (n >= 4 && strcasecmp(mac, TARGET_MAC) == 0) {
                found = 1;
                printf("[INFO] Found MAC %s -> IP %s (%s)\n", mac, ip, name);

                char cmd[128];
                snprintf(cmd, sizeof(cmd),
                         "sudo arping -I %s -c 1 -w 1 %s > /dev/null 2>&1", IFACE, ip);
                int ret = system(cmd);

                char payload[256];
                if (ret == 0) {
                    printf("[OK] %s (%s) is ALIVE\n", mac, ip);
                    snprintf(payload, sizeof(payload),
                             "{\"mac\":\"%s\",\"ip\":\"%s\",\"alive\":true}", mac, ip);
                } else {
                    printf("[NO] %s (%s) is NOT responding\n", mac, ip);
                    snprintf(payload, sizeof(payload),
                             "{\"mac\":\"%s\",\"ip\":\"%s\",\"alive\":false}", mac, ip);
                }

                int rc = mosquitto_publish(mosq, NULL, TOPIC,
                                           strlen(payload), payload, 0, false);
                if (rc == MOSQ_ERR_SUCCESS)
                    printf("[MQTT] Published -> %s : %s\n", TOPIC, payload);
                else
                    fprintf(stderr, "[MQTT] Publish failed: %s\n", mosquitto_strerror(rc));

                break;
            }
        }

        fclose(fp);

        if (!found)
            printf("[WARN] MAC %s not found in leases file.\n", TARGET_MAC);

        sleep(CHECK_INTERVAL);
    }

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    printf("[INFO] MQTT monitoring stopped.\n");
    return 0;
}
