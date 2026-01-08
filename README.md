test par chat gpt:
# 🎮 KrampusEngine

![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-0078D6.svg)
![Build](https://img.shields.io/badge/Build-Premake5-orange.svg)
![Compiler](https://img.shields.io/badge/Compiler-MSVC-purple.svg)
![Library](https://img.shields.io/badge/Library-SFML-lightgrey.svg)

**KrampusEngine** est un moteur de jeu 2D développé en **C++20**, conçu comme un **projet personnel** visant à explorer une architecture moderne, modulaire et orientée performance pour le développement de jeux **entièrement en code**.

Le moteur repose sur **SFML** pour le rendu, l’audio et les entrées, et utilise **Premake5** pour la génération de projets Visual Studio sous **Windows**.

---

## 📌 Sommaire

- ✨ Présentation
- 🚀 Fonctionnalités
- 🛠️ Technologies
- ⚙️ Prérequis
- 🔧 Installation & Build
- 🧱 Architecture
- 📁 Structure du projet
- 🛣️ Roadmap
- 🤝 Contribution
- 📄 Licence

---

## ✨ Présentation

KrampusEngine a pour objectif de fournir une base claire et extensible pour le développement de jeux 2D en C++, sans couche de scripting.  
Toutes les mécaniques, systèmes et comportements sont implémentés directement en **code natif**, favorisant la maîtrise totale du moteur et de ses performances.

Ce projet est avant tout :
- un **terrain d’apprentissage avancé**
- une **base technique sérieuse**
- un moteur **orienté développeurs**

---

## 🚀 Fonctionnalités

Le moteur inclut actuellement :

- 🪟 Gestion de la fenêtre & boucle principale
- ⌨️🖱️ Système d’input (clavier / souris)
- 🎬 Système de scènes
- 🧩 Entités & composants
- 🎨 Rendu 2D (SFML)
- 🔊 Audio & musique
- 📦 Gestion de ressources
- 📐 Physique & collisions basiques
- 🧪 UI simple intégrée
- 🛠️ Architecture modulaire et extensible

> ❌ Aucun système de scripting  
> ✅ Moteur **100 % C++ / code-driven**

---

## 🛠️ Technologies

- **Langage** : C++20  
- **Plateforme** : Windows  
- **Compilateur** : MSVC  
- **Build system** : Premake5  
- **Bibliothèque multimédia** : SFML  

---

## ⚙️ Prérequis

Avant de compiler KrampusEngine, assure-toi d’avoir :

- **Visual Studio** (2019 ou 2022 recommandé)
- **Premake5** accessible depuis le PATH
- Un environnement Windows 64 bits

> SFML est déjà incluse dans le dépôt via le dossier `ThirdParty`.

---

## 🔧 Installation & Build

1. **Cloner le dépôt**
   ```bash
   git clone https://github.com/Oronte/KrampusEngine.git
   cd KrampusEngine
