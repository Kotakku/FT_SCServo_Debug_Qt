#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QTableView>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QtCore/QDebug>
#include <QLineEdit>
#include <QIntValidator>
#include <QRegExpValidator>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("FT SCServo Debug Qt");

    graph_timer_ = new QTimer(this);
    connect(graph_timer_, &QTimer::timeout, this, &MainWindow::onGraphTimerTimeout);
    graph_timer_->start(30);

    serial_ = new QSerialPort(this);
    scserial_ = new feetech_servo::SMS_STS(serial_);

    setupComSettings();
    setupServoLists();

    setupServoControl();
    setupAutoDebug();

    setupProgramming();

    setIntRangeLineEdit(ui->upLimitLineEdit, 0, 1200);
    setIntRangeLineEdit(ui->downLimitLineEdit, 0, 1200);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete graph_timer_;
    delete serial_;
    delete scserial_;
    delete servo_list_model_;
    delete prog_mem_model_;
    delete port_search_timer_;
    delete search_timer_;
    delete servo_read_timer_;
    delete auto_debug_timer_;
    delete prog_timer_;
}

void MainWindow::setupComSettings()
{
    QStringList baudRates = {"1000000", "500000", "250000", "256000", "128000", "115200", "76800", "57600", "38400", "19200", "9600", "4800"};
    ui->BaudComboBox->addItems(baudRates);

    QStringList parity = {"NONE", "ODD", "EVEN"};
    ui->ParityComboBox->addItems(parity);

    setIntRangeLineEdit(ui->timeoutLineEdit, 0, 10000);
    onPortSearchTimerTimeout();
    connect(ui->ComOpenButton, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    port_search_timer_ = new QTimer(this);
    connect(port_search_timer_, &QTimer::timeout, this, &MainWindow::onPortSearchTimerTimeout);
    port_search_timer_->start(500);
}

void MainWindow::setupServoLists()
{
    connect(ui->SearchButton, &QPushButton::clicked, this, &MainWindow::onSearchButtonClicked);

    ui->ServoSearchText->setText("Stop");
    servo_list_model_ = new QStandardItemModel(0, 2);
    ui->ServoListView->setModel(servo_list_model_);
    clearServoList();

    search_timer_ = new QTimer(this);
    connect(search_timer_, &QTimer::timeout, this, &MainWindow::onSearchTimerTimeout);

    servo_read_timer_ = new QTimer(this);
    connect(servo_read_timer_, &QTimer::timeout, this, &MainWindow::onServoReadTimerTimeout);
    servo_read_timer_->start(10);

    connect(ui->ServoListView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onServoListSelection);
}

void MainWindow::setupServoControl()
{
    connect(ui->goalSlider, &QSlider::valueChanged, this, &MainWindow::onGoalSliderValueChanged);

    setIntRangeLineEdit(ui->accLineEdit, 0, std::numeric_limits<int>::max());
    setIntRangeLineEdit(ui->speedLineEdit, 0, std::numeric_limits<int>::max());
    setIntRangeLineEdit(ui->goalLineEdit, 0, 4095);
    setIntRangeLineEdit(ui->timeLineEdit, 0, std::numeric_limits<int>::max());
    
    connect(ui->setPushButton, &QPushButton::clicked, this, &MainWindow::onSetBuggonClicked);
}

void MainWindow::setupAutoDebug()
{
    setIntRangeLineEdit(ui->startLineEdit, 0, 4095);
    setIntRangeLineEdit(ui->endLineEdit, 0, 4095);
    setIntRangeLineEdit(ui->sweepLineEdit, 0, std::numeric_limits<int>::max());
    setIntRangeLineEdit(ui->setpLineEdit, 1, std::numeric_limits<int>::max());
    setIntRangeLineEdit(ui->setpDelayLineEdit, 1, std::numeric_limits<int>::max());

    connect(ui->sweepButton, &QPushButton::clicked, this, &MainWindow::onSweepButtonClicked);
    connect(ui->setpButton, &QPushButton::clicked, this, &MainWindow::onSetpButtonClicked);

    auto_debug_timer_ = new QTimer(this);
    connect(auto_debug_timer_, &QTimer::timeout, this, &MainWindow::onAutoDebugTimerTimeout);
}

void MainWindow::setupProgramming()
{
    prog_mem_model_ = new QStandardItemModel(0, 5);
    ui->memoryTableView->setModel(prog_mem_model_);
    ui->memoryTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->memoryTableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    ui->memoryTableView->verticalHeader()->setVisible(false);
    ui->memoryTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->memoryTableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->memoryTableView->setColumnWidth(0, 70);
    ui->memoryTableView->setColumnWidth(2, 70);
    ui->memoryTableView->setColumnWidth(3, 70);
    ui->memoryTableView->setColumnWidth(4, 70);
    ui->memoryTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    prog_mem_model_->setHorizontalHeaderLabels(QStringList() << "Address" << "Memory" << "Value" << "Area" << "R/W");

    for(auto &item : feetech_servo::STSMemConfig)
    {
        auto &[address, name, size, default_value, is_eprom, is_readonly, min_val, max_val] = item;
        QString area = is_eprom ? "EPROM" : "SRAM";
        QString rw = is_readonly ? "R" : "R/W";
        QList<QStandardItem*> rowList;

        rowList << new QStandardItem(QString::number(address)) << new QStandardItem(name) << new QStandardItem(QString::number(default_value)) << new QStandardItem(area) << new QStandardItem(rw);
        prog_mem_model_->appendRow(rowList);
    }

    connect(ui->memoryTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onMemoryTableSelection);
    connect(ui->memSetButton, &QPushButton::clicked, this, &MainWindow::onMemSetButtonClicked);

    prog_timer_ = new QTimer(this);
    connect(prog_timer_, &QTimer::timeout, this, &MainWindow::onProgTimerTimeout);
    prog_timer_->start(50);
}

void MainWindow::setEnableComSettings(bool state)
{
    ui->ComComboBox->setEnabled(state);
    ui->BaudComboBox->setEnabled(state);
    ui->ParityComboBox->setEnabled(state);
    ui->timeoutLineEdit->setEnabled(state);
}

void MainWindow::clearServoList()
{
    servo_list_model_->clear();
    servo_list_model_->setHorizontalHeaderLabels(QStringList() << "ID" << "Module");

    ui->ServoListView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->ServoListView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    ui->ServoListView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->ServoListView->horizontalScrollBar()->setDisabled(true);
    ui->ServoListView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->ServoListView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->ServoListView->setColumnWidth(0, 50);
    ui->ServoListView->verticalHeader()->setVisible(false);
    ui->ServoListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::appendServoList(const int id, const QString &name)
{
    servo_list_model_->appendRow(QList<QStandardItem*>() << new QStandardItem(QString::number(id)) << new QStandardItem(name));
}

void MainWindow::setIntRangeLineEdit(QLineEdit *edit, int min, int max)
{
    edit->setValidator(new QIntValidator(min, max, edit));
}

void MainWindow::setIntLineEdit(QLineEdit *edit)
{
    edit->setValidator(new QRegExpValidator(QRegExp("-?\\d*"), edit));
}



void MainWindow::onPortSearchTimerTimeout()
{
    if(serial_->isOpen())
        return;

    ui->ComComboBox->clear();
    for (const auto &info : QSerialPortInfo::availablePorts()) {
        ui->ComComboBox->addItem(info.portName());
    }
}

void MainWindow::onConnectButtonClicked()
{
    if (serial_->isOpen()) {
        serial_->close();
        ui->ComOpenButton->setText("Open");
        setEnableComSettings(true);
        select_id_ = -1;
    }
    else {
        serial_->setPortName(ui->ComComboBox->currentText());
        serial_->setBaudRate(ui->BaudComboBox->currentText().toInt());
        uint8_t pidx = ui->ParityComboBox->currentIndex();
        QSerialPort::Parity p = QSerialPort::Parity::NoParity;
        if(pidx == 1)
            p = QSerialPort::Parity::OddParity;
        if(pidx == 2)
            p = QSerialPort::Parity::EvenParity;
        serial_->setParity(p);
        serial_->setDataBits(QSerialPort::DataBits::Data8);
        serial_->setStopBits(QSerialPort::StopBits::OneStop);
        serial_->setFlowControl(QSerialPort::FlowControl::NoFlowControl);
        if (serial_->open(QIODevice::ReadWrite)) {
            ui->ComOpenButton->setText("Close");
            setEnableComSettings(false);
        }
        else {
            ui->ComOpenButton->setText("Open");
        }
        scserial_->set_timeout(ui->timeoutLineEdit->text().toUInt());
    }
}

void MainWindow::onSearchButtonClicked()
{
    if(!serial_->isOpen())
    {
        qDebug() << "serial not open";
        return;
    }

    is_searching_ = !is_searching_;

    if(is_searching_)
    {
        ui->SearchButton->setText("Stop");
        clearServoList();
        search_id_ = 0;
        search_timer_->start(10);
        onSearchTimerTimeout();
    }
    else
    {
        ui->SearchButton->setText("Search");
        search_timer_->stop();
        ui->ServoSearchText->setText(QString("Stop"));
    }
}

void MainWindow::onSearchTimerTimeout()
{
    search_timer_->stop();
    if(!is_searching_)
        return;

    if(0xfd < search_id_ || serial_->isOpen() == false)
    {
        is_searching_ = false;
        ui->SearchButton->setText("Search");
        ui->ServoSearchText->setText("Stop");
    }
    else
    {
        ui->ServoSearchText->setText(QString("Ping ID:%1 Servo...").arg(search_id_));
        int ret = scserial_->ping(search_id_);

        if(0 < ret)
        {
            int mid = scserial_->read_model_number(ret);
            QString name = feetech_servo::getModelType(mid);
            appendServoList(ret, name);
            select_id_ = ret;
        }
        search_id_++;
        search_timer_->start(1);
        
    }
}

void MainWindow::onServoListSelection()
{
    QModelIndex selectedRows = ui->ServoListView->selectionModel()->selectedRows()[0];
    std::size_t row = selectedRows.row();
    auto index = servo_list_model_->index(row, 0);
    select_id_ = servo_list_model_->data(index).toInt();
}

void MainWindow::onGoalSliderValueChanged()
{
    int goal = ui->goalSlider->value();
    ui->goalLineEdit->setText(QString::number(goal));

    if(serial_->isOpen() == false || select_id_ < 0)
        return;
    
    scserial_->rotation_mode(select_id_);
    scserial_->write_pos_ex(select_id_, goal, 0, 0);
}

void MainWindow::onSetBuggonClicked()
{
    int goal = ui->goalLineEdit->text().toUInt();
    int speed = ui->speedLineEdit->text().toUInt();
    int acc = ui->accLineEdit->text().toUInt();
    // int time = ui->timeLineEdit->text().toUInt();
    ui->goalSlider->setValue(goal);

    if(serial_->isOpen() == false || select_id_ < 0)
        return;
    
    scserial_->rotation_mode(select_id_);
    scserial_->write_pos_ex(select_id_, goal, speed, acc);
}

void MainWindow::onSweepButtonClicked()
{
    if(!isServoValidNow())
        return;
    
    if(sweep_running_)
    {
        sweep_running_ = false;
        ui->sweepButton->setText("Sweep");
        ui->setpButton->setEnabled(true);
        auto_debug_timer_->stop();
    }
    else
    {
        sweep_running_ = true;
        ui->sweepButton->setText("Stop");
        ui->setpButton->setEnabled(false);
        latest_auto_debug_goal_ = ui->startLineEdit->text().toUInt();
        scserial_->write_pos_ex(select_id_, latest_auto_debug_goal_, 0, 0);
        auto_debug_timer_->start(ui->sweepLineEdit->text().toUInt());
    }
}

void MainWindow::onSetpButtonClicked()
{
    if(!isServoValidNow())
        return;
    
    if(setp_running_)
    {
        setp_running_ = false;
        ui->setpButton->setText("Setp");
        ui->sweepButton->setEnabled(true);
        auto_debug_timer_->stop();
    }
    else
    {
        setp_running_ = true;
        setp_increase_ = true;
        ui->setpButton->setText("Stop");
        ui->sweepButton->setEnabled(false);
        latest_auto_debug_goal_ = ui->startLineEdit->text().toUInt();
        scserial_->write_pos_ex(select_id_, latest_auto_debug_goal_, 0, 0);
        auto_debug_timer_->start(ui->setpDelayLineEdit->text().toUInt());
    }
}

void MainWindow::onAutoDebugTimerTimeout()
{
    if(!isServoValidNow())
    {
        auto_debug_timer_->stop();
        sweep_running_ = false;
        setp_running_ = false;
        ui->sweepButton->setText("Sweep");
        ui->sweepButton->setEnabled(true);
        ui->setpButton->setText("Setp");
        ui->setpButton->setEnabled(true);
        return;
    }

    if(sweep_running_)
    {
        int start = ui->startLineEdit->text().toUInt();
        int end = ui->endLineEdit->text().toUInt();
        if(latest_auto_debug_goal_ == start)
        {
            latest_auto_debug_goal_ = end;
        }
        else
        {
            latest_auto_debug_goal_ = start;
        }
        scserial_->write_pos_ex(select_id_, latest_auto_debug_goal_, 0, 0);
    }
    else if(setp_running_)
    {
        int start = ui->startLineEdit->text().toUInt();
        int end = ui->endLineEdit->text().toUInt();
        int step = ui->setpLineEdit->text().toUInt();
        
        latest_auto_debug_goal_ += setp_increase_ ? step : -step;
        
        if(end < latest_auto_debug_goal_)
        {
            latest_auto_debug_goal_ = end;
            setp_increase_ = false;
        }
        else if(latest_auto_debug_goal_ < start)
        {
            latest_auto_debug_goal_ = start;
            setp_increase_ = true;
        }

        scserial_->write_pos_ex(select_id_, latest_auto_debug_goal_, 0, 0);
    }
    else
    {
        auto_debug_timer_->stop();
    }
}

void MainWindow::onProgTimerTimeout()
{
    if(ui->tabWidget->currentIndex() != 1)
        return;

    if(!isServoValidNow())
        return;

    if(is_mem_writing_)
        return;

    // 表に表示されている行の値のみ更新する
    int firstVisibleRow = ui->memoryTableView->indexAt(ui->memoryTableView->viewport()->rect().topLeft()).row();
    int lastVisibleRow = ui->memoryTableView->indexAt(ui->memoryTableView->viewport()->rect().bottomLeft()).row();
    if (firstVisibleRow != -1 && lastVisibleRow != -1)
    {
        for(int i = firstVisibleRow; i <= lastVisibleRow; i++)
        {
            // メモリ更新
            auto &[address, name, size, default_value, is_eprom, is_readonly, min_val, max_val] = feetech_servo::STSMemConfig[i];
            
            int val = 0;
            if(size == 2)
            {
                val = scserial_->read_word(select_id_, address);
            }
            else
            {
                val = scserial_->read_byte(select_id_, address);
            }

            ui->memoryTableView->model()->setData(ui->memoryTableView->model()->index(i,2), QString::number(val));
        }
    }
}

void MainWindow::onMemoryTableSelection()
{
    QModelIndex selectedRows = ui->memoryTableView->selectionModel()->selectedRows().first();
    std::size_t row = selectedRows.row();
    auto index = prog_mem_model_->index(row, 1);
    ui->memLabel->setText(prog_mem_model_->data(index).toString());
    ui->memSetLineEdit->setText(prog_mem_model_->data(prog_mem_model_->index(row, 2)).toString());
    bool is_readonly = feetech_servo::STSMemConfig[row].is_readonly;
    if(is_readonly)
    {
        ui->memSetLineEdit->setEnabled(false);
        ui->memSetButton->setEnabled(false);
    }
    else
    {
        ui->memSetLineEdit->setEnabled(true);
        ui->memSetButton->setEnabled(true);
    }
}

void MainWindow::onMemSetButtonClicked()
{
    is_mem_writing_ = true;
    QModelIndex selectedRows = ui->memoryTableView->selectionModel()->selectedRows().first();
    auto &[address, name, size, default_value, is_eprom, is_readonly, min_val, max_val] = feetech_servo::STSMemConfig[selectedRows.row()];

    if(address == 5)
    {
        uint8_t val = ui->memSetLineEdit->text().toShort();
        scserial_->write_byte(select_id_, 55, 0); // unlock
        scserial_->write_byte(select_id_, address, val);
        scserial_->write_byte(select_id_, 55, 1); // lock
        select_id_ = val;
    }
    if(size == 2)
    {
        int16_t val = ui->memSetLineEdit->text().toShort();
        scserial_->write_word(select_id_, address, val);
    }
    else
    {
        uint8_t val = ui->memSetLineEdit->text().toShort();
        scserial_->write_byte(select_id_, address, val);
    }
    is_mem_writing_ = false;
}

void MainWindow::onGraphTimerTimeout()
{
    ui->graphWidget->up_limit = ui->upLimitLineEdit->text().toUInt();
    ui->graphWidget->down_limit = ui->downLimitLineEdit->text().toUInt();
    ui->graphWidget->horizontal = ui->horizontalSlider->value();
    ui->graphWidget->zoom = ui->zoomSlider->value();

    if(is_searching_ || !serial_->isOpen() || select_id_ < 0)
        return;

    ui->positionLabel->setText(QString::number(latest_status_.pos));
    ui->torqueLabel->setText(QString::number(latest_status_.torque));
    ui->speedLabel->setText(QString::number(latest_status_.speed));
    ui->currentLabel->setText(QString::number(latest_status_.current));
    ui->temperatureLabel->setText(QString::number(latest_status_.temp));
    ui->voltageLabel->setText(QString::number(0.1*latest_status_.voltage, 'f', 1) + QString("V"));
    ui->movingLabel->setText(QString::number(latest_status_.move));
    ui->goalLabel->setText(QString::number(latest_status_.goal));

    ui->graphWidget->pos_visible = ui->posCheckBox->isChecked();
    ui->graphWidget->torque_visible = ui->torqueCheckBox->isChecked();
    ui->graphWidget->speed_visible = ui->speedCheckBox->isChecked();
    ui->graphWidget->current_visible = ui->currentCheckBox->isChecked();
    ui->graphWidget->temp_visible = ui->tempCheckBox->isChecked();
    ui->graphWidget->voltage_visible = ui->voltageCheckBox->isChecked();
}

void MainWindow::onServoReadTimerTimeout()
{
    if(ui->tabWidget->currentIndex() != 0)
        return;
    
    static int count = 0;
    if (serial_->isOpen() && select_id_ >= 0 && !is_searching_)
    {
        switch(count)
        {
            case 0:
                latest_status_.pos = scserial_->read_position(select_id_);
                latest_status_.torque = scserial_->read_load(select_id_);
                break;

            case 1:
                latest_status_.speed = scserial_->read_speed(select_id_);
                latest_status_.current = scserial_->read_current(select_id_);
                latest_status_.temp = scserial_->read_temperature(select_id_);
                break;

            case 2:
                latest_status_.voltage = scserial_->read_voltage(select_id_);
                latest_status_.move = scserial_->read_move(select_id_);
                latest_status_.goal = scserial_->read_goal(select_id_);
                ui->graphWidget->append_data(latest_status_.pos, latest_status_.torque, latest_status_.speed, latest_status_.current, latest_status_.temp, latest_status_.voltage);
                break;
        }
        count++;
        count %= 3;
    }
}
