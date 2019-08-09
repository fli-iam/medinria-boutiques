#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#define BOUTIQUE_GUI_STANDALONE
//#define TEST_DRAGGABLE_INPUTS

#ifdef BOUTIQUE_GUI_STANDALONE
#define BOUTIQUES_DIRECTORY "../"
#else
#define BOUTIQUES_DIRECTORY "BoutiquesGUI/"
#endif

#define BOUTIQUES_GUI_SETTINGS_PATH BOUTIQUES_DIRECTORY "boutiques-gui-settings.json"

#ifndef Q_OS_WINDOWS
#define BOSH_PATH BOUTIQUES_DIRECTORY "bosh"
#else
#define BOSH_PATH BOUTIQUES_DIRECTORY "\"python/python.exe\" python/bosh.py"
#endif

#endif // CONFIGURATION_H
