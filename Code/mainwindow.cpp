#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>

#define deb qDebug()
#define msg(a,b) QMessageBox::information(this,a,b)
#define MyApp "TimerJob - ZAKI - 2018"
#define MyOrg "ZAKI PRO"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Define
    timer = new QTimer(this);
    timerTMP = new QTimer(this); // Just For Decorations
    Player = new QMediaPlayer(this);
    TIME.setHMS(0,0,0);
    ui->lcdNumber->display("00:00:00");

    // Init
    QSettings settings(MyOrg,MyApp);
    this->volume = settings.value("VOLUME",50).toInt();
    this->lastPath = settings.value("LAST_PATH",QDir::rootPath()).toString();
    ui->path->setText(this->lastPath);
    ui->volume->setValue(this->volume);

    // When Timer Stop
    connect(ui->actionStop_Timer,&QAction::triggered,[&](){
        // Stop Timer
        this->timer->stop();
        this->counterTMP = 0;

        timerTMP->start(300);
    });
    // Decoration (^^)
    connect(timerTMP,&QTimer::timeout,[&](){
        if(counterTMP % 2 == 0){
            ui->lcdNumber->display(this->TIME.toString("HH:mm:ss"));
        }else{
            ui->lcdNumber->display("");
        }
        if(counterTMP >= 8){
            timerTMP->stop();
            ui->lcdNumber->display("00:00:00");
        }
        counterTMP++;
    });

    // Choose Style : Black or White
    connect(ui->actionStyle_1,&QAction::triggered,[&](bool checked){
        if(checked){
            ui->actionStyle_2->setChecked(false);
            ui->lcdNumber->setStyleSheet("background:black;color:white;border-radius:20px;");
        }
        else{
            ui->actionStyle_2->setChecked(true);
            ui->lcdNumber->setStyleSheet("background:white;color:black;border-radius:20px;");
        }
    });
    connect(ui->actionStyle_2,&QAction::triggered,[&](bool checked){
        if(checked){
            ui->actionStyle_1->setChecked(false);
            ui->lcdNumber->setStyleSheet("background:white;color:black;border-radius:20px;");
        }
        else{
            ui->actionStyle_1->setChecked(true);
            ui->lcdNumber->setStyleSheet("background:black;color:white;border-radius:20px;");
        }
    });

    // Go to Tools page
    connect(ui->actionTools,&QAction::triggered,[&](){
            ui->stackedWidget->setCurrentWidget(ui->Page_Tools);
    });

    // Player : When Stop :)
    connect(Player,&QMediaPlayer::stateChanged,[&](QMediaPlayer::State state){
        if(state == QMediaPlayer::StoppedState){
            deb << "STOP";
            ui->testSound->setText("Test Sound");
            ui->testSound->setCheckable(false);
            ui->testSound->setCheckable(true);
        }
        if(state == QMediaPlayer::PlayingState){
            deb << "START";
            ui->testSound->setText("STOP");
        }
    });

    /** *********************************************************************** **/
    // Choose The State : "Afer" or "Equal"
    connect(ui->AFTER_BOX,&QGroupBox::clicked,[&](bool checked){
        if(checked)
            ui->EQUAL_BOX->setChecked(false);
        else
            ui->EQUAL_BOX->setChecked(true);
    });
    connect(ui->EQUAL_BOX,&QGroupBox::clicked,[&](bool checked){
        if(checked)
            ui->AFTER_BOX->setChecked(false);
        else
            ui->AFTER_BOX->setChecked(true);
    });

    // Choose Target Types
    connect(ui->Target_PlaySound,&QRadioButton::toggled,[&](){
        ui->path->setEnabled(true);
        ui->brows->setEnabled(true);
        ui->testSound->setEnabled(true);
        ui->volume->setVisible(true);
        ui->volumeLabel->setEnabled(true);
    });
    connect(ui->Target_StartProgram,&QRadioButton::toggled,[&](){
        ui->path->setEnabled(true);
        ui->brows->setEnabled(true);
        ui->volume->setVisible(false);
        ui->testSound->setEnabled(false);
        ui->volumeLabel->setEnabled(false);
    });
    connect(ui->Target_TurnOff,&QRadioButton::toggled,[&](){
        ui->path->setEnabled(false);
        ui->brows->setEnabled(false);
        ui->volume->setVisible(false);
        ui->testSound->setEnabled(false);
        ui->volumeLabel->setEnabled(false);
    });

    // Timer
    connect(timer,&QTimer::timeout,[&](){

        /** Set The Right Operation : Ascending or Descending : By Seconds **/
        this->TIME = TIME.addSecs(operation);
        /** Show Time ^^ **/
        ui->lcdNumber->display(TIME.toString("HH:mm:ss"));
        /** State 1 : When We Get (00:00:00) : Stop **/
        if(operation == -1 && this->TIME == QTime(0,0,0)){
            doTarget();
            ui->actionStop_Timer->trigger();
        }
        /** State 2 : When CurrentTime = TIME : Stop **/
        if(operation == 1 && this->TIME == FINAL_TIME){
            doTarget();
            ui->actionStop_Timer->trigger();
        }
    });

    /** ************  SIMPLE CLOCK ************** */
    auto timer2 = new QTimer(this);
    connect(timer2,&QTimer::timeout,[&](){
        ui->clock->setText("Time now "+QTime::currentTime().toString("HH:mm:ss"));
    });
    timer2->start(1000);

    /// ***************** About *************** ///
    connect(ui->actionCreator,&QAction::triggered,this,&MainWindow::AboutMe);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Start: btn
