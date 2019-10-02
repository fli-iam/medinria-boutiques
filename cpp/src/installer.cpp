#include "installer.h"
#include <iostream>
#include <QtWidgets>

void Installer::checkBoutiquesInstallation(QWidget *parent)
{
    // Check the "installed" property in the settings
    QFile file(BoutiquesPaths::Settings());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // If can't open the settings: install boutiques
        file.close();
        Installer::installBoutiques(parent);
    } else {
        QJsonDocument jsonDocument(QJsonDocument::fromJson(file.readAll()));
        QJsonObject settings = jsonDocument.object();

        // If "installed" is null or false: install boutiques
        if(!settings.contains("installed") || !settings["installed"].toBool())
        {
            file.close();
            Installer::installBoutiques(parent, &settings);
        }
    }
}

void Installer::setBoutiquesInstalled(QJsonObject *settings)
{
    // Write the "installed" property in settings
    QFile file(BoutiquesPaths::Settings());
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        if(settings != nullptr)
        {
            (*settings)["installed"] = true;
            QJsonDocument settingsDocument(*settings);
            file.write(settingsDocument.toJson());
        }
        else
        {
            QJsonObject settings;
            settings["installed"] = true;
            QJsonDocument settingsDocument(settings);
            file.write(settingsDocument.toJson());
        }
    }
}

bool Installer::isPythonWorking(QWidget *parent, const QString &version)
{
    // Check that the python command is working ("python" on Linux and OS X, and "BoutiquesGUI-Data/python/python.exe" on Windows)
    QProcess pythonProcess(parent);
    pythonProcess.start(BoutiquesPaths::Python(), {"--version"});
    pythonProcess.waitForFinished();
    std::string output = pythonProcess.readAllStandardOutput().toStdString();
    std::string error = pythonProcess.readAllStandardError().toStdString();
    std::string pythonVersion = "Python " + version.toStdString();
    return output.find(pythonVersion) != std::string::npos || error.find(pythonVersion) != std::string::npos;
}

bool Installer::isDockerWorking(QWidget *parent)
{
    // Check that the docker command is working
    QProcess dockerProcess(parent);
    dockerProcess.start(BoutiquesPaths::Docker(), {"--version"});
    dockerProcess.waitForFinished();
    std::string output = dockerProcess.readAllStandardOutput().toStdString();
    std::string error = dockerProcess.readAllStandardError().toStdString();
    std::string dockerVersion = "Docker version";
    return output.find(dockerVersion) != std::string::npos || error.find(dockerVersion) != std::string::npos;
}

void Installer::installBoutiques(QWidget *parent, QJsonObject *settings)
{
    bool pythonAndDockerAreWorking = true;

#ifdef Q_OS_WIN
    // On Windows:

    if(!Installer::isPythonWorking(parent, "3"))
    {
        // If "BoutiquesGUI-Data/python/python.exe" does not work:
        // Install visual studio redistributable (required for python3)

        QProcess installVisualStudioRedistributableProcess(parent);
        installVisualStudioRedistributableProcess.start(BoutiquesPaths::VCRedis(), {"\\q"});

        if (!installVisualStudioRedistributableProcess.waitForFinished())
        {
            // If the install fails: ask the user to install it manullay
            pythonAndDockerAreWorking = false;
            QMessageBox::critical(parent, "Could not install Microsoft Visual C++ Redistributable for Visual Studio", "Error while installing Microsoft Visual C++ Redistributable for Visual Studio.\nThis software is required to run python3 and boutiques under windows.\n\nTry to install it manually.");
        }
        else if(!Installer::isPythonWorking(parent, "3"))
        {
            // If the install succeeds but python still does not work: ask the user to install python
            pythonAndDockerAreWorking = false;
            QMessageBox::critical(parent, "Python is not working", "Python.exe (" + BoutiquesPaths::Python() + ") is not working.\n\nYou need a working python3 version at this location to run boutiques tools.");
        }
    }
#else
    // On Linux:

    if(!Installer::isPythonWorking(parent))
    {
        // If python does not work: ask the user to install it
        pythonAndDockerAreWorking = false;
        QMessageBox::critical(parent, "Could not run Python", "Error while testing Python.\nInstall python 2.7 or 3 to run boutiques tools.");
    }

#endif

    if (!Installer::isDockerWorking(parent))
    {
        // If docker does not work: ask the user to install it
        pythonAndDockerAreWorking = false;
        QMessageBox::critical(parent, "Could not run Docker", "Error while testing Docker.\nInstall Docker or Singularity to run boutiques tools.");
    }

    if(pythonAndDockerAreWorking)
    {
        // If python and docker work: set the "installed" setting to "true"
        Installer::setBoutiquesInstalled(settings);
    }
}
