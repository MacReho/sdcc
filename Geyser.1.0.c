#include <8051.h>

// control pins for LCD
sbit rs = P1^0;     // Register Select
sbit rw = P1^1;     // Read/Write
sbit e = P1^2;      // Enable

// Button and peripheral pins
sbit dig_hr1 = P1^3;        // Hour button
sbit dig_min1 = P1^4;       // Minute button
sbit start = P1^5;          // Start button
sbit temperatureSensor = P0^0;
sbit heater = P0^1;
sbit redLight = P0^2;
sbit greenLight = P0^3;

int min1 = 0, hr1 = 0;
int min0 = 60, hr0 = 25;
unsigned char temp = 60, hr, min, sec, num[60] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0X09, 0X10, 0X11, 0X12, 0X13,
    0X14, 0X15, 0X16, 0X17, 0X18, 0X19, 0X20, 0X21, 0X22, 0X23, 0X24, 0X25, 0X26, 0X27, 0X28, 0X29, 0X30, 0X31, 0X32, 0X33, 0X34, 0X35,
    0X36, 0X37, 0X38, 0X39, 0X40, 0X41, 0X42, 0X43, 0X44, 0X45, 0X46, 0X47, 0X48, 0X49, 0X50, 0X51, 0X52, 0X53, 0X54, 0X55, 0X56, 0X57,
    0X58, 0X59
};

void delay(unsigned int msec) {
    int i, j;
    for (i = 0; i < msec; i++)
        for (j = 0; j < 1275; j++);
}

void lcd_cmd(unsigned char item) {
    P2 = item;
    rs = 0;
    rw = 0;
    e = 1;
    delay(1);
    e = 0;
    return;
}

void lcd_data(unsigned char item) {
    P2 = item;
    rs = 1;
    rw = 0;
    e = 1;
    delay(1);
    e = 0;
    return;
}

void lcd_string(unsigned char *str) {
    int i = 0;
    while (str[i] != '\0') {
        lcd_data(str[i]);
        i++;
        delay(1);
    }
    return;
}

void lcd_int(int time_val) {
    int int_amt;
    int_amt = time_val / 10;
    lcd_data(int_amt + 48);
    int_amt = time_val % 10;
    lcd_data(int_amt + 48);
}

void lcd_init() {
    lcd_cmd(0x38);      // 2 lines, 5x7 matrix
    delay(5);
    lcd_cmd(0x0C);      // Display on, cursor off
    delay(5);
    lcd_cmd(0x80);      // Set cursor to the beginning of the first line
    delay(5);
}

void set_hr1() {
    hr1++;
    if (hr1 > 23)
        hr1 = 0;
    lcd_cmd(0xc3);
    lcd_int(hr1);
    lcd_data(':');
    hr0 = hr1;
}

void set_min1() {
    min1++;
    if (min1 > 59)
        min1 = 0;
    lcd_cmd(0xc6);
    lcd_int(min1);
    min0 = min1;
}

void set_time() interrupt 2 {
    lcd_cmd(0x01);
    if (start == 0) {
        lcd_string("SET TIMING");
        lcd_cmd(0xc3);
        lcd_int(hr1);
        lcd_data(':');
        lcd_int(min1);
        while (start == 0) {
            delay(10);
            if (dig_hr1 == 0)
                set_hr1();
            if (dig_min1 == 0)
                set_min1();
        }
    }
    lcd_cmd(0x01);
    hr = num[hr1];
    min = num[min1];
    lcd_cmd(0x80);
    lcd_string("TIME:");
    hr0 = 25;
    min0 = 60;
}

void bcdconv(unsigned char mybyte) {
    unsigned char x, y;
    x = mybyte & 0x0F;
    x = x | 0x30;
    y = mybyte & 0xF0;
    y = y >> 4;
    y = y | 0x30;
    lcd_data(y);
    lcd_data(x);
}

void read_rtc_display() {
    lcd_cmd(0x85);
    bcdconv(hr);
    lcd_data(':');
    bcdconv(min);
    lcd_data(':');
    bcdconv(sec);
}

void update_display() {
    lcd_cmd(0xC0);
    lcd_string("Temp: ");
    lcd_int(temperatureSensor);
    lcd_data('C');

    lcd_cmd(0xD0);
    lcd_string("Heater: ");
    if (heater == 1)
        lcd_string("ON ");
    else
        lcd_string("OFF");

    lcd_cmd(0xD4);
    lcd_string("Light: ");
    if (redLight == 1)
        lcd_string("RED  ");
    else if (greenLight == 1)
        lcd_string("GREEN");
    else
        lcd_string("OFF  ");
}

void main() {
    TMOD = 0x20;    // Timer1 mode
    TH1 = 0xFD;     // Baud rate 9600
    SCON = 0x50;    // Serial mode 1, 8-bit data, enable reception
    TR1 = 1;        // Start Timer1

    lcd_init();
    lcd_cmd(0x80);
    lcd_string("TIME:");
    lcd_cmd(0xC0);
    lcd_string("TEMP:--C");
    lcd_cmd(0x94);
    lcd_string("HEATER:OFF");
    lcd_cmd(0xD4);
    lcd_string("LIGHT:OFF");

    while (1) {
        read_rtc_display();
        update_display();
        delay(200);
    }
}

