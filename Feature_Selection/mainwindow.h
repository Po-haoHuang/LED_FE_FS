#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
        void on_pushButton_clicked();
        void on_FS_ChooseFile_clicked();
        void on_pushButton_2_clicked();
        void on_pushButton_3_clicked();


        void on_FS_DMethodBox_activated(const QString &arg1);

        void on_pushButton_4_clicked();

private:
    Ui::MainWindow *ui;
    void open_dialog();
    void open_dialog_1();
};

#endif // MAINWINDOW_H
