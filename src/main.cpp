#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>


/* ================= USER CONFIG ================= */

const char* WIFI_SSID = "YOUR_WIFI";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

const char* API_KEY   = "SUPER_SECRET_PASSWORD";

const uint8_t RELAY_PIN = D1;

const uint32_t MIN_PRESS_MS = 100;
const uint32_t MAX_PRESS_MS = 15000;

/* Auto ESP reboot */
const uint64_t ESP_REBOOT_INTERVAL_MS =
    12ULL * 60ULL * 60ULL * 1000ULL;

/* ---------- Email ---------- */

const char* SMTP_SERVER = "smtp.gmail.com";
const uint16_t SMTP_PORT = 465;

const char* EMAIL_USER = "your@gmail.com";
const char* EMAIL_PASS = "APP_PASSWORD";

const char* EMAIL_FROM = "your@gmail.com";
const char* EMAIL_TO   = "alerts@yourdomain.com";

/* =============================================== */

ESP8266WebServer server(80);
WiFiClientSecure smtp;


/* ---------- State ---------- */

uint64_t bootTimeMs;
uint32_t lastWifiCheck;

/* ---------- Relay ---------- */

void triggerRelay(uint32_t pressMs) {
    pressMs = constrain(pressMs, MIN_PRESS_MS, MAX_PRESS_MS);

    digitalWrite(RELAY_PIN, HIGH);
    delay(pressMs);
    digitalWrite(RELAY_PIN, LOW);
}

/* ---------- Email ---------- */

bool sendEmail(uint32_t pressMs) {
    smtp.setInsecure(); // OK for embedded use

    if (!smtp.connect(SMTP_SERVER, SMTP_PORT)) {
        Serial.println("SMTP connect failed");
        return false;
    }

    auto sendCmd = [&](const String& cmd, int wait = 500) {
        smtp.println(cmd);
        delay(wait);
    };

    sendCmd("EHLO esp8266");
    sendCmd("AUTH LOGIN");
    sendCmd(base64::encode(EMAIL_USER));
    sendCmd(base64::encode(EMAIL_PASS));

    sendCmd("MAIL FROM:<" + String(EMAIL_FROM) + ">");
    sendCmd("RCPT TO:<" + String(EMAIL_TO) + ">");
    sendCmd("DATA");

    smtp.println("Subject: SERVER REBOOT TRIGGERED");
    smtp.println("From: ESP8266 <" + String(EMAIL_FROM) + ">");
    smtp.println("To: <" + String(EMAIL_TO) + ">");
    smtp.println();
    smtp.println("Relay triggered");
    smtp.println("Press duration: " + String(pressMs) + " ms");
    smtp.println("ESP IP: " + WiFi.localIP().toString());
    smtp.println(".");
    delay(500);

    sendCmd("QUIT");
    smtp.stop();

    Serial.println("Email alert sent");
    return true;
}

/* ---------- Auth ---------- */

bool authorized() {
    return server.hasArg("key") && server.arg("key") == API_KEY;
}

/* ---------- HTTP handler ---------- */

void handleReboot() {
    if (!authorized()) {
        server.send(403, "text/plain", "Forbidden");
        return;
    }

    if (!server.hasArg("ms")) {
        server.send(400, "text/plain", "Missing ms parameter");
        return;
    }

    uint32_t pressMs = server.arg("ms").toInt();
    if (pressMs == 0) {
        server.send(400, "text/plain", "Invalid ms");
        return;
    }

    server.send(200, "text/plain", "Relay trigger accepted");
    triggerRelay(pressMs);
    sendEmail(pressMs);

}

/* ---------- Auto reboot ---------- */

void checkAutoReboot() {
    if (millis() - bootTimeMs >= ESP_REBOOT_INTERVAL_MS) {
        ESP.restart();
    }
}

/* ---------- WiFi ---------- */

void checkWiFi() {
    if (millis() - lastWifiCheck < 10000) return;
    lastWifiCheck = millis();

    if (WiFi.status() != WL_CONNECTED) {
        WiFi.disconnect();
        WiFi.begin(WIFI_SSID, WIFI_PASS);
    }
}

/* ---------- Setup ---------- */

void setup() {
    Serial.begin(115200);

    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    ESP.wdtEnable(8000);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        ESP.wdtFeed();
    }

    server.on("/", []() {
        if (!authorized()) {
            server.send(403, "text/plain", "Forbidden");
        } else server.send(200, "text/plain", "ESP8266 Relay Rebooter Online");
    });

    server.on("/reboot", handleReboot);
    server.begin();

    bootTimeMs = millis();
    lastWifiCheck = millis();
}

/* ---------- Loop ---------- */

void loop() {
    server.handleClient();
    checkWiFi();
    checkAutoReboot();
    ESP.wdtFeed();
}
