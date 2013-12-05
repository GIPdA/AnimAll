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

Lancer des cycles en appelant `uiFeeder_startCycle()`.

Avant de quitter le programme, appeler `vFeeder_cleanup()` pour "nettoyer" la couche I/O.