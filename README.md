# ESP8266-ServerReboot

A small ESP8266 (PlatformIO / Arduino) project that allows you to **remotely reboot a server** via a relay using an **HTTP request**.

This project is designed for **headless / remote infrastructure recovery**, where physical access is limited.

---

## Features

- 🌐 HTTP API (single endpoint)
- 🔐 Password-protected trigger
- ⏱ Relay press duration via URL (`ms=` parameter)
- 📧 Email alert when triggered
- 🔁 ESP8266 auto-reboot every 12 hours
- 🐶 Hardware watchdog enabled
- 📡 WiFi auto-reconnect
- 🧱 Safety limits on relay press time

---

## Hardware Required

- ESP8266 (NodeMCU, Wemos D1 Mini, etc.)
- Relay module (5V or 3.3V logic compatible)
- Server motherboard reset or power pins
- Stable power supply for ESP8266

---

## Wiring Diagram

### ESP8266 → Relay Module

```
ESP8266           Relay Module
-------------------------------
D1 (GPIO5)  --->  IN
GND          --->  GND
5V / VIN     --->  VCC
```

### Relay → Server Motherboard

```
Server Reset / Power Header
---------------------------
[ RESET+ ] --- NO
[ RESET- ] --- COM
```

> The relay acts like a physical button press.
>  
> Use **NO (Normally Open)** and **COM** terminals.

---

## API Usage

### Trigger relay

```
/reboot?key=PASSWORD&ms=500
```

Examples:

Soft reboot (short press):
```
http://DEVICE_IP/reboot?key=PASSWORD&ms=500
```

Hard reboot (long press):
```
http://DEVICE_IP/reboot?key=PASSWORD&ms=5000
```
Static ip (DHCP Server entry) recommended
---

## Configuration

Edit these values in `src/main.cpp`:

```cpp
const char* WIFI_SSID = "YOUR_WIFI";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

const char* API_KEY   = "SUPER_SECRET_PASSWORD";
```

---

### - Email Alerts (SMTP)

```cpp
const char* SMTP_SERVER = "smtp.gmail.com";
const uint16_t SMTP_PORT = 465;

const char* EMAIL_USER = "your@gmail.com";
const char* EMAIL_PASS = "APP_PASSWORD";

const char* EMAIL_FROM = "your@gmail.com";
const char* EMAIL_TO   = "alerts@yourdomain.com";
```

⚠️ If using Gmail, enable **2FA** and create an **App Password**.

---

## PlatformIO

### platformio.ini

```ini
[env:esp8266]
platform = espressif8266
board = nodemcuv2
framework = arduino
monitor_speed = 115200
```

Build & upload:

```bash
pio run -t upload
pio device monitor
```

---

## Security Notes (IMPORTANT)

This firmware **must NOT be directly exposed to the public internet**.

### Recommended protections:
- Use VPN (eg. WireGuard)
- Strong random password (32+ characters)
- Non-standard port
- Firewall IP allowlist
- Reverse proxy with authentication
- Rate limiting

This project **does not implement HTTPS** by default.

---

## ⚠️ Use At Your Own Risk

This project controls **physical hardware** and can:
- Power-cycle servers
- Interrupt running systems
- Cause data loss if misused

You accept **all responsibility** for:
- Hardware damage
- Data loss
- Service outages
- Security exposure

Test thoroughly before production use.

---

## License

MIT License

---

## Disclaimer

This software is provided **AS IS**, without warranty of any kind.
