AnimAll
=======

Scale part of AnimAll project

*Untested*

----

## Prérequis

1. Raspberry Pi (Raspbian)
2. [WiringPi2](http://wiringpi.com/download-and-install/)
3. Compilateur gcc...


## Compilation

`g++ -Wall main.cpp scale.c -o scale -lwiringPi -lpthread`

or use the makefile.

## Utilisation de scale

Initialiser la couche I/O en appelant `bScale_setup()`.
**Le programme doit posséder les droits root sous peine de quitter avec une erreur !**

Efectuer une tare via `vScale_tareScale()` quelques secondes après le setup et dès que nécessaire ensuite. Sinon le premier appel de `bScale_getWeight(...)` effectue une tare.

Récupérer le poids de la balance via `bScale_getWeight(...)` autant de fois que nécessaire.

Avant de quitter le programme, appeler `vScale_cleanup()` pour "nettoyer" la couche I/O.