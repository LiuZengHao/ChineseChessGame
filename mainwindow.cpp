#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <vector>
#include <QDockWidget>
#include <QMessageBox>
#include <QMenuBar>
#include <QStatusBar>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QTextCodec>

static const QString REDLIST_FILE_NAME = "readlist.txt";
static const QString BLACKLIST_FILE_NAME = "blacklist.txt";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    chessclient = new ChessClient(this);
    resize(1000, 700);
    setWindowTitle(tr("中国象棋"));
    setWindowIcon(QIcon(":/images/icon/chineseChess.ico"));
    createMenus();
    createToolBars();
    creatDockWidgets();
    createStatusBar();

    connect(chessclient, SIGNAL(statusChanged(const char*, int)), this, SLOT(showStatus(const char*, int)));
    connect(chessclient, SIGNAL(stepsChanged(const QString &, int)), this, SLOT(showSteps(const QString &,int)));
    connect(chessclient, SIGNAL(clearSteps(const QString &)), this, SLOT(clearSteps(const QString &)));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete chessclient;
}

void MainWindow::createMenus()
{

    ModuleMenu = menuBar()->addMenu(tr("&模式选择"));

    ModuleMenu->addAction(tr("&双人对战"), chessclient, SLOT(man2man()));

    ModuleMenu->addAction(tr("&人机大战"), chessclient, SLOT(man2machine()));

    HelpMenu = menuBar()->addMenu(tr("&帮助"));

    HelpMenu->addAction(tr("&关于"), this, SLOT(about()));

    HelpMenu->addAction(tr("&版本与版权"),this, SLOT(version()));

}

void MainWindow::creatDockWidgets()
{
    QDockWidget *dockWidget= new QDockWidget(tr("红方"), this);
    dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    redList = new QListWidget(dockWidget);
    redList->setMaximumWidth(100);

    dockWidget->setWidget(redList);
    addDockWidget(Qt::TopDockWidgetArea, dockWidget);


    dockWidget = new QDockWidget(tr("黑方"), this);
    dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    blackList = new QListWidget(dockWidget);
    blackList->setMaximumWidth(100);

    dockWidget->setWidget(blackList);
    addDockWidget(Qt::RightDockWidgetArea, dockWidget);


    dockWidget->show();
}

void MainWindow::showSteps(const QString &item, int side){
    if(side == RED){
        blackList->addItems(QStringList() << item);
    }
    else{
         redList->addItems(QStringList() << item);
    }
    repaint();
}
void MainWindow::clearSteps(const QString & type){

    if(type == QString::number(RED) && blackList->count()){

            blackList->takeItem(blackList->count()-1);
    }
    else if(type == QString::number(BLACK) && redList->count()){
            redList->takeItem(redList->count()-1);
    }

    else if(type == tr("保存")){
        int count1 = redList->count();
        int count2 = blackList->count();
        QFile file1(REDLIST_FILE_NAME);
        QFile file2(BLACKLIST_FILE_NAME);
        file1.open(QIODevice::ReadWrite);
        file2.open(QIODevice::ReadWrite);
        file1.resize(0);
        file2.resize(0);
        QTextStream out1(&file1);
        QTextStream out2(&file2);
        out1.setCodec("UTF-8");
        out2.setCodec("UTF-8");
        for(int i=0; i<count1; i++){
            out1.operator <<((redList->item(i))->text().toUtf8());
            out1.operator <<(tr("\r\n").toUtf8());
        }
        for(int j=0; j<count2; j++){
            out2.operator <<(blackList->item(j)->text().toUtf8());
            out2.operator << (tr("\r\n").toUtf8());
        }
        out1.flush();
        out2.flush();
        file1.close();
        file2.close();
    }
    else if(type == tr("载入")){
        blackList->clear();
        redList->clear();
           QFile file1(REDLIST_FILE_NAME);
           QFile file2(BLACKLIST_FILE_NAME);
           file1.open(QIODevice::ReadWrite );
           file2.open(QIODevice::ReadWrite );
           QTextStream out1(&file1);
           QTextStream out2(&file2);
           out1.setCodec("UTF-8");
           out2.setCodec("UTF-8");
           QString str1 = out1.readLine(10);
           QString str2 = out2.readLine(10);

           while(str1 != tr("")){
               redList->addItems(QStringList() << str1);
               str1 = out1.readLine(10);
           }
           while(str2 != tr("")){
                blackList->addItems(QStringList() << str2);
                str2 = out2.readLine(10);
           }
           file1.close();
           file2.close();


    }
    else{
        //求和，认输
        blackList->clear();
        redList->clear();

    }
 }
void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("就绪"));
}

void MainWindow::createToolBars()
{
    toolBar = addToolBar(tr("Tool"));

    toolBar->addAction(tr("开始"), chessclient, SLOT(start()));
    toolBar->addSeparator();

    toolBar->addAction(tr("暂停"), chessclient, SLOT(stop()));
    toolBar->addSeparator();

    toolBar->addAction("悔棋", chessclient, SLOT(undo()));
    toolBar->addSeparator();

    toolBar->addAction("求和", chessclient, SLOT(peace()));
    toolBar->addSeparator();

    toolBar->addAction("认输", chessclient, SLOT(submit()));
    toolBar->addSeparator();

    toolBar->addAction("保存", chessclient, SLOT(save()));
    toolBar->addSeparator();

    toolBar->addAction("载入", chessclient, SLOT(load()));
    toolBar->addSeparator();

    toolBar->addAction("退出", this, SLOT(quit()));
}

void MainWindow::about()
{
    QMessageBox::about(NULL, tr("关于中国象棋"), tr("中国象棋使用方形格状棋盘及红黑二色圆形棋子进行对弈，棋盘上有十条横线、九条竖线共分成90个交叉点；象棋的棋子共有32个，每种颜色16个棋子，分为7个兵种，摆放和活动在交叉点上。双方交替行棋，先把对方的将（帅）“将死”的一方获胜（因为擒贼先擒王）。在中国，已有几千年历史、充满东方智慧的象棋在中国的群众基础远远超过围棋，一直是普及最广的棋类项目。"));
}

void MainWindow::version()
{
    QMessageBox::about(NULL,tr("关于版本与版权"),  tr("版本:2.0。由A，B，C,D人共同开发。本软件遵循GPL开源协议，允许代码的开源/免费使用和引用/修改/衍生代码的开源/免 费使用，但不允许修改后和衍生的代码做为闭源的商业软件发布和销售。"));
}


void MainWindow::quit(){
    QMessageBox::StandardButton rb = QMessageBox::information(NULL, "退出", "确定退出?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if(rb == QMessageBox::Yes){
        this->close();
    }
}

void MainWindow::showStatus(const char *status, int timeout)
{
    statusBar()->showMessage(status, timeout);
}
