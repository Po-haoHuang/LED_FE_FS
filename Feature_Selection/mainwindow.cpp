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

void MainWindow::on_FS_ChooseFile_clicked()
{
    //when clicked get input file dir from file dialog
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                             ""
                                                             ,tr("CSV (*.csv)"));
    ui->FS_InputFileLabel->setText(fileName);
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
    ui->FS_FeaturnameBox->clear();
    while(1){
        getline(temp,tempName, ',');
        if(tempName.c_str()==NULL||tempName=="")
            break;
        ui->FS_FeaturnameBox->addItem(QApplication::translate("FS_FeaturnameBox",tempName.c_str(), 0));
    }
    fb->close();

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
    exe_command += ui->FS_InputFileLabel->text().toLocal8Bit().constData();
    exe_command += "\" \"";
    exe_command += ui->FS_FeaturnameBox->currentText().toLocal8Bit().constData();
    exe_command += "\" ";

    if(ui->FS_DMethodBox->currentText().toLocal8Bit() == "Equal Width(Cycle)"){
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
    exe_command += ui->comboBox->currentText().toLocal8Bit().constData();
    exe_command += " ";
    exe_command += ui->comboBox_2->currentText().toLocal8Bit().constData();
    exe_command += " ";
    exe_command += ui->comboBox_3->currentText().toLocal8Bit().constData();
    exe_command += " ";
    exe_command += ui->comboBox_4->currentText().toLocal8Bit().constData();
    exe_command += " ";
    exe_command += ui->comboBox_6->currentText().toLocal8Bit().constData();
    exe_command += " ";
    exe_command += ui->comboBox_7->currentText().toLocal8Bit().constData();
    exe_command += " ";
    exe_command += ui->lineEdit_10->text().toLocal8Bit().constData();
    exe_command += " FSi_exclude_list FSi_use_feature_list";
    cout<<exe_command;






    if(system(exe_command.c_str())==0){
        d.changeLabel(QApplication::translate("Dialog", "Finish", 0));

    }
    else
        d.changeLabel(QApplication::translate("Dialog", "Fail", 0));
    d.setWindowFlags(Qt::WindowStaysOnTopHint);
    d.exec();


}

void MainWindow::on_FS_DMethodBox_activated(const QString &arg1)
{
    if (arg1 == "Manual"){
        ui->label_12->setText(QApplication::translate("FS_DMethodBox","Cut points(ex:1,5,10):", 0));
    }
    else {
        ui->label_12->setText(QApplication::translate("FS_DMethodBox","EW_Cycle Number:", 0));
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    //when clicked get input file dir from file dialog
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                             ""
                                                             ,tr("CSV (*.csv)"));
    ui->label_5->setText(fileName);

}

void MainWindow::on_pushButton_3_clicked()
{
    //when clicked get input file dir from file dialog
    QString path = QFileDialog::getExistingDirectory (this, tr("Directory"), "");
    path.toLocal8Bit().constData();
    ui->label_15->setText(path);

}


void MainWindow::open_dialog_1(){
    //when click start ,run no_gui app and show success/fail
    Dialog d;
    string temp,temp1;
    string exe_command = "FE_no_GUI.exe \"";
    exe_command += ui->label_5->text().toLocal8Bit().constData();
    exe_command += "\" \"";
    exe_command += ui->label_15->text().toLocal8Bit().constData();
    exe_command += "/\" \"";
    temp = ui->lineEdit_8->text().toLocal8Bit().constData();
    if(temp!=""){
    temp1 = strtok((char*)temp.c_str(),"-");
    temp1 += "\" \"";
    temp1 += strtok(NULL,"-");
    exe_command += temp1;
    }

    exe_command += "\" ";


    exe_command += ui->comboBox_5->currentText().toLocal8Bit().constData();


    if(system(exe_command.c_str())==0){
        d.changeLabel(QApplication::translate("Dialog", "Finish", 0));

    }
    else
        d.changeLabel(QApplication::translate("Dialog", "Fail", 0));
    d.setWindowFlags(Qt::WindowStaysOnTopHint);
    d.exec();
}

void MainWindow::on_pushButton_4_clicked()
{
    this->open_dialog_1();
}
