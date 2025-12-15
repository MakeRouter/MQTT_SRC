# MQTT_SRC
One step closer to a smart router

---

## install mosquitto

```
opkg update
opkg install mosquitto-ssl mosquitto-client-ssl
```
- mosquitto-ssl: MQTT 브로커 본체 (보안 기능 포함)
- mosquitto-client-ssl: 테스트할 때 쓸 mosquitto_pub, mosquitto_sub 명령어 도구

---

## Setting mosquitto

```
vi /etc/mosquitto/mosquitto.conf

# 포트 1883을 모든 IP에 대해 열기
listener 1883

# 익명 접속 차단
allow_anonymous false

# 비밀번호 파일 위치 지정
password_file /etc/mosquitto/passwd

# 비밀번호 권한 부여
chmod 644 /etc/mosquitto/passwd

```

### Setting password

```
mosquitto_passwd -c /etc/mosquitto/passwd user
```
- 입력 후: Password: 라고 뜨면 비밀번호를 입력하고 엔터, Reenter password: 가 뜨면 한 번 더 입력하세요.
- 참고: -c 옵션은 파일을 **새로 생성(Create)**한다는 뜻입니다. (기존 파일을 덮어쓰므로 주의!)
- 만약 두 번째 사용자를 추가하고 싶다면 -c를 빼고 mosquitto_passwd /etc/mosquitto/passwd user2 처럼 입력하면 됩니다.

---

## Open Firewall

```
vi /etc/config/firewall

config rule
        option name             Allow-MQTT
        option src              wan
        option proto            tcp
        option dest_port        1883
        option target           ACCEPT

```

### restart firewall

```
/etc/init.d/firewall restart
```

---

## MQTT enable & start

```
# 부팅 시 자동 실행 등록
/etc/init.d/mosquitto enable

# 지금 바로 실행
/etc/init.d/mosquitto start
```


