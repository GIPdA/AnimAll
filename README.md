AnimAll
=======

Hardware part of AnimAll project


----

## Prérequis

1. Raspberry Pi (Raspbian)
2. WiringPi2
3. Compilateur gcc...


## Installation

1. Compiler *distrib*
2. Copier le binaire *distrib* dans */usr/local/bin*
3. Copier *distrib_startup.sh* dans */etc/init.d/*
4. Modifier le chemin du fichier de contrôle si besoin dans *distrib_startup.sh*

## Utilisation

##### Commandes :
Écrire `start` avec un retour à la ligne dans le fichier de contrôle pour lancer un cycle. `stop` pour arrêter le moteur.

##### Acknowledge :
Si la commande est valide et le distributeur en attente, `<ok>` est ajouté à la fin du fichier :

	start
	<ok>

En cas de commande inconnue, `<err>` est ajouté, et si le distributeur est en cours d'execution d'un cycle, `<run>` est ajouté. Dans ce dernier cas la commande est ignorée, un nouveau cycle ne reprend pas automatiquement ensuite.
