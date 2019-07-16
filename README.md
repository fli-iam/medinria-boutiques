# Boutiques GUI

A small Qt GUI to search and execute Boutiques apps.

![Boutique Screenshot](boutiques-gui.png)

## Requirements

 - Qt5
 - Docker or Singularity

## Python version (in python/)

### Requirements

 - python3

### Install

 - Clone this repo: `git clone https://github.com/fli-iam/medinria-boutiques.git`
 - Initialize a [python3 virutal environment](https://docs.python.org/3/library/venv.html) in the *medinria-boutiques* python dir: `cd medinria-boutiques/python/ && python3 -m venv .`
 - Activate the virutal environment: `source bin/activate`
 - Install PySide2: `pip install PySide2`

### Usage

With the virutal environment activated, run `python3 main.py` from the `python/app/ directory`.

## C++ version (in cpp/)

### Open in QtCreator

In *QtCreator*, open `cpp/src/CMakeLists.txt` and set the build directory where you want.