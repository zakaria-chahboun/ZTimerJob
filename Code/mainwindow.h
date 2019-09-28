#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMediaPlayer>
#include <QMainWindow>
#include <QCloseEvent>
#include <QProcess>
#include <QTimer>
#include <QTime>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void closeEvent(QCloseEvent *event);
    enum Target{ PlaySound, RunProgram, TurnOff };
    void setTarget(Target t);
    void doTarget();

    ~MainWindow();

private slots:
    void on_Start_clicked();

    void on_brows_clicked();

    void on_volume_valueChanged(int value);

    void on_testSound_toggled(bool checked);

    void AboutMe();

private:
    Ui::MainWindow *ui;

    QTime TIME;
    QTime FINAL_TIME;
    QTimer *timer;
    QTimer *timerTMP;

    int counter = 0;
    int counterTMP = 0;
    int operation = -1; // Operation : Ascending or Descending

    int volume = 50;
    QString lastPath;

    QMediaPlayer *Player;
    Target targetVal = PlaySound; // by defualt!
};

#endif // MAINWINDOW_H
