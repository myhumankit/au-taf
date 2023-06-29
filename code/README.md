# Code

## Utilisation

Le code source se divise en 2 parties : l'exécutable et la configuration.

### **Exécutable**

L'exécutable est identique pour tout le monde et une version compilée est disponible sur le Github (seulement le français et l'anglais).

Si vous désirez compiler par vous-même, les explications pour la compilation sont disponibles plus bas à la section [Compilation](#compilation-de-lexécutable).

### **Config**

La configuration en revanche est personnelle et requiert à l'utilisateur de la paramétrer en fonction de ses besoins.
Les explications sur la configuration sont disponibles plus bas dans la section [Configuration](#configuration).


# Compilation de l'exécutable

Il existe plusieurs façons de compiler l'exécutable. Il n'est décrit ici que 2 façons : 1 pour Linux et 1 pour Windows. Les commandes à utiliser ne sont donc que des propositions et peuvent changer en fonction de l'environnement de la machine.

## Prérequis pour la compilation de l'exécutable

Compilateur :
- arm-none-eabi
    - Linux : [Site arm](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) où via le gestionnaire de package de la distribution (apt-get pour Ubuntu par exemple)
    - Windows : [MSYS2](https://www.msys2.org/) avec la commande
    ```
    pacman -S mingw-w64-x86_64-arm-none-eabi-gcc
    ```


Outils :
- Make :
    - Linux : disponible de base
    - Windows : via l'installation de [MSYS2](https://www.msys2.org/)
- Ninja (Linux seulement) : via le gestionnaire de package
- CMake
    -  Linux : via le gestionnaire de package si disponible ou [ici](https://cmake.org/download/)
    - Windows : télécharger l'installateur [ici](https://cmake.org/download/)

## Étapes pour la compilation

### Mise en place de l'environnement

Après avoir cloné le git avec la commande suivante :
```
git clone https://github.com/myhumankit/au-taf.git
```
On initialise les sous-modules du git avec la commande (en se plaçant dans le répertoire) :
```
git submodule update --init --recursive
```
On récupère enfin la librairie STM32_Audio disponible sur le site de [ST](https://www.st.com/en/embedded-software/stm32cubewb.html). La version github ne dispose pas de la librairie par soucis de licence, il faut donc prendre celle avec le bouton "Get latest" puis accepter la licence. Le téléchargement ne requiert qu'une adresse email valide.

Après avoir extrait le dossier, on copie le répertoire `Middlewares/ST/STM32_Audio`dans celui du projet (`Middlewares/ST/STM32_Audio`).

### Compilation

On choisit la bonne la langue dans le [CMakeLists](Projects/STM32F429I-Discovery/Applications/au-taf/CMakeLists.txt) du projet (variable PICOVOICE_LANGUAGE). Les choix possibles sont les suivants :
 * Anglais : `en`
 * Allemand : `de`
 * Français : `fr`
 * Italien : `it`

D'autres langages sont disponibles dont la liste sont les dossiers dans [stm32f411](/code/Modules/picovoice/sdk/mcu/lib/stm32f411/).

Une fois que la langue est choisie, on peut générer les fichiers de compilation avec CMake. On utilise la commande suivante dans le répertoire `code` où l'on spécifie le chemin d'accès au compilateur :

* Windows (avec MSYS2) :
    ```
    cmake -S . -B build -DCMAKE_C_COMPILER:FILEPATH=C:\msys64\mingw64\bin\arm-none-eabi-gcc.exe -DCMAKE_CXX_COMPILER:FILEPATH=C:\msys64\mingw64\bin\arm-none-eabi-g++.exe
    ```
    Remarque : vérifier le nom du disque (noté `C` ici) où MSYS2 est installé si jamais il a été modifié lors de l'installation.
* Linux :
    ```
    cmake -S . -B build -DCMAKE_C_COMPILER:FILEPATH=C:/usr/bin/arm-none-eabi-gcc -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/arm-none-eabi-g++
    ```

L'exécutable est alors ensuite dans le dossier `build/Projects/STM32F429I-Discovery/Applications/au-taf` et se nomme picovoice.

# Configuration

La configuration requiert l'exécutable (soit téléchargé, soit compilé depuis le code source comme décrit plus [haut](#compilation)). On place ensuite l'exécutable dans le dossier [Json](/code/Json/) avec `picovoice` comme nom.

Il faut ensuite générer les `wake-words` via [Picovoice](https://console.picovoice.ai/). Les instructions détaillées sont disponibles sur la page du projet au-taf sur MHK.

La configuration passe ensuite par l'édition sur fichier [config.json](/code/Json/config.json) qui se découpe en 3 sections :

* **Menu** : décrit les différents menus et commande disponibles (un exemple est fourni).
* **Picovoice** : configuration de Picovoice (clé + wake-words)
* **Phone** : numéro de téléphone

## Menu

## Picovoice

On renseigne la clé de l'utilisateur dans le champ `key` ainsi que le langage `language` utilisé (il doit être identique à celui de l'exécutable). On donne ensuite le chemin des 3 où 4 mots (.ppn) généré via Picovoice. Le mot `command` est optionnel et peut être supprimé (la sortie de mise en veille se fera alors avec le mot pour `back`).

## Phone