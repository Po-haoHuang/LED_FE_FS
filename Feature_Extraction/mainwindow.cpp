#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialog.h"
#include "qprocess.h"
#include <iostream>
#include "qfiledialog.h"

using namespace std;

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

void MainWindow::on_pushButton_clicked()
{

    this->open_dialog();

}

void MainWindow::open_dialog(){
    Dialog d;
    string temp,temp1;
    string exe_command = "FE_no_GUI.exe ";
    exe_command += ui->label_5->text().toLocal8Bit().constData();
    exe_command += " ";
    exe_command += ui->label_7->text().toLocal8Bit().constData();
    exe_command += "/ ";
    temp = ui->lineEdit_2->text().toLocal8Bit().constData();
    temp1 = strtok((char*)temp.c_str(),"-");
    temp1 += " ";
    temp1 += strtok(NULL,"-");
    exe_command += temp1;
    exe_command += " ";
    if(atoi(ui->lineEdit->text().toLocal8Bit().constData())<=10)
        exe_command += ui->lineEdit->text().toLocal8Bit().constData();
    else
        exe_command += "10";


    if(system(exe_command.c_str())==0){
        d.changeLabel(QApplication::translate("Dialog", "Finish", 0));

    }
    else
        d.changeLabel(QApplication::translate("Dialog", "Fail", 0));
    d.setWindowFlags(Qt::WindowStaysOnTopHint);
    d.exec();



}

void MainWindow::on_pushButton_2_clicked()
{

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                             ""
                                                             ,tr("CSV (*.csv)"));
    ui->label_5->setText(fileName);

}

void MainWindow::on_pushButton_3_clicked()
{

    QString path = QFileDialog::getExistingDirectory (this, tr("Directory"), "");
    path.toLocal8Bit().constData();
    ui->label_7->setText(path);

}
