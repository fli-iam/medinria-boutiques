#ifndef SEARCHTOOLSWIDGET_H
#define SEARCHTOOLSWIDGET_H

#include <vector>
#include <QWidget>
#include <QProcess>
#include <QJsonObject>
#include <QJsonArray>

#define BOSH_PATH "../bosh"

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLineEdit;
class QGroupBox;
class QLabel;
class QTableWidget;
class QTextEdit;
QT_END_NAMESPACE

using namespace std;

struct SearchResult {
    string id;
    string title;
    string description;
    int downloads;
};

class SearchToolsWidget : public QWidget
{
    Q_OBJECT

private:
    QLineEdit* searchLineEdit;
    QPushButton* button;
    QGroupBox* searchGroupBox;
    QLabel* loadingLabel;
    QTableWidget* table;
    QLabel* infoLabel;
    QTextEdit* info;
    QProcess* searchProcess;
    QProcess* pprintProcess;
    QProcess* pullProcess;
    std::vector<SearchResult> searchResults;
    vector<pair<QString, int>> zenodoIdsAndDownloads;
    bool mustCreateToolDatabase;
    QJsonArray descriptors;

public:
    explicit SearchToolsWidget(QWidget *parent = nullptr);

    SearchResult *getSelectedTool();

private:
    void createTable();
    void createProcesses();
    void downloadTools();
    void createToolDatabase();
    void loadToolDatabase();

signals:
    void toolSelected();
    void toolDeselected();

public slots:
    void selectionChanged();
    void searchBoutiquesTools();
    void errorOccurred(QProcess::ProcessError error);
    void searchProcessStarted();
    void searchProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void pprintProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void pullProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

};

#endif // SEARCHTOOLSWIDGET_H
