#ifndef SEARCHTOOLSWIDGET_H
#define SEARCHTOOLSWIDGET_H

#include <vector>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLineEdit;
class QGroupBox;
class QLabel;
class QTableWidget;
class QTextEdit;
class QProcess;
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
public:
    explicit SearchToolsWidget(QWidget *parent = nullptr);

    SearchResult *getSelectedTool();

private:
    void createTable();
    void createProcess();

signals:
    void toolSelected();
    void toolDeselected();

public slots:
    void selectionChanged();
    void searchBoutiquesTools();
    void processFinished(int exitCode);

private:
    QLineEdit* searchLineEdit;
    QPushButton* button;
    QGroupBox* searchGroupBox;
    QLabel* loadingLabel;
    QTableWidget* table;
    QLabel* infoLabel;
    QTextEdit* info;
    QProcess* process;
    std::vector<SearchResult> searchResults;
};

#endif // SEARCHTOOLSWIDGET_H
