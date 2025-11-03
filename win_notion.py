import os
import json
import subprocess
import webbrowser
import time
import psutil
import paho.mqtt.client as mqtt

BROKER = "192.168.3.8"
PORT = 1883
TOPIC = "router/alive"
USER = "pi_mqtt"
PASS = "00000011"

NOTION_URI = "notion://www.notion.so"
NOTION_EXE = os.path.expandvars(r"%LOCALAPPDATA%\Programs\Notion\Notion.exe")
NOTION_WEB = "https://www.notion.so"

notion_opened = False

def open_notion():
    global notion_opened

    # 이미 실행 중인지 확인
    for proc in psutil.process_iter(['name']):
        try:
            if "Notion.exe" in proc.info['name']:
                print("[INFO] Notion already running. Skipping launch.")
                notion_opened = True
                return
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            continue

    print("[INFO] Trying to open Notion...")

    try:
        os.startfile(NOTION_URI)
        print("[INFO] Notion URI opened successfully.")
        notion_opened = True
        return
    except Exception as e:
        print(f"[WARN] URI launch failed: {e}")

    if os.path.exists(NOTION_EXE):
        try:
            subprocess.Popen([NOTION_EXE], shell=True)
            print("[INFO] Notion.exe started.")
            notion_opened = True
            return
        except Exception as e:
            print(f"[WARN] Notion.exe launch failed: {e}")
    else:
        print(f"[WARN] Notion.exe not found: {NOTION_EXE}")

    try:
        webbrowser.open(NOTION_WEB)
        print("[INFO] Opened Notion in web browser.")
        notion_opened = True
    except Exception as e:
        print(f"[ERROR] All launch methods failed: {e}")

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("[MQTT] Connected to broker.")
        client.subscribe(TOPIC)
    else:
        print(f"[MQTT] Connection failed with code {rc}")

def on_message(client, userdata, msg):
    global notion_opened

    if notion_opened:
        return

    try:
        payload = msg.payload.decode("utf-8")
        data = json.loads(payload)
        mac = data.get("mac")
        alive = data.get("alive")

        print(f"[MQTT] Received: MAC={mac}, alive={alive}")

        if alive is True:
            print("[ACTION] Alive=True detected. Launching Notion.")
            open_notion()
        else:
            print("[INFO] Alive=False. No action taken.")

    except json.JSONDecodeError:
        print("[WARN] Invalid JSON received.")
    except Exception as e:
        print(f"[ERROR] Exception while processing message: {e}")

def main():
    client = mqtt.Client()
    client.username_pw_set(USER, PASS)
    client.on_connect = on_connect
    client.on_message = on_message

    try:
        client.connect(BROKER, PORT, 60)
    except Exception as e:
        print(f"[ERROR] MQTT connection failed: {e}")
        return

    print("[INFO] MQTT subscription started. Press Ctrl+C to exit.")

    try:
        client.loop_forever()
    except KeyboardInterrupt:
        print("[INFO] Stopping MQTT listener...")
    finally:
        client.disconnect()

if __name__ == "__main__":
    main()
