#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QToolBar>
#include "chessclient.h"
#include "config.h"

namespace Ui {
class MainWindow;
}
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void createMenus();
    void createToolBars();
    void creatDockWidgets();
    void createStatusBar();

public slots:
    void showStatus(const char *status, int timeout);
    void showSteps(const QString &item, int side);
    void clearSteps(const QString & type);

private slots:
    void about();
    void version();
    void quit();

private:

    Ui::MainWindow *ui;

    ChessClient *chessclient;

   /***/
    QMenu *ModuleMenu;
    QMenu *HelpMenu;
    QToolBar *toolBar;
    QListWidget *redList;
    QListWidget *blackList;

};

#endif // MAINWINDOW_H
