#include <DHT.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

// Broche du capteur de température et d'humidité
#define DHT_PIN D3
#define DHT_TYPE DHT22

// Broche du capteur de qualité de l'air
#define MQ135_PIN A0

// Informations de connexion WiFi
const char* ssid = "Groupe3-IoT";
const char* password = "tpRT9025";

// Informations du serveur MQTT
const char* mqtt_server = "192.168.1.100";
const int mqtt_port = 8883; // Port MQTT sécurisé

WiFiClientSecure espClient;
PubSubClient client(espClient);

DHT dht(DHT_PIN, DHT_TYPE);
const int pirPin = 16; // Broche du capteur PIR
const int ledPin = 2; // Broche de la LED

void setup() {
  pinMode(MQ135_PIN, INPUT);
  pinMode(pirPin, INPUT); // Configurer la broche du capteur PIR en entrée
  pinMode(ledPin, OUTPUT); // Configurer la broche de la LED en sortie
  
  Serial.begin(9600);
  setupWiFi();
  setupTemperature();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Vérifier la connexion MQTT
  if (client.connected()) {
    Serial.println("Connecté au broker MQTT");
  } else {
    Serial.println("Erreur : Non connecté au broker MQTT !");
  }
  
  // Lecture et publication de la température et de l'humidité
  readAndPublishTemperature();
  readAndPublishHumidity();
  
  // Lecture et publication de la qualité de l'air
  readAndPublishAirQuality(MQ135_PIN);
  
  Serial.println("\n -------------------"); // Ajout d'une nouvelle ligne
  // Attendre une seconde avant la prochaine itération
  delay(1000);
}

void setupWiFi() {
  Serial.print("Connexion à ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  // Attendre que la connexion soit établie
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connecté");
  Serial.println("Adresse IP : ");
  Serial.println(WiFi.localIP());

  // Configuration du client MQTT avec le serveur et le port
  client.setServer(mqtt_server, mqtt_port);
  // Configuration du client MQTT avec le certificat (aucun pour ce cas)
  espClient.setInsecure();
}

void setupTemperature() {
  dht.begin();
}

void readAndPublishTemperature() {
  float temperature = dht.readTemperature();
  if (!isnan(temperature)) {
    Serial.print("Température : ");
    Serial.print(temperature);
    Serial.println(" °C");
    
    // Publication de la température sur le serveur MQTT si connecté
    if (client.connected()) {
      client.publish("accueil/sensors/temperature", String(temperature).c_str());
    } else {
      Serial.println("Erreur : Non connecté au broker MQTT !");
    }
  } else {
    Serial.println("Échec de lecture de la température !");
  }
}

void readAndPublishHumidity() {
  float humidity = dht.readHumidity();
  if (!isnan(humidity)) {
    Serial.print("Humidité : ");
    Serial.print(humidity);
    Serial.println(" %");
    
    // Publication de l'humidité sur le serveur MQTT si connecté
    if (client.connected()) {
      client.publish("accueil/sensors/humidite", String(humidity).c_str());
    } else {
      Serial.println("Erreur : Non connecté au broker MQTT !");
    }
  } else {
    Serial.println("Échec de lecture de l'humidité !");
  }
}

void readAndPublishAirQuality(int gasSensor) {
  // Lecture du niveau de gaz (MQ-5)
  float gasLevel = analogRead(gasSensor) * 100.0 / 1023;
  
  // Affichage du niveau de gaz
  Serial.print("Niveau de gaz : ");
  Serial.print(gasLevel);
  Serial.println("%");
  
  // Publication du niveau de gaz sur le serveur MQTT si connecté
  if (client.connected()) {
    client.publish("accueil/sensors/fumee", String(gasLevel).c_str());
  } else {
    Serial.println("Erreur : Non connecté au broker MQTT !");
  }
}

void reconnect() {
  // Tentative de reconnexion au broker MQTT
  while (!client.connected()) {
    Serial.print("Tentative de connexion au broker MQTT...");
    
    // Essayer de se connecter
    if (client.connect("ESP8266Client")) {
      Serial.println("connecté");
    } else {
      Serial.print("échec, rc=");
      Serial.print(client.state());
      Serial.println(" nouvelle tentative dans 5 secondes");
      // Attendre 5 secondes avant de réessayer
      delay(5000);
    }
  }
}
