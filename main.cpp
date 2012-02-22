/*
 *2010 Weather station
 *Made by Nicholas Dart
 *
 *mbed microcontroller (http://mbed.org/)
 *All libs can be found on the mbed site
 *
 *licensed under the creative commons (CC BY-SA 3.0)
*/

#include "mbed.h"
#include "SDFileSystem.h"


//----------------------------------------



//the definiton of what a counter is, as well as how to increace and reset it
class Counter {
public:
    Counter(PinName pin) : _interrupt(pin) {        // create the InterruptIn on the pin specified to Counter
        _interrupt.rise(this, &Counter::increment); // attach increment function of this counter instance
    }

    void increment() {
        _count++;
    }

    int read() {
        return _count;
    }
    void reset() {
        _count = 0;
    }



private:
    InterruptIn _interrupt;
    volatile int _count;
};

//----------------------------------------

DigitalOut lde1(LED1);
AnalogIn In1 (p15);
Serial pc(USBTX, USBRX);
Timeout timeout;
Counter WSCounter(p14);             //14
Counter RFCounter(p13);             //13
DigitalOut led1(LED1);
AnalogIn Humid(p16);
AnalogIn Temp(p17);
LocalFileSystem local("local");
SDFileSystem sd(p5, p6, p7, p8, "sd");

float Voltage;


//----------------------------------------

void reset_RN_value() {
    RFCounter.reset();
}

//the buffer that will hold the text for writing to the .TXT document
char Buffer[1024];

//this defines what 'c' is, in this case the press of a key on the keybord. the value of c can be set EG.          if (c == 'y') { led1 = !led1 }
char c;

//Array to hold characters entered for the file name in write();
char FileName[16];

//Array to hold the option for Overwrite/Append/New
char choice [1];

FILE* fp;


//the Sub-Routine that will recieve the
void Wind_dir () {
    Voltage = In1;
//    pc.printf("Wind Direction ");
    printf(Buffer + strlen(Buffer), "Wind Direction ");
    //Voltage  from Vein is refined
    if ((In1 >= 0.9375)||(In1 < 0.0625)) {
//        pc.printf("North      at:");
        printf(Buffer + strlen(Buffer), "North      at:");
    }
    if ((In1 >= 0.0625)&&(In1 < 0.1875)) {
//        pc.printf("North East at:");
        printf(Buffer + strlen(Buffer), "North East at:");
    }
    if ((In1 >= 0.1875)&&(In1 < 0.3125)) {
//        pc.printf("East       at:");
        printf(Buffer + strlen(Buffer), "East       at:");
    }
    if ((In1 >= 0.341275)&&(In1 < 0.4375)) {
//        pc.printf("South East at:");
        printf(Buffer + strlen(Buffer), "South East at:");
    }
    if ((In1 >= 0.4375)&&(In1 < 0.5625)) {
//        pc.printf("South      at:");
        printf(Buffer + strlen(Buffer), "South      at:");
    }
    if ((In1 >= 0.5625)&&(In1 < 0.6875)) {
//        pc.printf("South West at:");
        printf(Buffer + strlen(Buffer), "South West at:");
    }
    if ((In1 >= 0.6875)&&(In1 < 0.8125)) {
//        pc.printf("West       at:");
        printf(Buffer + strlen(Buffer), "West       at:");
    }
    if ((In1 >= 0.8125)&&(In1 <= 0.9375)) {
//        pc.printf("North West at:");
        printf(Buffer + strlen(Buffer), "North West at:");
    }
}




//the Sub0routine that will count the pulses from the wind "fan" and desplay that (a reset of this every second will give you the value in Meters per second) pulses clocked on the rise
void WindSpeed() {
    //Meters per Second is calculated from wind speed pulses
//    pc.printf(" %03dm/S  ", WSCounter.read());
    printf(Buffer + strlen(Buffer),"  %03dm/S  ", WSCounter.read());
    //Wind Speed Counter is reset to 0
    WSCounter.reset();
}