void MainWindow::on_Start_clicked()
{
    // Move to Timer Page
    ui->stackedWidget->setCurrentWidget(ui->Page_Timer);

    // AFTER TIME
    if(ui->AFTER_BOX->isChecked()){
        this->operation = -1; /** Minus 1 By Seconds **/
        auto time = ui->AFTER_TIMER->time(); /** Get Time **/
        TIME.setHMS(time.hour(),time.minute(),time.second());
    }
    // EQUAL TIME
    else if(ui->EQUAL_BOX->isChecked()){
        this->operation = 1; /** Add 1 By Seconds **/
        FINAL_TIME = ui->EQUAL_TIMER->time(); /** Get Time **/
        auto tmp = QTime::currentTime();
        TIME.setHMS(tmp.hour(),tmp.minute(),tmp.second()); /** Save Current Time **/
    }

    // Set right target
    if(ui->Target_PlaySound->isChecked())
        this->setTarget(PlaySound);

    else if(ui->Target_StartProgram->isChecked())
        this->setTarget(RunProgram);

    else if(ui->Target_TurnOff->isChecked())
        this->setTarget(TurnOff);


    // Go!!
    timer->start(1000); // 1 Sec
}

// Choose File : btn
void MainWindow::on_brows_clicked()
{
    QString Title, Types;

    // If Sounds Type
    if(ui->Target_PlaySound->isChecked()){
        Title = "Open Sounds";
        Types = "Audio Files (*.wav *.mp3 *amr *wma)";

    // If Any Type :)
    }else if(ui->Target_StartProgram->isChecked()){
        Title = "Open Files/Programes/Images";
        Types = "All Files (*.*)";
    }

    auto fileName = QFileDialog::getOpenFileName(this,Title, this->lastPath, Types);
    ui->path->setText(fileName);
}

// Volume
void MainWindow::on_volume_valueChanged(int value)
{
    // Change Label
    ui->volumeLabel->setText(QString("Volume (%1)").arg(value));
    // Change Player Volume
    Player->setVolume(value);
}

// Test Sound
void MainWindow::on_testSound_toggled(bool checked)
{
    if(checked){
        auto ext = QFileInfo(ui->path->text()).suffix();
        if(ext == "wav" || ext == "mp3" || ext == "amr" || ext == "wma"){
            this->Player->setMedia(QUrl::fromLocalFile(ui->path->text()));
            Player->play();
        }else{
            msg("Not Sound!", "<b>Only Audio Files <span style='color:red'>[.wav .mp3 .amr .wma]</span></b>");
            Player->stop();
        }
    }else{
        Player->stop();
    }
}

// When Close Program
void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    QSettings settings(MyOrg,MyApp);

    // Save last path
    settings.setValue("LAST_PATH",ui->path->text());

    // Save volume
    settings.setValue("VOLUME",ui->volume->value());
}
// Set : Target Function
void MainWindow::setTarget(MainWindow::Target t)
{
    targetVal = t;
}
// Do Job! : Target Function
void MainWindow::doTarget()
{
    if(targetVal == PlaySound){
        ui->testSound->setChecked(true); // like : "on_testSound_toggled(true)" but better :)

    }else if(targetVal == RunProgram){
        auto text = ui->path->text();
        QDesktopServices::openUrl(QUrl::fromUserInput(text));
    }
    else if (targetVal == TurnOff) {
        QProcess::startDetached("shutdown -s -f -t 00");
    }
}

// About
void MainWindow::AboutMe()
{
    QMessageBox about;
    about.setWindowTitle("About Creator");
    about.setIconPixmap(QPixmap(":/res/stopwatch.png").scaledToWidth(100,Qt::SmoothTransformation));

    about.setText( "<b style='font-size:12pt'><span style='color:#ff0f6f'>Z</span>Timer Job <span style='color:#67c411'>v1.0</span></b>"
                   "<br>"
                   "<span style='color:gray;font-size:12pt'>single job at time!</span>"
                   "<br><br>"
                   "The project was developed by : <b>ZAKARIA CHAHBOUN</b>"
                   "<br>"
                   "Generated at : 04/08/2018 07:35"
                   "<br><br>"
                   "Contacts:"
                   "<br>"
                   "________________________________________________"
                   "<br>"
                   "<span style='color:#1f6c9b'>Facebook: </span>zakaria.chahboun.2018"
                   "<br>"
                   "<span style='color:#1f6c9b'>Twitter: </span>zaki_chahboun"
                   "<br>"
                   "<span style='color:#1f6c9b'>GitHub: </span>zakaria-chahboun"
                   "<br>"
                   "<span style='color:#1f6c9b'>Gmail: </span>zakaria.chahboun.1996@gmail.com"
                   "<br>"
                   "________________________________________________"
                   "<br><br>"
                   "Call : +212 678807280 | +212 680982106"
                   "<br><br>"
                   "<b style='color:#e0115d'>All Rights Reserved.</b>"
                   "<br><br>"
                   "<img src=':/res/ZAKI PRO - PNG.png'/>"
                   );
    about.exec();
}
