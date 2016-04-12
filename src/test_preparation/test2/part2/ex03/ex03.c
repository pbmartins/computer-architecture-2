#include <detpic32.h>
#define CALIBRATION_VALUE 4983
#define N_CONVERTIONS 4 

static unsigned char displayFlag = 0, value2displays;
static const unsigned char display7Scodes[] = {
    0b01110111,
    0b01000001,
    0b00111011,
    0b01101011,
    0b01001101,
    0b01101110,
    0b01111110,
    0b01000011,
    0b01111111,
    0b01001111,
    0b01011111,
    0b01111100,
    0b00110110,
    0b01111001,
    0b00111110,
    0b00011110,
    0b00000000
};

void initADC(void) {
    TRISBbits.TRISB14 = 1; // RB14 digital output disconnected
    AD1PCFGbits.PCFG14 = 0; // RB14 configured as analog input (AN14)

    AD1CHSbits.CH0SA = 14;
    AD1CON1bits.SSRC = 7;
    AD1CON1bits.CLRASAM = 1;
    AD1CON2bits.SMPI = N_CONVERTIONS-1;
    AD1CON1bits.ON = 1;

    // Configure interrupts
    IFS1bits.AD1IF = 0;
    IPC6bits.AD1IP = 6;
    IEC1bits.AD1IE = 1;
}

unsigned int convert2celsius(int value) {
    return value * 50 / 1023;
}

unsigned char toBcd(unsigned char value) {
    return ((value / 10) << 4) + (value % 10);
}

void send2displays(unsigned char value) {
    unsigned char digit_low, digit_high;
    digit_low = value & 0x0F;
    digit_high = value >> 4;

    if (!displayFlag)
        LATB = (LATB & 0xFC00) | display7Scodes[digit_low] | 0x0200;
    else
        LATB = (LATB & 0xFC00) | display7Scodes[digit_high] | 0x0100;
    
    displayFlag = !displayFlag;
}

void delay(unsigned int n_intervals) {
    volatile unsigned int i;
    for(; n_intervals != 0; n_intervals--)
        for(i = CALIBRATION_VALUE; i != 0; i--);
}

int average(int* p) {
    int sum = 0, i;
    for (i = 0; i < N_CONVERTIONS; i++)
        sum += p[i<<2];
    return sum / N_CONVERTIONS;
}

int main(void) {
    unsigned int i = 0;
    
    initADC();
    EnableInterrupts();

    // Configure displays as outputs
    TRISB = TRISB & 0xFC00;
    
    while (1) {
        delay(10);
        if (i++ == 2) {
            AD1CON1bits.ASAM = 1;
            i = 0;
        }
        send2displays(toBcd(value2displays));
    }    

    return 0;
}

void _int_(27) isr_adc(void) {
    int* p = (int*)(&ADC1BUF0);
    value2displays = convert2celsius(average(p));
    IFS1bits.AD1IF = 0;
}
