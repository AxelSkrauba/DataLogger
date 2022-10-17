#include <SparkFun_ADXL345.h>         // SparkFun ADXL345 Library
#include <PinChangeInterrupt.h>

# define DIM 200
/*********** COMMUNICATION SELECTION ***********/
/*    Comment Out The One You Are Not Using    */
ADXL345 adxl = ADXL345();           // USE FOR SPI COMMUNICATION, ADXL345(CS_PIN);
//ADXL345 adxl = ADXL345();             // USE FOR I2C COMMUNICATION

// Encoder
const byte encoderPinA = 2;
const byte encoderPinB = 3;

/*
#define readA bitRead(PIND,encoderPinA)//faster than digitalRead()
#define readB bitRead(PIND,encoderPinB)//faster than digitalRead()
*/

#define readA digitalRead(encoderPinA)
#define readB digitalRead(encoderPinB)

volatile long int count_RPM = 0; //depending on count rate, you may be able to use int
long int copyCount_RPM = 0;



/****************** INTERRUPT ******************/
/*      Uncomment If Attaching Interrupt       */
//const byte interruptPin = 18;                 // Setup pin 2 to be the interrupt pin (for most Arduino Boards)

int v_x[DIM];
int v_y[DIM];
int v_z[DIM];
int rpm[DIM];
int v_x2[DIM];
int v_y2[DIM];
int v_z2[DIM];
int rpm_2[DIM];

bool send_info = false;
int vec = 1;
int index = 0;

long int check = 0; // Para el CHECKSUM

/******************** SETUP ********************/
/*          Configure ADXL345 Settings         */
void setup(){
  
  Serial.begin(500000);                 // Start the serial terminal
  //Serial.println("Test ADXL345 Accelerometer");
  //Serial.println();
  
  adxl.powerOn();                     // Power on the ADXL345

  adxl.setRangeSetting(2);           // Give the range settings
                                      // Accepted values are 2g, 4g, 8g or 16g
                                      // Higher Values = Wider Measurement Range
                                      // Lower Values = Greater Sensitivity

  adxl.setSpiBit(0);                  // Configure the device to be in 4 wire SPI mode when set to '0' or 3 wire SPI mode when set to 1
                                      // Default: Set to 1
                                      // SPI pins on the ATMega328: 11, 12 and 13 as reference in SPI Library 
   
  // Frecuencia Muestreo
  adxl.setRate(1200);

  Serial.print("Fs:");
  Serial.print(adxl.getRate());
  Serial.println();

  // Ganancias
  double gains[3];
  adxl.getAxisGains(&gains[0]);
  Serial.print("G:");
  Serial.print(gains[0]);
  Serial.print(",");
  Serial.print(gains[1]);
  Serial.print(",");
  Serial.print(gains[2]);
  Serial.println();
 
  //I had issues with int pin 2, was unable to reset it
  adxl.setInterruptMapping(ADXL345_INT_DATA_READY_BIT, ADXL345_INT2_PIN);

  //register interupt actions - 1 == on; 0 == off
  adxl.setInterrupt(ADXL345_INT_DATA_READY_BIT, 1);
  
  
  //pinMode(interruptPin, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(interruptPin), isrAccel, RISING);   // Attach Interrupt

  //Encoder
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderPinA), isrA, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(encoderPinB), isrB, CHANGE);
  //attachPCINT(digitalPinToPCINT(encoderPinA), isrA, CHANGE);
  //attachPCINT(digitalPinToPCINT(encoderPinB), isrB, CHANGE);

}

/****************** MAIN CODE ******************/
/*     Accelerometer Readings and Interrupt    */
void loop(){
  
  // Accelerometer Readings
  
  
  ADXL_ISR();
  // You may also choose to avoid using interrupts and simply run the functions within ADXL_ISR(); 
  //  and place it within the loop instead.  
  // This may come in handy when it doesn't matter when the action occurs.

  if (send_info){
    if (vec == 2){
      serialize_vec(1, &v_x[0]);
      serialize_vec(2, &v_y[0]);
      serialize_vec(3, &v_z[0]);
      serialize_vec(4, &rpm[0]);
      
    }
    else{
      serialize_vec(1, &v_x2[0]);
      serialize_vec(2, &v_y2[0]);
      serialize_vec(3, &v_z2[0]);
      serialize_vec(4, &rpm_2[0]);
    }
    send_info = false;
  }

}

void serialize_vec(int vec_n, int* vec){
  switch(vec_n){
        case 1:
            Serial.print("X:"); 
            break;
        case 2:
            Serial.print("Y:"); 
            break;
        case 3:
            Serial.print("Z:"); 
            break;
        case 4:
            Serial.print("RPM:"); 
            break;
        default:
            Serial.print("?:"); 
            break;
    }
  check = 0;
  for(int i=0; i<DIM-1; i++){
    Serial.print(vec[i]);
    Serial.print(",");
    check += vec[i];
  }
  Serial.print(vec[DIM-1]);
  check += vec[DIM-1];
  
  Serial.print(":");
  Serial.print(DIM);
  Serial.print(":");
  Serial.println(check);
}
/********************* ISR *********************/
/* Look for Interrupts and Triggered Action    */
void ADXL_ISR() {
  
  // getInterruptSource clears all triggered actions after returning value
  // Do not call again until you need to recheck for triggered actions
  byte interrupts = adxl.getInterruptSource();
  
  // Data Ready Detection
  if(adxl.triggered(interrupts, ADXL345_DATA_READY)){
    noInterrupts();
    //copyCount_RPM = count_RPM/600.00*60; // Encoder de 600 pulsos por revolucion
    copyCount_RPM = count_RPM;
    count_RPM = 0;
    interrupts();
    
    
    //Serial.println("*** Data Ready ***");
    int x,y,z;   
    adxl.readAccel(&x, &y, &z);         // Read the accelerometer values and store them in variables declared above x,y,z
  
    if (vec==1){
      v_x[index] = x;
      v_y[index] = y;
      v_z[index] = z;
      rpm[index] = copyCount_RPM;
      index++;
      if (index==DIM){
        vec = 2;
        index = 0;
        send_info = true;
      }
    }
    else{
      v_x2[index] = x;
      v_y2[index] = y;
      v_z2[index] = z;
      rpm_2[index] = copyCount_RPM;
      index++;
      if (index==DIM){
        vec = 1;
        index = 0;
        send_info = true;
      }
    }
    
    // Output Results to Serial
    /* UNCOMMENT TO VIEW X Y Z ACCELEROMETER VALUES */
    /* 
    Serial.print(x);
    Serial.print(", ");
    Serial.print(y);
    Serial.print(", ");
    Serial.println(z);
    */
    
    //add code here to do when free fall is sensed
  } 
  
}


void isrA() {
  if (readB != readA) {
    count_RPM ++;
  } else {
    count_RPM --;
  }
}
void isrB() {
  if (readA == readB) {
    count_RPM ++;
  } else {
    count_RPM --;
  }
}
