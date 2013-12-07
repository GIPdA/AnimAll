AnimAll
=======

Feeder part of AnimAll project


----

## Prérequis

1. Raspberry Pi (Raspbian)
2. [WiringPi2](http://wiringpi.com/download-and-install/)
3. Compilateur gcc...


## Compilation

`g++ -Wall main.cpp feeder.c -o feeder -lwiringPi`

## Utilisation de feeder

Initialiser la couche I/O en appelant `bFeeder_setup()`.
**Le programme doit posséder les droits root ou quittera avec une erreur !**

Lancer des cycles en appelant `iFeeder_startCycle()`.

Stopper le moteur immédiatement avec `vFeeder_stop()`.

Stopper les cycles en appelant `vFeeder_cancel()` (le moteur est stoppé à la fin du cycle en cours).

Utiliser `bFeeder_isRunning()` pour savoir si un cycle est en cours ou non.

Utiliser `iFeeder_remainingCycles()` pour connaitre le nombre de cycles restants.

Avant de quitter le programme, appeler `vFeeder_cleanup()` pour "nettoyer" la couche I/O.