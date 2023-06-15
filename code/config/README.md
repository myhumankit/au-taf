# Configuration

## Fichier config.json

La configuration passe par la modification du fichier [config.json](config.json).

Il existe 4 grandes parties dans le fichier de configuration :
- COMMANDS : regroupe les différentes commandes que l'on souhaite pouvoir utiliser.
- ALIAS : sert lorsque 2 commandes sont identiques (allumer et éteindre un appareil par exemple).
- MENUS : définit comment les menus sont agencés pour retrouver facilement les différentes commandes définies dans la partie commande.
- CONFIGS : paramétrage pour le logiciel Picovoive (reconnaissance vocale) et regroupe les actions possibles utilisables pour les commandes.

### COMMANDS
Le choix de l'arborescence est libre, voir l'exemple dans [example_config.json](example_config.json) pour la syntaxe et une proposition d'arborescence.

### ALIAS
On rajoute entre les crochets les alias séparés par une virgule comme suivant : [["CHEMIN","COMMANDE"],["CHEMIN","ALIAS"]], ...

### MENUS
Les menus vont suivre ce qui est définis dans commandes (voir [example_config.json](example_config.json) pour la syntaxe)

### CONFIGS

#### Picovoice
- key : Clé d'accès Picovoice récupérable sur la page d'accueil du [site](https://console.picovoice.ai/)
- language : Langage utilisé pour la détection des mots, on peut choisir soit l'anglais (en) ou français (fr)
- command / next / ok / back : respectivement commande / suivant / confirmer / retour sont les mots utilisés pour naviguer et sélectionner les commandes.
    - file : le fichier du "wake word" récupéré sur Picovoice avec l'extension en .ppn
    - threshold : tolérance de la détection de mots (on peut mettre 0.75)

#### Options

Après la partie Picovoice, on rajoute les différentes commandes liées aux appareils que l'on souhaite utiliser (les trames radio fréquences par exemple). 