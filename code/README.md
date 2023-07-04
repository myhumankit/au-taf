# Code

## Utilisation

Le code source se divise en 2 parties : l'exécutable et la configuration.

### **Exécutable**

L'exécutable est identique pour tout le monde et une version compilée est disponible sur le Github (seulement le français et l'anglais).

Si vous désirez compiler par vous-même ou faire des modifications, des explications sont disponibles plus bas à la section [Compilation](#compilation-de-lexécutable).

### **Config**

La configuration est obligatoire et requiert que l'utilisateur la paramètre en fonction de ses besoins.
Les explications sur la configuration sont disponibles plus bas dans la section [Configuration](#configuration).


# Compilation de l'exécutable

Il existe plusieurs façons de compiler l'exécutable.
Il n'est décrit ici que 2 façons : 1 pour Linux et 1 pour Windows.
Les commandes à utiliser ne sont donc que des propositions et peuvent changer en fonction de l'environnement de la machine et du choix de l'utilisateur.

## Prérequis pour la compilation de l'exécutable

Compilateur :
- arm-none-eabi
    - Linux : [Site arm](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) où via le gestionnaire de package de la distribution si disponible (apt-get pour Ubuntu par exemple)
    - Windows : [MSYS2](https://www.msys2.org/) avec la commande
    ```
    pacman -S mingw-w64-x86_64-arm-none-eabi-gcc
    ```

Outils :
- [Git](https://code.visualstudio.com/https://code.visualstudio.com/) 
- [Visual Studio Code](https://code.visualstudio.com/) ou dérivé avec l'extension CMake Tools installable depuis Visual Studio Code.
- Make :
    - Linux : disponible de base sur la plupart des distributions sinon via le gestionnaire de package
    - Windows : via l'installation de [MSYS2](https://www.msys2.org/)
- Ninja (Linux seulement) : via le gestionnaire de package
- CMake
    - Linux : via le gestionnaire de package si disponible ou [ici](https://cmake.org/download/)
    - Windows : via l'installateur [ici](https://cmake.org/download/)

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

Après avoir extrait le dossier, on copie le répertoire `Middlewares/ST/STM32_Audio` dans celui du projet (`Middlewares/ST/STM32_Audio`).

### Compilation

On choisit la bonne la langue dans le [CMakeLists](Projects/STM32F429I-Discovery/Applications/au-taf/CMakeLists.txt) du projet (variable PICOVOICE_LANGUAGE). Les choix possibles sont les suivants :
 * Anglais : `en`
 * Allemand : `de`
 * Français : `fr`
 * Italien : `it`

D'autres langages sont disponibles, la liste complète correspond aux dossiers le répertoire [stm32f411](/code/Modules/picovoice/sdk/mcu/lib/stm32f411/).

Une fois que la langue est choisie, on peut générer les fichiers de compilation avec CMake. 
Pour cela, on ouvre le répertoire `code` dans Visual Studio Code. Si l'extension CMake Tools a bien été installé, on peut lui demander de rechercher les compilateurs installés sur la machine.
On peut ensuite sélectionner le compilateur `arm-none-eabi` et la configuration avec CMake se lancera.
Pour la compilation, il suffit de cliquer sur `build` en bas de l'écran.

L'exécutable se trouve ensuite dans le dossier `build/Projects/STM32F429I-Discovery/Applications/au-taf` et se nomme picovoice.

# Configuration

La configuration requiert l'exécutable (soit téléchargé, soit compilé depuis le code source comme décrit plus [haut](#compilation)). On place ensuite l'exécutable dans le dossier [Json](/code/Json/) avec `picovoice` comme nom.

Il faut ensuite générer les `wake-words` via [Picovoice](https://console.picovoice.ai/). Les instructions détaillées sont disponibles sur la page du projet au-taf sur MHK.

## Fichier `config.json`

La configuration passe ensuite par l'édition sur fichier [config.json](/code/Json/config.json) qui se découpe en 3 sections :

* **Menu** : décrit les différents menus et commande disponibles (un exemple est fourni).
* **Picovoice** : configuration de Picovoice (clé + wake-words)
* **Phone** : numéro de téléphone

### Menu

Le point d'entrée pour les menus est "START", c'est ici qu'on se trouve lors de la sortie de veille. On peut ensuite naviguer dans les menus ou éxecuter des commandes. La syntaxe est la suivante :

["Nom", [["Nom_de_la_commande", "Parametre1", "Parametre2", ... ]]]. Le `Nom` correspond à ce qui va être affiché sur l'écran. Tandis que `Nom_de_la_commande` fait référence au nom qui sont présents dans l'exécutable et les `Parametre` sont liés à la commande en question. Voici les choix qui sont possibles :
- MENU : Changement de menu
    - *Nom du menu*
- RF_433 : Radio fréquence
    - PEREL
        - 0x155533
        - 0x1555c3
        - 0x155703

### Picovoice

On renseigne la clé de l'utilisateur dans le champ `key` ainsi que le langage `language` utilisé (il doit être identique à celui de l'exécutable). On donne ensuite le chemin des 3 où 4 mots (.ppn) généré via Picovoice. Le mot `command` est optionnel et peut être supprimé. La sortie de mise en veille se fait alors avec le mot pour `back`.

### Phone

Non utilisé pour le moment.

# Génération de l'exécutable final

## Prérequis pour la compilation de l'exécutable final

- [Python 3](https://www.python.org/downloads/)
- Python-lief 0.13+ : 
    ```
    pip install lief
    ```
- Make

## Compilation

Une fois la configuration du ficher `config.json` terminée, il suffit d'exécuter la commande `make` dans le répertoire [Json](/Json/) après avoir placé l'exécutable `picovoice` dans ce dernier.