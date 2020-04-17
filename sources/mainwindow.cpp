#include "../headers/mainwindow.h"
#include "qdir.h"
#include "qdialog.h"
#include "qfiledialog.h"
#include "ui_mainwindow.h"
#include "qmessagebox.h"
#include "qfontdialog.h"
#include "../headers/simulator.h"

extern "C"

{

 #include "../headers/compiler.h"

}

#include <string>
#include <cstring>

char compileBuffer[65536*16];

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    highlighter = new Highlighter(ui->codeEdit->document());
    highlighter = new Highlighter(ui->memoryText->document());
    highlighter = new Highlighter(ui->registerText->document());
    //ui->linenoText->setFont(ui->codeEdit->font());
    sim = new simulator(MAXMEMSIZE);
    initSim();
    ui->tabWidget->setCurrentIndex(0);
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
    QString pcstr = ui->PCtext->text();
    code i = pcstr.toUInt();
    sim->setPC(i);
}

void MainWindow::on_setBPButton_clicked()
{
    QString bpstr = ui->breakpointText->text();
    code i = bpstr.toUInt();
    sim->setBreakpoint(i);
}

void MainWindow::on_actionabout_triggered()
{
    QMessageBox::about(this, tr("About the MIPS compiler"),
                tr("<p> <b>MIPS compiler</b></p> " \
                   "<p>made by yunwei </p>" \
                   "<p>version 0.1</p>" \
                   ""));
}

