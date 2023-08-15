#ifndef SIMPEGRAPHWIGET_H
#define SIMPEGRAPHWIGET_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>

class SimpeGraphWiget : public QWidget
{
    Q_OBJECT
public:
    explicit SimpeGraphWiget(QWidget *parent = nullptr);

    template<typename T>
    class RingBuffer {
    public:
        RingBuffer(std::size_t size = 300)
            : buffer(size), capacity(size), head(0), tail(0), count(0) {}

        void push(const T& value) {
            if (count == capacity) {
                tail = (tail + 1) % capacity;
            } else {
                count++;
            }

            buffer[head] = value;
            head = (head + 1) % capacity;
        }

        bool pop(T& value) {
            if (is_empty()) {
                return false; // バッファが空
            }

            value = buffer[tail];
            tail = (tail + 1) % capacity;
            count--;
            return true;
        }

        T at(std::size_t index) const {
            if (index >= count) {
                throw std::out_of_range("Index out of range");
            }
            return buffer[(tail + index) % capacity];
        }

        bool is_empty() const {
            return count == 0;
        }

        std::size_t size() const {
            return count;
        }

        std::size_t max_size() const {
            return capacity;
        }

        void resize(std::size_t size){
            buffer.resize(size); 
        }

        void clear() {
            head = 0;
            tail = 0;
            count = 0;
        }

    private:
        std::vector<T> buffer;
        std::size_t capacity;
        std::size_t head;
        std::size_t tail;
        std::size_t count;
    };

    void reset_data();
    void append_data(int pos, int torque, int speed, int current, int temp, int voltage);

    bool pos_visible;
    bool torque_visible;
    bool speed_visible;
    bool current_visible;
    bool temp_visible;
    bool voltage_visible;

    int down_limit;
    int up_limit;
    int horizontal;
    int zoom;

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onTimeout();

private:
    QTimer *timer_;
    double phase_ = 0;

    RingBuffer<int> pos_buf_;
    RingBuffer<int> torque_buf_;
    RingBuffer<int> speed_buf_;
    RingBuffer<int> current_buf_;
    RingBuffer<int> temp_buf_;
    RingBuffer<int> voltage_buf_;
};

#endif // SIMPEGRAPHWIGET_H
