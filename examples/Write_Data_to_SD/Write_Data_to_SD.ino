#include <SD.h>
#include <HX711_ADC.h>
#include <math.h>

// file init
File csvLogFile;
// pins for load cell
const int HX711_dout = 8; // mcu > HX711 dout pin
const int HX711_sck = 9; // mcu > HX711 sck pin
HX711_ADC LoadCell(HX711_dout, HX711_sck); // constructor
// Number of seconds to log to SD for
const int logging_seconds = 10;
// Number of miliseconds between CSV writes
const int csv_write_delay = 0;
// Load cell calibration factor found using the Calibrate.ino example
const float CALIBRATION_VALUE = LoadCell.getCal_LoadCellA();
// Filename for SD logging
const String csv_filename = "data_11.csv";

// SD chip select pin
const int chipSelect = 10;
// various SD card logging variables
int currentIndex = 0;
unsigned long t = 0;
unsigned long timestamp = 0;
bool loggingEnd = 0;

void setup() {
  // Start Serial communication
  Serial.begin(9600);
  // Wait for the Serial Monitor to connect (for native USB boards)
  while (!Serial);

  Serial.println("Beginning Load Cell startup...");
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
    Serial.println("Load Cell startup complete.");
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
  csvLogFile = SD.open(csv_filename, FILE_WRITE);
  if (!csvLogFile) {
    csvLogFile = SD.open(csv_filename, O_CREAT);
    while(!csvLogFile) {
      if (millis() > 10000) break;
    }
    if (!csvLogFile) {
      Serial.println("Error opening CSV file. Please check the filename and try again");
      while (true); // Stop if file opening fails
    } 
  }

  // Print successful start message
  Serial.println("CSV file successfully opened.");
  Serial.println("LOAD CELL LOGGING STARTED");
  
  // Initialize the timestamp
  timestamp = millis();
  timestamp = timestamp + (logging_seconds * 1000);
}

void loop() {
  // If 10 seconds have passed, end the logging
  if (millis() > timestamp) {
    loggingEnd = 1;
    Serial.println("Ended Logging");
  }

  // If the logging has been ended, close the file and stop the program
  if (loggingEnd) {
    csvLogFile.close();
    Serial.println("CSV File Closed");
    while (true);
  }

  static boolean newDataReady = 0; // used to identify that new data is ready for retrieval

  // Check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // Get smoothed value from the dataset:
  if (newDataReady) {
    // If there is a write delay, check to make sure that amount of time has passed
    if (millis() > t + csv_write_delay) {
      // Round the load cell value to an integer (this will be in grams)
      int loadcell_val = round(LoadCell.getData());
      // Write alternating values to the CSV file continuously
      if (csvLogFile) {
        currentIndex = currentIndex + 1;
        // Write the current milisecond value followed by the load cell value
        csvLogFile.print(millis());
        csvLogFile.print(",");
        csvLogFile.println(loadcell_val);

        // Flush the file buffer to ensure data is written to the SD card
        csvLogFile.flush();

        // LOGGING (if required)
        // Serial.print("Printed line ");
        // Serial.println(currentIndex);
      } else {
        // If the file isn't open, try reopening it
        csvLogFile = SD.open(csv_filename, FILE_WRITE);
        delay(100);
        if (!csvLogFile) {
          Serial.println("Error reopening CSV file. Please check the SD card or file and try again.");
          while (true); // Stop if file opening fails again
        }
      }
    }
  }  
}