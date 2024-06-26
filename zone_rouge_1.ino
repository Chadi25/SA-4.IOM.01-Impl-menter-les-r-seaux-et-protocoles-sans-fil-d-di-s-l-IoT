#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <DHT.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Broches pour le capteur de température et d'humidité
#define DHT_PIN D3
#define DHT_TYPE DHT22

// Broches pour le lecteur RFID
#define SS_PIN D8
#define RST_PIN D4

// Broches pour les LEDs
#define RED_LED D0
#define YELLOW_LED D1
#define GREEN_LED D2

// Initialisation des capteurs
DHT dht(DHT_PIN, DHT_TYPE);
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Configuration du réseau WiFi
const char* ssid = "Groupe3-IoT"; // Remplacez par le SSID de votre réseau WiFi
const char* password = "tpRT9025"; // Remplacez par le mot de passe de votre réseau WiFi

// Configuration du serveur MQTT
const char* mqtt_server = "192.168.1.100"; // Remplacez par l'adresse IP de votre serveur MQTT
const char* rfidTopic = "datacenter/sensors/RFID";

// Initialisation du client WiFi et MQTT
WiFiClientSecure espClient;
PubSubClient client(espClient);

// Liste des cartes autorisées
String card_list[10] = {"40AC1C7A"}; // Carte admin pré-enregistrée
int total_card = 1; // Commence à 1 en raison de la carte admin
String card_num;

// Variables pour le mode d'enregistrement
#define DELAI_PASSAGE_CARTE_MAITRE 10000
bool isRegisterMode = false;
unsigned long masterCardPresentedTime = 0;

// Variable pour vérifier si le capteur de température est trouvé
bool temperatureSensorFound = false;

// Variables pour stocker l'heure de début et de fin de l'accès
unsigned long accessStartTime = 0;
unsigned long accessEndTime = 0;

// Variables pour la gestion du temps
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 7200); // Décalage horaire pour la France (7200 secondes)

// Fonction de configuration, appelée une fois au démarrage
void setup() {
  Serial.begin(115200);

  // Configuration du client MQTT et du réseau WiFi
  espClient.setInsecure();
  client.setServer(mqtt_server, 8883);
  setup_wifi();

  // Initialisation des capteurs et des broches
  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  setupSensors(); 

  // Démarrer la mise à jour de l'heure via NTP
  timeClient.begin();
}

// Fonction principale, exécutée en boucle
void loop() {
  // Mettre à jour l'heure via NTP
  timeClient.update();

  // Vérification de la connexion MQTT
  if (!client.connected()) {
    reconnect();
  } else {
    // Vérification du lecteur RFID
    checkRFID();
    
    // Lecture et publication de la température et de l'humidité
    readAndPublishTemperature();
    readAndPublishHumidity();
    delay(1000);
  }
  Serial.println("\n -------------------"); // Ajout d'une nouvelle ligne
  // Maintien de la connexion MQTT active
  client.loop();
}

// Fonction pour obtenir le numéro de carte RFID
String getCardNumber() {
  String UID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    UID.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    UID.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  UID.toUpperCase();
  return UID;
}

// Fonction pour vérifier le lecteur RFID
void checkRFID() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    card_num = getCardNumber();

    // Vérification de la carte admin
    if (card_num == "40AC1C7A") {
      // Mode d'enregistrement activé par la carte admin
      isRegisterMode = true;
      masterCardPresentedTime = millis();
      Serial.println("Mode d'enregistrement activé par la carte admin.");
      digitalWrite(YELLOW_LED, HIGH);
      delay(1000);
      digitalWrite(YELLOW_LED, LOW);
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      return;
    }

    // Gestion du mode d'enregistrement
    if (isRegisterMode && millis() - masterCardPresentedTime > DELAI_PASSAGE_CARTE_MAITRE) {
      isRegisterMode = false;
      Serial.println("Mode d'enregistrement désactivé (temps écoulé).");
      digitalWrite(YELLOW_LED, LOW);
    }

    // Enregistrement d'une nouvelle carte
    if (isRegisterMode) {
      if (total_card < 10) {
        card_list[total_card++] = card_num;
        Serial.println("Nouvelle carte enregistrée avec succès.");
        isRegisterMode = false;
      } else {
        Serial.println("Limite de cartes atteinte.");
      }
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      return;
    }

    // Vérification de l'accès autorisé
    showData();
  }
}

