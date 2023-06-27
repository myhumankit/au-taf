# Code

## Utilisation

Le code source se divise en 2 parties : l'exécutable et la configuration.

### Exécutable

L'exécutable est identique pour tout le monde et une version compilée est disponible sur le Github.
Si vous désirez compiler par vous-même, les explications pour la compilation sont disponibles dans le dossier [executable](executable/README.md)

### Config

La configuration en revanche est personnelle et requiert que l'utilisateur la paramètre en fonction de ses besoins.
Les explications sur la configuration est disponible dans le dossier [config](config/README.md)


# Exécutable

## Prérequis pour la compilation de l'exécutable

Compilateur :
- arm-none-eabi

Outils :
- CMake
- Ninja

## Étapes pour la compilation

### STM32 Firmware
Récupérer le firmware STM32 disponible [ici](https://www.st.com/en/embedded-software/stm32cubef4.html).
Le téléchargement ne requiert qu'une adresse email valide.

Remarque : la version disponible n'est pas complète, il faut bien prendre celle sur le site (bouton "Get latest").

On peut ensuite extraire le contenu dans le répertoire où l'on souhaite travailler pour compiler.

### Picovoice

Picovoice n'est pas inclus dans le firmware, il faut donc l'installer manuellement.
On commence par créer un dossier Modules dans le répertoire STM32Cube_FX_Vx.xx.x (où les x représente la version du firmware).

On récupère le code sur le Github de Picovoice ou directement sur ce [lien](https://github.com/Picovoice/porcupine) en téléchargeant le .zip.
On extrait ensuite le contenu dans le répertoire Modules.
On obtient l'arborescence suivante : *Insérer une image*

### Au-taf
On copie le répertoire *executable* dans le répertoire Applications du Firmware (Projects/STM32F429I-Discovery/Applications).
 
