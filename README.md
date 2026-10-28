# Application de gestion de restaurant (C)

Application console en langage C pour gérer le **menu**, les **tables** et les **commandes**
d'un restaurant, avec sauvegarde automatique dans des fichiers texte.

## Fonctionnalités

- **Menu** : afficher, ajouter, modifier le prix, supprimer un plat
- **Tables** : afficher (libre / occupée et commande en cours), ajouter, supprimer
- **Commandes** : ouvrir une commande sur une table libre, ajouter / retirer des articles,
  afficher l'addition, encaisser (la table est libérée), annuler
- **Rapport** : nombre de commandes payées, chiffre d'affaires, panier moyen, plat le plus vendu
- **Persistance** : les données sont enregistrées dans `data/` après chaque modification
- **Saisies sécurisées** : toutes les entrées sont lues avec `fgets` et validées (pas de `scanf`)

## Compilation et lancement

Avec `make` :

```bash
make
./restaurant
```

Ou directement avec le compilateur :

```bash
gcc -std=c11 -Wall -Wextra -Iinclude src/*.c -o restaurant
./restaurant
```

Au premier lancement, un menu et 6 tables d'exemple sont créés dans le dossier `data/`.

## Structure du projet (programmation modulaire)

```
├── include/          # fichiers d'en-tête (.h) : l'interface de chaque module
│   ├── utils.h       # saisie sécurisée, texte, fichiers
│   ├── menu.h        # plats
│   ├── tables.h      # tables
│   ├── orders.h      # commandes
│   ├── app.h         # état global + chargement / sauvegarde
│   └── ui.h          # écrans et menus
├── src/              # implémentations (.c)
│   ├── main.c        # point d'entrée
│   └── ...
├── data/             # fichiers de données (créés automatiquement)
└── Makefile
```

## Structures de données

| Donnée | Structure | Pourquoi |
|---|---|---|
| Menu | tableau dynamique (`malloc` / `realloc`) | accès simple par parcours, taille qui grandit |
| Tables | tableau dynamique | petit ensemble, recherche par numéro |
| Commandes | **liste chaînée** de `Order` | ajout en fin et suppression au milieu sans décalage |
| Lignes d'une commande | tableau dynamique dans chaque `Order` | nombre d'articles variable |

Chaque ligne de commande **copie** le nom et le prix du plat au moment de la commande :
modifier ou supprimer un plat ne change pas les additions déjà ouvertes ou payées.

## Fichiers de données

Format texte, un enregistrement par ligne, séparateur `|` :

```
menu.txt     id|catégorie|nom|prix
tables.txt   numéro|places|statut        (0 = libre, 1 = occupée)
orders.txt   ORDER|id|table|statut       puis une ligne LINE|id_plat|nom|prix|quantité par article
```

## Gestion de la mémoire

Toute la mémoire allouée est libérée en fin de programme (`app_free`). Le code a été vérifié
avec `-fsanitize=address,undefined` : aucune fuite ni accès invalide détectés.

## Pistes d'amélioration

- Ajouter la date et l'heure des commandes et un rapport par jour
- Gérer des serveurs et des droits d'accès
- Remplacer les fichiers texte par une base SQLite
- Ajouter des tests unitaires pour les modules `menu`, `tables` et `orders`
