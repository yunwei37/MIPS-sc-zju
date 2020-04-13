#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_actiondocument_triggered()
{

}

void MainWindow::on_setPCButton_clicked()
{

}

void MainWindow::on_setBPButton_clicked()
{

}
