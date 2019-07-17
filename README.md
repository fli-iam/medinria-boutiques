# Boutiques GUI

A small Qt GUI to search and execute Boutiques apps.

![Boutique Screenshot](boutiques-gui.png)

## Requirements

 - python3
 - boutiques (pip install boutiques)
 - Qt5
 - Docker or Singularity

## Python version (in python/)

### Install

 - Clone this repo: `git clone https://github.com/fli-iam/medinria-boutiques.git`
 - Initialize a [python3 virutal environment](https://docs.python.org/3/library/venv.html) in the *medinria-boutiques* python directory: `cd medinria-boutiques/python/ && python3 -m venv .`
 - Activate the virutal environment: `source bin/activate`
 - Install PySide2: `pip install PySide2`

### Usage

With the virutal environment activated, run `python3 main.py` from the `python/app/ directory`.

## C++ version (in cpp/)

### Development

#### Open in QtCreator

In *QtCreator*, open `cpp/src/CMakeLists.txt` and set the build directory where you want.

#### Compile in QtCreator

 - Create a new Clang 8 / CMake kit: 
   - Projects > Manage kits > Clone the default kit and modify it to use Clang 8 and CMake
   - The build directories must be at the same level as the `cpp/src/` directory to find the bosh executable in `cpp/`

#### Run

*Note: BoutiqueGUI will look for the `bosh` program one level up where it is executed ; 
therefor the `bosh` script is in the *medinria-boutiques* cpp directory: `cpp/bosh`.*