#include <DHT.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

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

// Broche du capteur PIR et broche de la LED
const int pirPin = 16;
const int ledPin = 2;

void setup() {
  // Configuration des broches en entrée ou en sortie
  pinMode(MQ135_PIN, INPUT);
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  
  Serial.begin(9600);
  setupWiFi();
}

void loop() {
  // Vérification de la connexion au serveur MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Lecture et publication de la qualité de l'air
  readAndPublishAirQuality(MQ135_PIN);
  
  // Lecture et publication du mouvement
  readAndPublishMovement();
  
  // Attendre une seconde avant la prochaine itération
  Serial.println("\n-------------------"); // Ajout d'une nouvelle ligne
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

void readAndPublishAirQuality(int gasSensor) {
  // Lecture du niveau de gaz (MQ-5)
  float gasLevel = analogRead(gasSensor) * 100.0 / 1023;
  
  // Affichage du niveau de gaz
  Serial.print("Niveau de gaz : ");
  Serial.print(gasLevel);
  Serial.println("%");
  
  // Publication du niveau de gaz sur le serveur MQTT si connecté
  if (client.connected()) {
    client.publish("datacenter/sensors/fumee", String(gasLevel).c_str());
  } else {
    Serial.println("Erreur : Non connecté au broker MQTT !");
  }
}

void readAndPublishMovement() {
  int pirState = digitalRead(pirPin); // Lire l'état du capteur PIR
  
  if (pirState == HIGH) { // Si le capteur détecte un mouvement
    digitalWrite(ledPin, HIGH); // Allumer la LED
    Serial.println("True"); // Afficher un message de débogage
    
    if (client.connected()) {
      client.publish("datacenter/sensors/mouvement", "1");
    } else {
      Serial.println("Erreur : Non connecté au broker MQTT !");
    }
    
    delay(1000); // Attendre pendant 1 seconde
  } else {
    digitalWrite(ledPin, LOW); // Éteindre la LED si aucun mouvement n'est détecté
    
    if (client.connected()) {
      client.publish("datacenter/sensors/mouvement", "0");
    } else {
      Serial.println("Erreur : Non connecté au broker MQTT !");
    }
    Serial.println("False"); //
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
