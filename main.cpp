//#include "mbed_config.h" // CHECK!
#include "mbed.h"
#include "math.h"

// standard settings as in TARGET_L476_L486/PinNames.h
//#define SPI_MOSI PA_7
//#define SPI_MISO PA_6
//#define SPI_CS   PB_6
//#define SPI_SCK  PA_5
#define SPI_SS   SPI_CS
// IRQ of ADIS DIO (DIO1 for ADIS 16485)
#define ADIS_DIO PB_11
// RST (reset) for ADIS
#define ADIS_RST PA_12

// IMU
int IMUClockCounter = 0; // counter for IMU samples per sec
//#include "ADIS1648x.h" // ADIS1648x interface
#include "LogUtil/LogUtil.h"
#include "ADIS16488/Adis16488.h"

// 
DigitalOut led(LED2);    // signal LED

// 
Serial pc(USBTX, USBRX, 115200); // tx, rx
LogUtil logger(pc);

inline void blinkPause(DigitalOut &led, double freq, double duration)
{
    unsigned int n = (int)floor(freq*duration);
    double wtime = 0.5 / freq;

    for (unsigned int i=0; i<n; i++)
    {
        led = 1;
        wait(wtime);
        led = 0;
        wait(wtime);
    }
}

int main() {

    pc.printf("======================================================\n");
    //pc.printf("Press any key to start...\n");
    //pc.getc(); // wait for keyboard

    //set up the SPI on pins 5, 6, 7 to read from the ADIS16488
    SPI spi(SPI_MOSI, SPI_MISO, SPI_SCK); // mosi (DIN), miso (DOUT), sclk (CLK) //, ssel (CS)
    spi.format(16, 3); // TODO: why is _spi->format call in class not working?
    DigitalOut cs(SPI_SS);     // Chip/Slave Select for the ADIS SPI
    InterruptIn dr(ADIS_DIO);   // DataReady interrupt connected to DIO for ADIS
    DigitalOut rst(ADIS_RST);   // ADIS reset pin

    Adis16488 imu(spi, cs, rst, dr);
    imu.enable();
    pc.printf("\n### IMU enabled. ###\n");

    // show signal (and wait)
    blinkPause(led, 4.0, 2.0);

    // TODO: provide generic function
    float acc_fact_out = 0.00025f;
    while(true) {
      // write acc
      for(uint8_t i=0; i<3; ++i) {
        // _LOW bits
        float acc_low = 0.5f * acc_fact_out; // factor for ACCL_LOW MSB
        float out_low = 0.0f;
        // reading from MSB to LSB
        for (uint16_t mask=1<<(16-1); mask!=0; mask>>=1) {
          if (imu.accel.data[2*i] & mask) {
            out_low += acc_low;
          }
          acc_low *= 0.5f; // halved every bit
        }
        //pc.printf("%f\t", out_low);
        // _OUT (signed Int16!)
        float out = acc_fact_out * (int16_t)imu.accel.data[2*i+1];
        //pc.printf("%f\t", out);
        // TODO: Check if indeed signed addition of _LOW values is correct
        if (out<0.0f){
          out -= out_low;
        } else {
          out += out_low;
        }
        pc.printf("%f", out);
        if (i<2) {
          pc.printf(";");
        }
      }
      pc.printf("\n");
      wait_ms(20);
    }

}
