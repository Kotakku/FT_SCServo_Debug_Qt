#include "simpegraphwiget.h"
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QtCore/QDebug>
#include <cmath>
#include <tuple>

SimpeGraphWiget::SimpeGraphWiget(QWidget *parent) :
    QWidget(parent)
{
    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &SimpeGraphWiget::onTimeout);
    timer_->start(30);    
}

void SimpeGraphWiget::paintEvent(QPaintEvent *event) {
    (void) event;
    QPainter painter(this);

    auto mapping = [](double val, double in_min, double in_max, double out_min, double out_max) {
        return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    };

    int w = width();
    int h = height();

    // Draw axis
    QPen grid_pen;
    grid_pen.setStyle(Qt::CustomDashLine);
    grid_pen.setDashPattern(QVector<qreal>({12, 4}));
    grid_pen.setWidthF(1.0);
    grid_pen.setColor(QColor(0,0,0));
    painter.setPen(grid_pen);

    // 垂直のグリッドラインを描画
    const double max_scale = 6.0;
    int min_grid_size = (h/11.5);
    int max_grid_size = min_grid_size * max_scale;
    int grid_size = (int)mapping(zoom, 0.0, 100.0, min_grid_size, max_grid_size);
    int hoffset = (int)mapping(horizontal, 0.0, 100.0, 0, w);
    double scale = mapping(zoom, 0.0, 100.0, 1.0, max_scale);

    for (int i = 0; i < w / grid_size + 1; i++) {
        int x = w - i * grid_size;
        painter.drawLine(x, 0, x, h);
    }

    // 水平のグリッドラインを描画
    for (int i = 0; i < 11; i++) {
        int y = h/2 + (i-5) * (h/11.5);
        painter.drawLine(0, y, w, y);
    }
    int grid_y_min = h/2 - 5 * (h/11.5);
    int grid_y_max = h/2 + 5 * (h/11.5);

    // Draw sine wave
    QPen pen;
    pen.setWidthF(1.2);
    painter.setPen(pen);
    QPainterPath path;


    auto left_axis_y = [&](int val) { return (int)(mapping(val, 0, 4000, grid_y_max, grid_y_min)); };
    auto right_axis_y = [&](int val) { return (int)(mapping(val, -1000, 1000, grid_y_max, grid_y_min)); };
    auto axis_x = [&](int x) { return (int)(mapping(x, 0, pos_buf_.max_size(), w - w*scale + hoffset, w + hoffset)); };
    
    using PlotConfig = std::tuple<QColor, bool, RingBuffer<int>*, std::function<int(int)>, double>;
    std::vector<PlotConfig> configs =
    {
        PlotConfig(QColorConstants::Svg::black, pos_visible, &pos_buf_, left_axis_y, 1.0),
        PlotConfig(QColorConstants::Svg::orange, torque_visible, &torque_buf_, right_axis_y, 1.0),
        PlotConfig(QColorConstants::Svg::green, speed_visible, &speed_buf_, right_axis_y, 0.2),
        PlotConfig(QColorConstants::Svg::cyan, current_visible, &current_buf_, right_axis_y, 1.0),
        PlotConfig(QColorConstants::Svg::yellowgreen, temp_visible, &temp_buf_, right_axis_y, 1.0),
        PlotConfig(QColorConstants::Svg::magenta, voltage_visible, &voltage_buf_, right_axis_y, 1.0),
    };
    painter.setRenderHint(QPainter::Antialiasing, true);
    for(auto &config : configs)
    {
        auto [color, visible, buf, axis_y_mapping, gain] = config;
        path.clear();
        if(visible)
        {
            pen.setColor(color);
            painter.setPen(pen);
            for(std::size_t i = 0; i < buf->size(); i++) {
                int x = axis_x(buf->max_size()-buf->size()+i);
                int y = axis_y_mapping(gain * buf->at(i));
                if(i == 0)
                    path.moveTo(x, y);
                else
                    path.lineTo(x, y);
            }
            painter.drawPath(path);
        }
    }

    // Up/Low
    path.clear();
    std::array<int, 2> limits = {up_limit, down_limit};
    for(auto & limit : limits)
    {
        if(limit != 0)
        {
            pen.setColor(QColorConstants::Svg::plum);
            painter.setPen(pen);
            int y = right_axis_y(limit);
            path.clear();
            path.moveTo(0, y);
            path.lineTo(w, y);
            painter.drawPath(path);
            y = right_axis_y(-limit);
            path.clear();
            path.moveTo(0, y);
            path.lineTo(w, y);
            painter.drawPath(path);
        }
    }
}

void SimpeGraphWiget::onTimeout() {
    update();
}

void SimpeGraphWiget::reset_data()
{
    pos_buf_.clear();
    torque_buf_.clear();
    speed_buf_.clear();
    current_buf_.clear();
    temp_buf_.clear();
    voltage_buf_.clear();
}

void SimpeGraphWiget::append_data(int pos, int torque, int speed, int current, int temp, int voltage)
{
    pos_buf_.push(pos);
    torque_buf_.push(torque);
    speed_buf_.push(speed);
    current_buf_.push(current);
    temp_buf_.push(temp);
    voltage_buf_.push(voltage);
}