void MainWindow::on_actionopen_triggered()
{
    QString curPath=QDir::currentPath();//获取系统当前目录
    //获取应用程序的路径
    QString dlgTitle="选择一个文件"; //对话框标题
    QString filter="source(*.asm);;all(*.*)"; //文件过滤器
    QString aFileName=QFileDialog::getOpenFileName(this,dlgTitle,curPath,filter);
    if (aFileName.isEmpty())
        return;
    QFile   aFile(aFileName);
    if (!aFile.exists())
        return;
    if (!aFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    if (!aFileName.isEmpty())
        ui->codeEdit->setPlainText(aFile.readAll());
    aFile.close();
    ui->compileOutput->setText("open file successfully");
    return;
}

void MainWindow::on_compileButton_clicked()
{
    ui->tabWidget->setCurrentIndex(0);
    QString str = ui->codeEdit->toPlainText();
    std::string s = str.toStdString();
    strcpy(compileBuffer,s.c_str());
    compileText(compileBuffer);
    QString output = compileOutput;
    ui->compileOutput->setText(output);
}

void MainWindow::on_actionsave_triggered()
{
    QString curPath=QDir::currentPath();//获取系统当前目录
    QString dlgTitle="save"; //对话框标题
    QString filter="source(*.asm);;text(*.txt);;all(*.*)";
    QString aFileName=QFileDialog::getSaveFileName(this,dlgTitle,curPath,filter);
    if (aFileName.isEmpty())
        return;
     QFile aFile(aFileName);
     if (!aFile.open(QIODevice::WriteOnly | QIODevice::Text))
         return;
      QString str = ui->codeEdit->toPlainText();
      QByteArray  strBytes=str.toUtf8();
      aFile.write(strBytes,strBytes.length());
      aFile.close();
      ui->compileOutput->setText("save file successfully");
      return;
}

void MainWindow::on_simulateButton_clicked()
{
    on_compileButton_clicked();
    ui->tabWidget->setCurrentIndex(1);
    initSim();
}

void MainWindow::on_actionnew_triggered()
{
    ui->codeEdit->clear();
}

void MainWindow::on_actionsetFont_triggered()
{
    QFont iniFont=ui->codeEdit->font();
    bool   ok=false;
    QFont font=QFontDialog::getFont(&ok,iniFont);
    if (ok)
        ui->codeEdit->setFont(font);
    //ui->linenoText->setFont(ui->codeEdit->font());
}

void MainWindow::on_codeEdit_textChanged()
{

}

void MainWindow::on_viewMemoryButton_clicked()
{
    QString maddr = ui->memoryAddEdit->text();
    code addr = maddr.toUInt();
    QTextCursor tc = ui->memoryText->textCursor();
    tc.setPosition(0);
    int h = ui->memoryText->height();
    //char buffer[20];
    //itoa(h,buffer,10);
   // ui->outputText->setText(buffer);
    ui->memoryText->scroll(0,addr/sim->getSize()*h);
}

void MainWindow::on_setMemoryButton_clicked()
{
    QString maddr = ui->memoryAddEdit->text();
    code addr = maddr.toUInt();
    QString mdata = ui->memdataEdit->text();
    code data = mdata.toUInt();
    sim->setMemory(addr,data);
    refreshMemline(addr);
}

void MainWindow::refreshMemline(int addr)
{
    QTextCursor txtcur = ui->memoryText->textCursor();
    txtcur.setPosition(0);
    txtcur.movePosition(QTextCursor::Down,QTextCursor::MoveAnchor,addr);
    txtcur.movePosition(QTextCursor::EndOfLine,QTextCursor::KeepAnchor);
    txtcur.movePosition(QTextCursor::Down,QTextCursor::KeepAnchor);
    txtcur.movePosition(QTextCursor::StartOfLine,QTextCursor::KeepAnchor);
    QString qstr= txtcur.selectedText();
    //ui->outputText->setText(qstr);
    txtcur.removeSelectedText();

    QString mtext = "";
    mtext += sim->memoryText[addr].address;
    mtext += sim->memoryText[addr].hextext;
    mtext += sim->memoryText[addr].asciitext;
    txtcur.insertText(mtext);
}

void MainWindow::freshallMem()
{
    ui->memoryText->clear();
    QString mtext = "";
    for(int i=0;i<sim->getSize();++i){
        //mtext.clear();
        mtext += sim->memoryText[i].address;
        mtext += sim->memoryText[i].hextext;
        mtext += sim->memoryText[i].asciitext;
    }
    ui->memoryText->setText(mtext);
}

void MainWindow::initSim()
{
    sim->loadbinary();
    freshDisplay();
    freshallMem();
}

void MainWindow::freshDisplay()
{
    char buffer[20];
    sprintf(buffer,"0x%08x",sim->getPC());
    ui->PCtext->setText(buffer);
    if(ui->enableBP->checkState() == Qt::Checked && sim->getBreakpoint() != ERRORCODE){
            sprintf(buffer,"0x%08x",sim->getBreakpoint());
            ui->breakpointText->setText(buffer);
    }
    QString ir = "IR content: ";
    sprintf(buffer,"0x%08x",sim->getIR());
    ui->labelIR->setText(ir+buffer);


    ui->registerText->clear();
    QString rtext = "";
    for(int i=0;i<32;++i){
        rtext += regMapDict[i];
        sprintf(buffer,"0x%08x",sim->getReg(i));
        rtext += ": ";
        rtext += buffer;
        rtext += "  ";
        if(i%3 == 2){
            rtext += "\n";
        }
    }
    ui->registerText->setText(rtext);
}

// run
void MainWindow::on_pushButton_clicked()
{
    sim->run();
    freshDisplay();
    freshallMem();
}

// step
void MainWindow::on_pushButton_2_clicked()
{
    sim->step();
    freshDisplay();
    //freshallMem();
    refreshMemline(sim->getchangedMemAddr());
}

// reset
void MainWindow::on_pushButton_3_clicked()
{
    sim->reset();
    freshDisplay();
}

void MainWindow::on_actionexit_triggered()
{
    exit(0);
}

void MainWindow::on_actionexport_triggered()
{
    on_compileButton_clicked();
    sim->loadbinary();
    if(compiledflag == 0) return;
    QString curPath=QDir::currentPath();//获取系统当前目录
    QString dlgTitle="export"; //对话框标题
    QString filter="binary(*.bin);;all(*.*)";
    QString aFileName = QFileDialog::getSaveFileName(this,dlgTitle,curPath,filter);
    if (aFileName.isEmpty())
        return;
    string fname = aFileName.toStdString();
    sim->exportMemory(fname);
}

void MainWindow::on_actionimport_binary_triggered()
{
    on_simulateButton_clicked();
    QString curPath=QDir::currentPath();//获取系统当前目录
    QString dlgTitle="import"; //对话框标题
    QString filter="binary(*.bin);;all(*.*)";
    QString aFileName=QFileDialog::getOpenFileName(this,dlgTitle,curPath,filter);
    if (aFileName.isEmpty())
        return;
    string fname = aFileName.toStdString();
    sim->loadMemory(fname);
    freshDisplay();
    freshallMem();
}