//The Sub-routine that will count every pulse from the rain guage (the pulse being the fall of the bucket, and it being counted on the rise
void RainGauge() {
    float RainFall=0.0;
    RainFall = RFCounter.read() * 0.2;
//    pc.printf("Rain Fall per Hour: %03.1fmm ", RainFall);
    printf(Buffer + strlen(Buffer),"Rain Fall per Hour: %03.1fmm ",RainFall);
}





//the Sub-Routine that will recieve the infomation for the Humidity and change into a percentage
void Humidity () {
    float humidity = 0.0;
    humidity = ((Humid * 3.3) -1.60)/0.0312;
    //pc.printf("Humidity: ");
    printf(Buffer + strlen(Buffer),"Humidity: ");
    //pc.printf("%03.1f%% ", humidity);
    printf(Buffer + strlen(Buffer), "%04.1f%s",humidity,"%% ");
}




//The Sub-routine that will recieve the data for temperature and translate into a Degrees Celcius figure
void Temperature () {
    float Temperature1 = 0.0;
    Temperature1 = ((1000/Temp)-2005 )/4;
//    pc.printf("temperature is %02.0f°C   ", Temperature1);
    printf(Buffer + strlen(Buffer), "temperature is %02.0f°C   ", Temperature1);
}


//The Sub-Routine that will write the content of "Buffer" into a text file on the micro SD card connected to the m-bed
void Choice () {
    int N=4;
    pc.printf("\r\n\r\nInput file name : ");
    FileName[0] = '/';
    FileName[1] = 's';
    FileName[2] = 'd';
    FileName[3] = '/';
    do {
        FileName[N] = pc.getc();
        pc.printf("%c",FileName[N]);
    } while ((FileName[N++] == '\n') | (N < 11));
    pc.printf("\n\r%s\n\r", &FileName[0]);
    printf(&FileName[--N],".txt");
    pc.printf("\n\r%s\n\r", &FileName[0]);
    wait (0.5);
    //Does it Exist?? Yes go to options no - create - Y/N??  Y -> Create and continue N-> Go to do
    fp = fopen(&FileName[0], "r");
    if (!fp) {
        pc.printf("%s Doesn't Exist! Creating new file...\n\r", &FileName[0]);
        fp = fopen(&FileName[0], "w");
        if (!fp) {
            pc.printf("ERROR CREATING FILE!");
        }
    } else {
        fclose(fp);
        pc.printf("\r\n\r\n%s allready exists, do you want to: \n\r Append [A] \n\r Overwrite [O] \n\r or Create a new file [C]\n\r", &FileName[0]);
        do {
            c = pc.getc();
        } while ( !((c == 'A')||(c == 'a')||(c == 'O')||(c == 'o')) );
        if ((c == 'A')||(c == 'a')) {
            pc.printf("Appending %s", &FileName[0]);
            fp = fopen(&FileName[0], "a");
        }
        if ((c == 'O') || (c == 'o')) {
            pc.printf("Overwriting %s", &FileName[0]);
            fp = fopen(&FileName[0], "w");
        }
    }
    pc.printf("\n\rFile opened successfuly\n\r\n\r\n\r\n\r");
//    fprintf(fp, Buffer);
    fclose(fp);
}


void Write () {
    pc.printf(Buffer);
    fp = fopen(&FileName[0], "a");
    fprintf(fp, Buffer);
    Buffer[0] = 0;
    fclose(fp);
}

//----------------------------------------

int main() {
    Choice();
//    pc.printf("------------------------------------------------------------------------------------------------\r\n\r\n\r\n\r\n\r\n\r\n ----- M-BED Weather Monitoring -----\r\n\r\n");
    printf(Buffer + strlen(Buffer), "------------------------------------------------------------------------------------------------\r\n\r\n\r\n\r\n\r\n\r\n ----- M-BED Weather Monitoring -----\r\n\r\n");
    while (1) {
        Wind_dir();
        WindSpeed();
        RainGauge();
        Humidity();
        Temperature();
//        pc.printf("\r\n-----\r\n");
        printf(Buffer + strlen(Buffer), "\r\n-----\r\n");
        // timeout.attach(&reset_RN_value, 3600.0);
        Write();
        wait(1.0);
    }
}
//----------------------------------------

