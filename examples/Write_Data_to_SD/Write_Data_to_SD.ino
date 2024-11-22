#include <SD.h>
#include <HX711_ADC.h>
#include <math.h>

// pins for load cell:
const int HX711_dout = 8; //mcu > HX711 dout pin
const int HX711_sck = 9; //mcu > HX711 sck pin

// HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

// For SD card & filename
const int chipSelect = 10;
File myFile;
const String csv_filename = "data_10.csv";

// Values to alternate between
int values[] = {1, 2, 3};
int currentIndex = 0;

// Load cell calibration factor found using the Calibrate.ino example
const float CALIBRATION_VALUE = 82.0;
unsigned long t = 0;

void setup() {
  // Start Serial communication
  Serial.begin(9600);
  // Wait for the Serial Monitor to connect (for native USB boards)
  while (!Serial);

  // ALL LOAD SELL SETIP
  LoadCell.begin();
  //LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive
  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag() || LoadCell.getSignalTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (true);
  }
  else {
    LoadCell.setCalFactor(CALIBRATION_VALUE); // user set calibration value (float), initial value 1.0 may be used for this sketch
    Serial.println("Startup is complete");
  }
  while (!LoadCell.update());

  // ALL SD CARD SETUP
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("Initialization failed. Please check the SD card and try again.");
    while (true); // Stop execution if SD initialization fails
  }
  Serial.println("SD card initialized.");

  // Open the file in append mode
  myFile = SD.open(csv_filename, FILE_WRITE);
  if (!myFile) {
    myFile = SD.open(csv_filename, O_CREAT);
    while(!myFile) {
      if (millis() > 10000) break;
    }
    if (!myFile) {
      Serial.println("Error opening CSV file... please check the filename and try again");
      while (true); // Stop if file opening fails
    } 
  }
  Serial.println("CSV file successfully opened.");
  Serial.println("LOAD CELL LOGGING STARTED");
}

void loop() {
  static boolean newDataReady = 0;
  const int serialPrintInterval = 0; // increase value to slow down serial print 

  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      int loadcell_val = round(LoadCell.getData());
      // Write alternating values to the CSV file continuously
      if (myFile) {
        currentIndex = currentIndex + 1;
        // Write the current milisecond value followed by the load cell value
        myFile.print(millis());
        myFile.print(",");
        myFile.println(loadcell_val);

        // Flush the file buffer to ensure data is written to the SD card
        myFile.flush();

        // LOGGING
        // Serial.print("Printed line ");
        // Serial.println(currentIndex);

        // DELAY
        // delay(5); // 1-second delay between writes
      } else {
        // If the file isn't open, try reopening it
        myFile = SD.open(csv_filename, FILE_WRITE);
        if (!myFile) {
          Serial.println("Error reopening CSV file");
          while (true); // Stop if file opening fails again
        }
      }
    }
  }  
}