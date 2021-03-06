# Boutiques GUI

A small Qt GUI to search and execute Boutiques apps.

![Boutique Screenshot](boutiques-gui.png)

## Requirements

 - Python 3
 - BoutiquesGUI-Data (download it [from the latest release](https://github.com/fli-iam/medinria-boutiques/releases), see the *Installation* section below)
 - Docker or Singularity

## Installation

 - Download [the latest linux release](https://github.com/fli-iam/medinria-boutiques/releases) to get the required *BoutiquesGUI-Data* directory, even if you use OS X or Windows (you will only need the *BoutiquesGUI-Data* directory inside *linux-x86_64.zip*).
 - Put the *BoutiquesGUI-Data* directory in the `cpp/`
 - (Optional) Make sure boutiques works with Python 3:
   - Go to the `BoutiquesGUI-Data/` directory: `cd cpp/BoutiquesGUI-Data/`
   - Test boutiques (it should return a list of tools): 
     - under Linux and OS X: `python3 boutiques/bosh.py search bet`
     - under Windows: `python/python.exe boutiques/bosh.py search bet`

## C++ version (in cpp/)

### Development

#### Open in QtCreator

In *QtCreator*, open `cpp/src/CMakeLists.txt` and set the build directory where you want.

#### Compile in QtCreator

 - Create a new Clang 8 / CMake kit: 
   - Projects > Manage kits > Clone the default kit and modify it to use Clang 8 and CMake
   - The build directories must be at the same level as the `cpp/src/` directory to find the boutiques data in `cpp/BoutiquesGUI-Data/`

#### Create / Install boutiques in BoutiquesGUI-Data

This is how to install the boutiques library along with its dependencies in the `BoutiquesGUI-Data` directory: 

 - From the location of your choice, create a python 3 virtual environment: `python3 -m venv boutiques` (this temporary environment is only used to install boutiques in an isolated environment)
 - Go to this virtual environment and activate it: `cd boutiques & source bin/activate`
 - Install boutiques in a target directory: `pip install --target=boutiques boutiques`. This will install boutiques along with all its dependencies in the `boutiques` directory in the `boutiques` virtual env.
 - Move the `boutiques` directory (of the `boutiques` virtual env, that is `boutiques/boutiques`) in the `BoutiquesGUI-Data` directory.
 - You can now delete the boutiques virtual environment. 
 - Create a `bosh.py` file in the `boutiques` directory containing the following code:
```
import sys

if __name__ == '__main__':

  from boutiques.bosh import bosh
  sys.argv[0] = 'bosh'
  sys.exit(bosh())
```
 - When python runs `bosh.py` (for example `python3 path/to/bosh.py`), all libraries which lie beside `bosh.py` (that is all boutiques dependencies) are imported.

### Directory structure

The required data and executables lie in the `cpp/BoutiquesGUI-Data`:

 - `boutiques/`: contains boutiques for python3, and the command line tool bosh,
 - `python/`: contains the python executable for windows (python2.7 is installed by default on Ubuntu and OS X)
 - `boutiques-gui-settings.json`: the boutiques gui settings,
 - `vc_redist.x86.exe`: the Visual C++ Redistributable for Visual Studio 2015, 2017 and 2019 installer ; required to run python.exe (python3) on windows. 

### Running BoutiquesGUI for the first time

On Windows, BoutiquesGUI will try to execute python3 (`BoutiquesGUI-Data/python/python.exe`) ; if it fails it will try to silently install Visual C++ Redistributable for Visual Studio 2015, 2017 and 2019 (with `BoutiquesGUI-Data/vc_redist.x86.exe`). After the installation, BoutiquesGUI will retry to run python3 ; a message will ask the user to install python3 (in `BoutiquesGUI-Data/python/python.exe`) it fails again.

On Linux, BoutiquesGUI checks that the `python3` command works ; if not, a message asks the user to install it manually.

Then, on all platforms, BoutiquesGUI checks that Docker is installed (by checking the return result of the `docker` command). If not, a message asks the user to install it manually. 

If everything works properly (both `python3` and `docker` can be executed), BoutiquesGUI writes the property `installed: true` to the settings (`cpp/BoutiquesGUI-Data/boutiques-gui-settings.json`) ; the install checks will be ignored for the next times.

BoutiquesGUI will run `boutiques/bosh.py` (the boutiques command line tool) with `python3`, which will in turn run `boutiques`.

### Database

BoutiquesGUI downloads the entire tool database form zenodo (search the 1000 first boutiques tools on Zenodo) at each startup.

The database is stored and updated in the file "~ /.cache/boutiques/all-descriptors.json".
This is useful to keep BoutiquesGUI working when the zenodo database is down (we can then rely on the data stored in "all-descriptors.json"), and to get instantaneous search result.
This process is executed on the background, and updates the database once all descriptors are pulled from Zenodo.

### Settings

The BoutiquesGUI settings are managed automatically, although it could also be set manually.

 - `dataTypeToFormatAndExtension`: describes which format to associate with each data type. When converting an input data to a temporary file (BoutiquesGUI saves medInria input data / images to temporary files to feed them to boutiques) the file format will be given by this settings, depending on the data type. When the user sets a parameter as input (by drag-and-dropping data, or with the corresponding `Set input` button), BoutiquesGUI opens a dialog to choose a format from a list of formats compatible with the data type (only if no compatible format is found in `dataTypeToFormatAndExtension` or `preferredFormatsAndExtensions`). This setting is updated when the user chooses a format to associate with the data type. 

 - `preferredFormatsAndExtensions`: also used to determine the format of temporary files, but instead of associating each data type to a format, it just enumerates the preferred formats and extensions. If the data type is not found in the `dataTypeToFormatAndExtension` map, BoutiquesGUI will use the first compatible format in the `preferredFormatsAndExtensions` list.

 - `outputExtensions`: define the extensions of the boutiques tools outputs which can then be opened in medInria. If an output parameter has an extension listed in `outputExtensions`, a button will enable to set the output as *current output* ; it will automatically be opened in medInria once the process is finished.

  - `installed`: a simple boolean which is set to true when BoutiquesGUI successfully run `python` and `docker` commands for the first time. The install check will not be executed if `installed` is true.

```
    "dataTypeToFormatAndExtension": {
    },
    "installed": true,
    "outputExtensions": [
        ".nii.gz",
        ".vtk"
    ],
    "preferredFormatsAndExtensions": [
        [
            "itkNiftiDataImageWriter",
            ".nii.gz"
        ]
    ]
```

### Using BoutiquesGUI in medInria (medBoutiques plugin)

#### Input

Input data can be drag-and-dropped on the input parameters. The medBoutiques plugin will write the input data to a temporary file (the format is defined by the `dataTypeToFormatAndExtension` or `preferredFormatsAndExtensions` settings ; if nothing corresponds to the data type in those settings, a dialog enables the user to choose a format compatible with the data type, which can optionally be saved in the settings) which will be used by the boutiques tool.
Another way to set the current input data displayed in medInria (in the *input panel*) to a parameter of a boutiques tool is to click the correponding `Set input` button.

#### Output

The `Set output` button following the boutiques tools parameters is used to automatically open the file in medInria *output panel* once the boutiques tool execution is finished. 

### Convert to medInria plugin

The script `cpp/convertToMedInriaPlugin.py` converts the BoutiqueGUI standalone code to a medInria plugin.
It duplicates and rename the BoutiquesGUI .h and .cpp files, and rename classes to follow medInria naming standards.
The output files go to the `medInriaPlugin/` directory.
Those files can then be copied to `medInria-sources/src/plugins/legacy/medBoutiques/boutiquesGUI/` and the medInria plugin should compile.
The directory `cpp/BoutiquesGUI-Data/` must be copied to `medInria-build/bin/` (it must lie beside the `medInria` executable)

## Python version (in python/)

*Note: the python version is not up-to-date and will not be maintained anymore.*

### Install

 - Clone this repo: `git clone https://github.com/fli-iam/medinria-boutiques.git`
 - Initialize a [python3 virutal environment](https://docs.python.org/3/library/venv.html) in the *medinria-boutiques* python directory: `cd medinria-boutiques/python/ && python3 -m venv .`
 - Activate the virutal environment: `source bin/activate`
 - Install PySide2: `pip install PySide2`

### Usage

With the virutal environment activated, run `python3 main.py` from the `python/app/ directory`.