// Fonction pour afficher les données
void showData() {
  bool accessGranted = false;
  for (int i = 0; i < total_card; i++) {
    if (card_list[i].equals(card_num)) {
      accessGranted = true;
      break;
    }
  }

  // Gestion de l'accès
  if (accessGranted) {
    Serial.println("Accès autorisé");
    digitalWrite(GREEN_LED, HIGH);
    delay(1000);
    digitalWrite(GREEN_LED, LOW);
    accessStartTime = millis();
    unsigned long accessDuration = (millis() - accessStartTime) / 1000;
    String dateTime = getCurrentTime();
    String message = "ID carte : " + card_num + ", Accès autorisé, Date : " + dateTime + ", Durée d'accès : " + String(accessDuration) + " secondes";
    publishMessage(message);
  } else {
    Serial.println("Accès refusé");
    digitalWrite(RED_LED, HIGH);
    delay(1000);
    digitalWrite(RED_LED, LOW);
    String dateTime = getCurrentTime();
    String message = "ID carte : " + card_num + ", Accès refusé, Date : " + dateTime;
    publishMessage(message);
  }
}

// Fonction pour publier un message sur le topic MQTT
void publishMessage(String message) {
  // Attendre que la connexion au serveur MQTT soit établie
  while (!client.connected()) {
    Serial.println("Connexion au serveur MQTT en cours...");
    if (client.connect("ESP8266Client", "Groupe3-IoT", "tpRT9025")) {
      Serial.println("Connecté au serveur MQTT");
    } else {
      Serial.println("Échec de connexion au serveur MQTT. Nouvelle tentative dans 5 secondes...");
      delay(5000); // Attendre 5 secondes avant la nouvelle tentative
    }
  }

  // Publier le message sur le topic MQTT
  if (client.publish(rfidTopic, message.c_str())) {
    Serial.println("Message publié avec succès sur le topic RFID");
  } else {
    Serial.println("Échec de la publication du message sur le topic RFID");
  }
}


// Fonction pour initialiser les capteurs
void setupSensors() {
  dht.begin();
  Serial.println("Capteur DHT initialisé");
  temperatureSensorFound = true;
}

// Fonction pour lire et publier la température
void readAndPublishTemperature() {
  if (temperatureSensorFound) {
    float temperature = dht.readTemperature();
    if (!isnan(temperature)) {
      Serial.print("Température: ");
      Serial.print(temperature);
      Serial.println(" °C");
      if (client.connected()) {
        client.publish("datacenter/sensors/temperature", String(temperature).c_str());
      } else {
        reconnect();
      }
    } else {
      Serial.println("Échec de lecture de la température !");
    }
  } else {
    Serial.println("Capteur de température introuvable !");
  }
}

// Fonction pour lire et publier l'humidité
void readAndPublishHumidity() {
  if (temperatureSensorFound) {
    float humidity = dht.readHumidity();
    if (!isnan(humidity)) {
      Serial.print("Humidité: ");
      Serial.print(humidity);
      Serial.println(" %");
      if (client.connected()) {
        client.publish("datacenter/sensors/humidite", String(humidity).c_str());
      } else {
        reconnect();
      }
    } else {
      Serial.println("Échec de lecture de l'humidité !");
    }
  } else {
    Serial.println("Capteur de température introuvable !");
  }
}

// Fonction pour obtenir l'heure actuelle sous forme de chaîne de caractères
String getCurrentTime() {
  return timeClient.getFormattedTime();
}

// Fonction de configuration du réseau WiFi
void setup_wifi() {
  Serial.print("Connexion à ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connecté");
  Serial.println("Adresse IP: ");
  Serial.println(WiFi.localIP());
}

// Fonction pour reconnecter au serveur MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentative de connexion au serveur MQTT...");
    if (client.connect("ESP8266Client", "Groupe3-IoT", "tpRT9025")) {
      Serial.println("Connecté");
    } else {
      Serial.print("Échec, rc=");
      Serial.print(client.state());
      Serial.println(" nouvelle tentative dans 5 secondes");
      delay(5000);
    }
  }
}
