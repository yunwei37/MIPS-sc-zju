#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "highlighter.h"
#include "simulator.h"
#include "QTextEdit"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:


    void on_actiondocument_triggered();

    void on_setPCButton_clicked();

    void on_setBPButton_clicked();

    void on_actionabout_triggered();

    void on_actionopen_triggered();

    void on_compileButton_clicked();

    void on_actionsave_triggered();

    void on_simulateButton_clicked();

    void on_actionnew_triggered();

    void on_actionsetFont_triggered();

    void on_codeEdit_textChanged();

    void on_viewMemoryButton_clicked();

    void on_setMemoryButton_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_actionexit_triggered();

    void on_actionexport_triggered();

    void on_actionimport_binary_triggered();

private:
    Ui::MainWindow *ui;
    simulator *sim;
    Highlighter *highlighter;
    void initSim();
    void freshDisplay();
    void refreshMemline(int addr);
    void freshallMem();

};
#endif // MAINWINDOW_H
