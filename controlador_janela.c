#include <WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>
#include <SPIFFS.h>

#define IO_USERNAME "Kaioalves"
#define IO_KEY "aio_umNm76tAZFwqNkw0jdXLu6WLB6pa"
const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqttServer = "io.adafruit.com";
const int mqttPort = 1883;
const char* mqttUser = IO_USERNAME;
const char* mqttPassword = IO_KEY;

#define DHTPIN 15
#define LED_AZUL 21
#define LED_ROSA 19
#define POT_PIN 35

DHTesp dht;
WiFiClient espClient;
PubSubClient client(espClient);

float temperatura = 0.0;
int posicao = 0;
bool exibiuJanelaFechada = false;
bool exibiuJanelaAberta = false;
bool ledAzulLigado = false;
bool ledRosaLigado = false;

void setup_wifi() {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
        }
    Serial.println("\nWiFi connected. IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
    while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32-WindowControl";
    if (client.connect(clientId.c_str(), mqttUser, mqttPassword)) {
        Serial.println("connected");
    } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        delay(5000);
    }
    }
}

void logToSPIFFS(const char* message) {
    File logFile = SPIFFS.open("/log.txt", FILE_APPEND);
    if (!logFile) {
    Serial.println("Failed to open log file for writing");
    return;
    }
        logFile.println(message);
        logFile.close();
}

void setup() {
    Serial.begin(115200);

    if (!SPIFFS.begin(true)) {
    Serial.println("An error occurred while mounting SPIFFS");
    return;
    }

    setup_wifi();
    client.setServer(mqttServer, mqttPort);

    dht.setup(DHTPIN, DHTesp::DHT22);
    pinMode(LED_AZUL, OUTPUT);
    pinMode(LED_ROSA, OUTPUT);
    digitalWrite(LED_AZUL, LOW);
    digitalWrite(LED_ROSA, LOW);
}

void loop() {
    if (!client.connected()) {
    reconnect();
    }
        client.loop();

    temperatura = dht.getTemperature();
    posicao = map(analogRead(POT_PIN), 0, 4095, 0, 100);

    if (temperatura > 30) {
    if (!ledAzulLigado && !exibiuJanelaFechada) {
            const char* message = "Janela fechando...";
            Serial.println(message);
            client.publish("Kaioalves/feeds/janela-status", message);
            logToSPIFFS(message);
            digitalWrite(LED_AZUL, HIGH);
            ledAzulLigado = true;
            ledRosaLigado = false;
            exibiuJanelaFechada = false;
    }
    if (posicao == 100 && !exibiuJanelaFechada) {
            digitalWrite(LED_AZUL, LOW);
            const char* message = "Janela fechada";
            Serial.println(message);
            client.publish("Kaioalves/feeds/janela-status", message);
            logToSPIFFS(message);

        char tempStr[8];
        dtostrf(temperatura, 1, 2, tempStr);
        client.publish("Kaioalves/feeds/temperatura", tempStr);
        exibiuJanelaFechada = true;
        ledAzulLigado = false;
    }
    } else if (temperatura < 24) {
    if (!ledRosaLigado && !exibiuJanelaAberta) {
        const char* message = "Janela abrindo...";
        Serial.println(message);
        client.publish("Kaioalves/feeds/janela-status", message);
        logToSPIFFS(message);
        digitalWrite(LED_ROSA, HIGH);
        ledRosaLigado = true;
        ledAzulLigado = false;
        exibiuJanelaAberta = false;
    }
    if (posicao == 0 && !exibiuJanelaAberta) {
        digitalWrite(LED_ROSA, LOW);
        const char* message = "Janela aberta";
        Serial.println(message);
        client.publish("Kaioalves/feeds/janela-status", message);
        logToSPIFFS(message);

        char tempStr[8];
        dtostrf(temperatura, 1, 2, tempStr);
        client.publish("Kaioalves/feeds/temperatura", tempStr);
        exibiuJanelaAberta = true;
        ledRosaLigado = false;
    }
    } else {
    if (ledAzulLigado || ledRosaLigado) {
        digitalWrite(LED_AZUL, LOW);
        digitalWrite(LED_ROSA, LOW);
        ledAzulLigado = false;
        ledRosaLigado = false;
    }
    }

    if (temperatura <= 30 && temperatura >= 24) {
    exibiuJanelaFechada = false;
    exibiuJanelaAberta = false;
        }

    delay(500);
}
