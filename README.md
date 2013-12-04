AnimAll
=======

Hardware part of AnimAll project


----

## Prérequis

1. Raspberry Pi (Raspbian)
2. WiringPi2
3. Compilateur gcc...


## Installation

1. Compiler *distrib* (`g++ -Wall distrib.cpp -o distrib -lwiringPi -lpthread`)
2. Copier le binaire *distrib* dans */usr/local/bin*
3. Copier *distrib_startup.sh* dans */etc/init.d/*
4. Modifier le chemin du fichier de contrôle si besoin dans *distrib_startup.sh*

## Utilisation

##### Commandes :
Écrire `start` avec un retour à la ligne dans le fichier de contrôle pour lancer un cycle. `stop` pour arrêter le moteur.

Écrire plusieurs `start` pour lancer plusieurs cycles d'affilée (séparés par un retour à la ligne).
