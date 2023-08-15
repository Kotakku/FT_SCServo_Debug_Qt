#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <QSerialPort>
#include <QTableView>
#include <QStandardItemModel>
#include "scserial.h"
#include <map>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupComSettings();
    void setupServoLists();
    void setupServoControl();
    void setupAutoDebug();
    void setupProgramming();

    void setEnableComSettings(bool state);
    void clearServoList();
    void appendServoList(const int id, const QString &name);
    void setIntRangeLineEdit(QLineEdit *edit, int min, int max);
    void setIntLineEdit(QLineEdit *edit);

    bool isServoValidNow() { return !(is_searching_ || !serial_->isOpen() || select_id_ < 0); }

private slots:
    // com
    void onPortSearchTimerTimeout();
    void onConnectButtonClicked();

    // servo list
    void onSearchButtonClicked();
    void onSearchTimerTimeout();
    void onServoListSelection();

    // servo control
    void onGoalSliderValueChanged();
    void onSetBuggonClicked();

    // auto debug
    void onSweepButtonClicked();
    void onSetpButtonClicked();
    void onAutoDebugTimerTimeout();

    // programming
    void onProgTimerTimeout();
    void onMemoryTableSelection();
    void onMemSetButtonClicked();

    // other
    void onGraphTimerTimeout();
    void onServoReadTimerTimeout();

private:
    Ui::MainWindow *ui;
    QTimer *graph_timer_;
    QSerialPort *serial_;
    feetech_servo::SMS_STS *scserial_;
    QStandardItemModel *servo_list_model_;
    QStandardItemModel *prog_mem_model_;
    QTimer *port_search_timer_;
    QTimer *search_timer_;
    QTimer *servo_read_timer_;
    QTimer *auto_debug_timer_;
    QTimer *prog_timer_;

    bool is_searching_ = false;
    int search_id_ = 0;
    int select_id_ = -1;
    bool sweep_running_ = false;
    bool setp_running_ = false;
    int latest_auto_debug_goal_ = 0;
    bool setp_increase_ = true;
    bool is_mem_writing_ = false;

    struct
    {
        int pos = 0;
        int torque = 0;
        int speed = 0;
        int current = 0;
        int temp = 0;
        int voltage = 0;
        int move = 0;
        int goal = 0;
    } latest_status_;
};
#endif // MAINWINDOW_H
