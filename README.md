
![alt text](https://github.com/Chadi25/SA-4.IOM.01-Impl-menter-les-r-seaux-et-protocoles-sans-fil-d-di-s-l-IoT/blob/main/https://www.creativefabrica.com/wp-content/uploads/2022/07/17/Data-Center-Logo-Graphics-34330071-1.jpg?raw=true)

# Projet de Sécurisation Datacenter avec IoT

## Introduction

Ce projet vise à établir un système de sécurité avancé pour un datacenter en utilisant la technologie IoT. Il implique la mise en place de différents capteurs dans des zones spécifiques (rouge, jaune, verte) pour surveiller et contrôler l'accès ainsi que les conditions environnementales.

## Structure du Projet

Le projet est structuré autour de l'utilisation de plusieurs applications et outils tels que **MQTT** pour la messagerie, **Node-RED** pour le flux de données, **Grafana** pour la visualisation, et **InfluxDB** pour la base de données. Des capteurs spécifiques sont déployés dans chaque zone pour garantir une surveillance adéquate.

## Mise en Œuvre des Applications

- **MQTT** : Configurer le broker MQTT pour permettre la communication entre les dispositifs IoT et le serveur.
- **Node-RED** : Utiliser Node-RED pour créer des flux de données logiques et automatiser les actions en fonction des données des capteurs.
- **Grafana** : Intégrer Grafana pour visualiser en temps réel les données collectées par les capteurs.
- **InfluxDB** : Stocker les données des capteurs dans InfluxDB pour l'analyse et la récupération.

## Configuration des Capteurs par Zone

### Zone Rouge : Cœur du Datacenter

- **Lecteur de carte RFID** : Contrôle l'accès basé sur des badges RFID autorisés.
- **Capteur de vibration** : Détecte toute vibration indiquant une tentative d'effraction.
- **Détecteur de présence** : Surveille la présence non autorisée dans la zone.
- **Capteur de température et d'humidité** : Assure le maintien des conditions optimales pour le matériel du datacenter.
- **Capteur de fumée** : Détecte la présence de fumée pouvant indiquer un début d'incendie.
- **LED jaune** : Indicateur visuel pour l'état d'alerte.

### Zone Jaune : Zone Technique

- **Détecteur de présence**
- **Capteur de température et d'humidité**
- **Capteur de fumée**
- **LED verte**

### Zone Verte : Hall d'Accueil, Salles de Réunion

- **Capteur de température et d'humidité**
- **Capteur de fumée**

## Codes des Capteurs

Les scripts pour chaque zone sont fournis dans le répertoire `codes`. Chaque script est dédié à une zone spécifique et configure les capteurs appropriés.

### Zone Rouge

Deux scripts distincts pour la configuration des capteurs RFID, de température, d'humidité, de fumée, et de présence.

### Zone Jaune

Script configurant les détecteurs de présence, de fumée, et les capteurs de température et d'humidité.

### Zone Verte

Script pour les capteurs de température, d'humidité, et de fumée.

## Guide d'Installation

Chaque script contient des instructions détaillées pour la configuration et le déploiement des capteurs. Suivez les étapes indiquées dans les commentaires de chaque fichier de code.

> **Note :** Chaque code de zone doit être lié à son ESP8266 à l'exception de la zone rouge qui en raison de son grand nombre de capteurs nécessite un deuxième ESP (manque de digital pin). Une fois les paramètres du broker et Wi-Fi modifiés dans le code, il faudra tout téléverser les codes et observer avec un sub les topics.

## Contribution

Nous encourageons les contributions pour améliorer et étendre les fonctionnalités du système de sécurité. N'hésitez pas à soumettre des pull requests ou à signaler des problèmes.

## Licence

Ce projet est distribué sous la licence IOM DATACENTER. Veuillez consulter le fichier `LICENSE` pour plus d'informations.


