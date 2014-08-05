#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qfiledialog.h"
#include "dialog.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "qprocess.h"

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

void MainWindow::on_pushButton_2_clicked()
{
    //when clicked get input file dir from file dialog
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                             ""
                                                             ,tr("CSV (*.csv)"));
    ui->label_5->setText(fileName);
    //read from file to add variable option in combo box to be selected
    string tempAttr,tempName;
    ifstream is;
    filebuf *fb = is.rdbuf();
    fb->open (fileName.toLocal8Bit().constData(),ios::in);
    getline(is,tempAttr);
    stringstream temp(tempAttr);
    getline(temp,tempName, ',');
    getline(temp,tempName, ',');
    getline(temp,tempName, ',');
    for(int i = 0;i < 84; i++){
        getline(temp,tempName, ',');
        if(tempName.c_str()==NULL||tempName=="")
            break;
        ui->comboBox->addItem(QApplication::translate("comboBox",tempName.c_str(), 0));
    }

}

void MainWindow::on_pushButton_clicked()
{

    this->open_dialog();

}

void MainWindow::open_dialog(){
    //when click start ,run no_gui app and show success/fail
    Dialog d;
    string temp,temp1;
    string exe_command = "FS_no_gui.exe \"";
    exe_command += ui->label_5->text().toLocal8Bit().constData();
    exe_command += "\" \"";
    exe_command += ui->comboBox->currentText().toLocal8Bit().constData();
    exe_command += "\" ";
    exe_command += ui->lineEdit->text().toLocal8Bit().constData();
    exe_command += " ";
    if(ui->comboBox_2->currentText().toLocal8Bit() == "Equal Width(Cycle)"){
        exe_command += "\"ew_cycle=";
        exe_command += ui->lineEdit_5->text().toLocal8Bit().constData();
        exe_command += "\" ";
    }
    else{
        exe_command += ui->lineEdit_5->text().toLocal8Bit().constData();
        exe_command += " ";
    }
    exe_command += ui->lineEdit_7->text().toLocal8Bit().constData();
    exe_command += " ";
    exe_command += ui->lineEdit_6->text().toLocal8Bit().constData();
    exe_command += " ";
    exe_command += ui->lineEdit_2->text().toLocal8Bit().constData();
    exe_command += " ";
    exe_command += ui->lineEdit_3->text().toLocal8Bit().constData();
    exe_command += " ";
    exe_command += ui->lineEdit_4->text().toLocal8Bit().constData();
    if(system(exe_command.c_str())==0){
        d.changeLabel(QApplication::translate("Dialog", "Finish", 0));

    }
    else
        d.changeLabel(QApplication::translate("Dialog", "Fail", 0));
    d.setWindowFlags(Qt::WindowStaysOnTopHint);
    d.exec();


}

void MainWindow::on_comboBox_2_activated(const QString &arg1)
{
    if (arg1 == "Manual"){
        ui->label_12->setText(QApplication::translate("comboBox_2","Cut points(ex:1,5,10):", 0));
    }
    else {
        ui->label_12->setText(QApplication::translate("comboBox_2","ew_cycle number:", 0));
    }
}
