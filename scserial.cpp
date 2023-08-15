#include "scserial.h"

namespace feetech_servo
{

#define SERVO_MODEL(major, minor, name) { (minor<<8 | major), name }
QString getModelType(uint16_t id)
{
    static std::map<uint16_t, QString> model_list =
    {
        SERVO_MODEL(5, 0, "SCSXX"), 
        SERVO_MODEL(5, 4, "SCS009"), 
        SERVO_MODEL(5, 8, "SCS2332"), 
        SERVO_MODEL(5, 12, "SCS45"), 
        SERVO_MODEL(5, 15, "SCS15"), 
        SERVO_MODEL(5, 16, "SCS315"), 
        SERVO_MODEL(5, 25, "SCS115"), 
        SERVO_MODEL(5, 35, "SCS215"), 
        SERVO_MODEL(5, 40, "SCS40"), 
        SERVO_MODEL(5, 60, "SCS6560"), 
        SERVO_MODEL(5, 240, "SCDZZ"), 
        SERVO_MODEL(6, 0, "SMXX-360M"), 
        SERVO_MODEL(6, 3, "SM30-360M"), 
        SERVO_MODEL(6, 8, "SM60-360M"), 
        SERVO_MODEL(6, 12, "SM80-360M"), 
        SERVO_MODEL(6, 16, "SM100-360M"), 
        SERVO_MODEL(6, 20, "SM150-360M"), 
        SERVO_MODEL(6, 24, "SM85-360M"), 
        SERVO_MODEL(6, 26, "SM60-360M"), 
        SERVO_MODEL(8, 0, "SM30BL"), 
        SERVO_MODEL(8, 1, "SM30BL"), 
        SERVO_MODEL(8, 2, "SM30BL"), 
        SERVO_MODEL(8, 3, "SM30BL"), 
        SERVO_MODEL(8, 4, "SM30BL"), 
        SERVO_MODEL(8, 5, "SM30BL"), 
        SERVO_MODEL(8, 6, "SM30BL"), 
        SERVO_MODEL(8, 7, "SM30BL"), 
        SERVO_MODEL(8, 8, "SM30BL"), 
        SERVO_MODEL(8, 9, "SM30BL"), 
        SERVO_MODEL(8, 10, "SM30BL"), 
        SERVO_MODEL(8, 11, "SM30BL"), 
        SERVO_MODEL(8, 12, "SM30BL"), 
        SERVO_MODEL(8, 13, "SM30BL"), 
        SERVO_MODEL(8, 14, "SM30BL"), 
        SERVO_MODEL(8, 15, "SM30BL"), 
        SERVO_MODEL(8, 16, "SM30BL"), 
        SERVO_MODEL(8, 17, "SM30BL"), 
        SERVO_MODEL(8, 18, "SM30BL"), 
        SERVO_MODEL(8, 19, "SM30BL"), 
        SERVO_MODEL(8, 25, "SM29BL(LJ)"), 
        SERVO_MODEL(8, 29, "SM29BL(FT)"), 
        SERVO_MODEL(8, 30, "SM30BL(FT)"), 
        SERVO_MODEL(8, 20, "SM30BL(LJ)"), 
        SERVO_MODEL(8, 40, "SM40BLHV"), 
        SERVO_MODEL(8, 42, "SM45BLHV"), 
        SERVO_MODEL(8, 44, "SM85BLHV"), 
        SERVO_MODEL(8, 120, "SM120BLHV"), 
        SERVO_MODEL(8, 220, "SM200BLHV"), 
        SERVO_MODEL(9, 0, "STSXX"), 
        SERVO_MODEL(9, 2, "STS3032"), 
        SERVO_MODEL(9, 3, "STS3215"), 
        SERVO_MODEL(9, 4, "STS3040"), 
        SERVO_MODEL(9, 5, "STS3020"), 
        SERVO_MODEL(9, 6, "STS3046"), 
        SERVO_MODEL(9, 20, "SCSXX-2"), 
        SERVO_MODEL(9, 15, "SCS15-2"), 
        SERVO_MODEL(9, 35, "SCS225"), 
        SERVO_MODEL(9, 40, "SCS40-2"), 
    };

    if(auto it = model_list.find(id); it != model_list.end())
    {
        return it->second;
    }

    return "Unknown";
}
#undef SERVO_MODEL

SCSerial::SCSerial(QSerialPort *serial)
    : serial_(serial)
{
    level_ = 1;
    error_ = 0;
    end_ = 0;
}

int SCSerial::gen_write(uint8_t id, uint8_t mem_addr, uint8_t *n_dat, uint8_t n_len)
{
    read_flush();
    write_buf(id, mem_addr, n_dat, n_len, INST_WRITE);
    write_flush();
    return ask(id);
}

int SCSerial::reg_write(uint8_t id, uint8_t mem_addr, uint8_t *n_dat, uint8_t n_len)
{
    read_flush();
    write_buf(id, mem_addr, n_dat, n_len, INST_REG_WRITE);
    write_flush();
    return ask(id);
}

int SCSerial::reg_write_action(uint8_t id)
{
    read_flush();
    write_buf(id, 0, NULL, 0, INST_REG_ACTION);
    write_flush();
    return ask(id);
}

void SCSerial::sync_write(uint8_t id[], uint8_t idn, uint8_t mem_addr, uint8_t *n_dat, uint8_t n_len)
{
    read_flush();
    uint8_t mes_len = ((n_len+1)*idn+4);
    uint8_t sum = 0;
    uint8_t b_buf[7];
    b_buf[0] = 0xff;
    b_buf[1] = 0xff;
    b_buf[2] = 0xfe;
    b_buf[3] = mes_len;
    b_buf[4] = INST_SYNC_WRITE;
    b_buf[5] = mem_addr;
    b_buf[6] = n_len;
    write(b_buf, 7);

    sum = 0xfe + mes_len + INST_SYNC_WRITE + mem_addr + n_len;
    uint8_t i, j;
    for(i=0; i<idn; i++){
        write(id[i]);
        write(n_dat+i*n_len, n_len);
        sum += id[i];
        for(j=0; j<n_len; j++){
            sum += n_dat[i*n_len+j];
        }
    }
    write(~sum);
    write_flush();
}

int SCSerial::write_byte(uint8_t id, uint8_t mem_addr, uint8_t b_dat)
{
    read_flush();
    write_buf(id, mem_addr, &b_dat, 1, INST_WRITE);
    write_flush();
    return ask(id);
}

int SCSerial::write_word(uint8_t id, uint8_t mem_addr, uint16_t w_dat)
{
    uint8_t b_buf[2];
    host_2_scs(b_buf+0, b_buf+1, w_dat);
    read_flush();
    write_buf(id, mem_addr, b_buf, 2, INST_WRITE);
    write_flush();
    return ask(id);
}

int SCSerial::read(uint8_t id, uint8_t mem_addr, uint8_t *n_data, uint8_t n_len)
{
    read_flush();
    write_buf(id, mem_addr, &n_len, 1, INST_READ);
    write_flush();
    if(!check_head()){
        return 0;
    }
    uint8_t b_buf[4];
    error_ = 0;
    if(read(b_buf, 3)!=3){
        return 0;
    }
    int size = read(n_data, n_len);
    if(size!=n_len){
        return 0;
    }
    if(read(b_buf+3, 1)!=1){
        return 0;
    }
    uint8_t cal_sum = b_buf[0]+b_buf[1]+b_buf[2];
    uint8_t i;
    for(i=0; i<size; i++){
        cal_sum += n_data[i];
    }
    cal_sum = ~cal_sum;
    if(cal_sum!=b_buf[3]){
        return 0;
    }
    error_ = b_buf[2];
    return size;
}

int SCSerial::read_byte(uint8_t id, uint8_t mem_addr)
{
    uint8_t b_dat;
    int size = read(id, mem_addr, &b_dat, 1);
    if(size!=1){
        return -1;
    }else{
        return b_dat;
    }
}

int SCSerial::read_word(uint8_t id, uint8_t mem_addr)
{
    uint8_t n_dat[2];
    int size;
    uint16_t w_dat;
    size = read(id, mem_addr, n_dat, 2);
    if(size!=2)
        return -1;
    w_dat = scs_2_host(n_dat[0], n_dat[1]);
    return w_dat;
}

int SCSerial::ping(uint8_t id)
{
    read_flush();
    write_buf(id, 0, NULL, 0, INST_PING);
    write_flush();
    error_ = 0;
    if(!check_head()){
        return -1;
    }
    uint8_t b_buf[4];
    if(read(b_buf, 4)!=4){
        return -1;
    }
    if(b_buf[0]!=id && id!=0xfe){
        return -1;
    }
    if(b_buf[1]!=2){
        return -1;
    }
    uint8_t cal_sum = ~(b_buf[0]+b_buf[1]+b_buf[2]);
    if(cal_sum!=b_buf[3]){
        return -1;			
    }
    error_ = b_buf[2];
    return b_buf[0];
}

void SCSerial::write_buf(uint8_t id, uint8_t mem_addr, uint8_t *n_dat, uint8_t n_len, uint8_t fun)
{
    uint8_t msg_len = 2;
    uint8_t b_buf[6];
    uint8_t check_sum = 0;
    b_buf[0] = 0xff;
    b_buf[1] = 0xff;
    b_buf[2] = id;
    b_buf[4] = fun;
    if(n_dat){
        msg_len += n_len + 1;
        b_buf[3] = msg_len;
        b_buf[5] = mem_addr;
        write(b_buf, 6);
        
    }else{
        b_buf[3] = msg_len;
        write(b_buf, 5);
    }
    check_sum = id + msg_len + fun + mem_addr;
    uint8_t i = 0;
    if(n_dat){
        for(i=0; i<n_len; i++){
            check_sum += n_dat[i];
        }
        write(n_dat, n_len);
    }
    write(~check_sum);
}

void SCSerial::host_2_scs(uint8_t *data_l, uint8_t* data_h, uint16_t data)
{
    if(end_){
        *data_l = (data>>8);
        *data_h = (data&0xff);
    }else{
        *data_h = (data>>8);
        *data_l = (data&0xff);
    }
}

uint16_t SCSerial::scs_2_host(uint8_t data_l, uint8_t data_h)
{
    uint16_t data;
    if(end_){
        data = data_l;
        data<<=8;
        data |= data_h;
    }else{
        data = data_h;
        data<<=8;
        data |= data_l;
    }
    return data;
}

int	SCSerial::ask(uint8_t id)
{
    error_ = 0;
    if(id!=0xfe && level_){
        if(!check_head()){
            return 0;
        }
        uint8_t b_buf[4];
        if(read(b_buf, 4)!=4){
            return 0;
        }
        if(b_buf[0]!=id){
            return 0;
        }
        if(b_buf[1]!=2){
            return 0;
        }
        uint8_t cal_sum = ~(b_buf[0]+b_buf[1]+b_buf[2]);
        if(cal_sum!=b_buf[3]){
            return 0;			
        }
        error_ = b_buf[2];
    }
    return 1;
}

int SCSerial::check_head()
{
    uint8_t b_dat;
    uint8_t b_buf[2] = {0, 0};
    uint8_t cnt = 0;
    while(1){
        if(!read(&b_dat, 1)){
            return 0;
        }
        b_buf[1] = b_buf[0];
        b_buf[0] = b_dat;
        if(b_buf[0]==0xff && b_buf[1]==0xff){
            break;
        }
        cnt++;
        if(cnt>10){
            return 0;
        }
    }
    return 1;
}

int SCSerial::write(uint8_t *n_dat, int n_len) {
    return serial_->write(reinterpret_cast<const char *>(n_dat), n_len);
}

int SCSerial::read(uint8_t *n_dat, int n_len) {
    if(serial_->bytesAvailable() < n_len)
        serial_->waitForReadyRead(100);
    return serial_->read(reinterpret_cast<char *>(n_dat), n_len);
}

int SCSerial::write(uint8_t b_dat) {
    return serial_->write(reinterpret_cast<const char *>(&b_dat), 1);
}

} // namespace feetech_servo
