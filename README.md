# FT SCServo Debug Qt

Utility for Feetech SCS/STS servo

## Development Env
- Ubuntu 22.04
- Qt Creator 6.0.2
- Qt 5.14 or 5.15

## Dependices for servo board
- CH340 driver

## Build with Qt
Clone and open this repository and build the project.

## Build with qmake
Install dependencies
```
sudo apt update
sudo apt install -y libqt5serialport5 libqt5serialport5-dev qt5-qmake g++ cmake git
```

Then clone and build repository
```
git clone https://github.com/Kotakku/FT_SCServo_Debug_Qt.git
cd FT_SCServo_Debug_Qt/
qmake .
make
```

## Build deb package with Docker
```
git clone https://github.com/Kotakku/FT_SCServo_Debug_Qt.git
cd FT_SCServo_Debug_Qt/deb
bash create_deb.bash
```


## ⚠️Unported Features⚠️
- Sync Write
- Reg Write
- Data analysis
- Upgrade tab & features
- PWM tabs & features

## Screenshot

![](pic/debug.png)
![](pic/prog.png)