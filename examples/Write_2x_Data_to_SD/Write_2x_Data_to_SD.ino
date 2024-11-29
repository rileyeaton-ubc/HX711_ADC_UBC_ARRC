#include <SD.h>
#include <HX711_ADC_UBC_ARRC.h>
#include <math.h>

// file init
File csvLogFile;
// pins for load cells
const int HX711_dout_1 = 6; // mcu > HX711 no 1 dout pin
const int HX711_sck_1 = 7; // mcu > HX711 no 1 sck pin
const int HX711_dout_2 = 8; // mcu > HX711 no 2 dout pin
const int HX711_sck_2 = 9; // mcu > HX711 no 2 sck pin
HX711_ADC LoadCell_1(HX711_dout_1, HX711_sck_1); // HX711 1
HX711_ADC LoadCell_2(HX711_dout_2, HX711_sck_2); // HX711 2
// Number of seconds to log to SD for
const int logging_seconds = 10;
// Number of miliseconds between CSV writes
const int csv_write_delay = 0;
// Load cell calibration factors found using the Calibrate.ino example
const float CALIBRATION_VALUE_1 = LoadCell_1.getCal_LoadCellA();
const float CALIBRATION_VALUE_2 = LoadCell_2.getCal_LoadCellB();
// Filename for SD logging
const String csv_filename = "data_15.csv";

// SD chip select pin
const int chipSelect = 10;
// various SD card logging variables
int currentIndex = 0;
unsigned long t = 0;
unsigned long timestamp = 0;
bool loggingEnd = 0;

void setup() {
  // Start Serial communication
  Serial.begin(115200);
  // Wait for the Serial Monitor to connect (for native USB boards)
  while (!Serial);

  Serial.println("Beginning Load Cell startup...");
  int loadcell_fail = 0;
  // ALL LOAD SELL SETIP
  LoadCell_1.begin();
  LoadCell_2.begin();
  //LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive
  unsigned long stabilizingtime = 2000; // tare preciscion can be improved by adding a few seconds of stabilizing time
  bool _tare = true; //set this to false if you don't want tare to be performed in the next step
  byte loadcell_1_rdy = 0;
  byte loadcell_2_rdy = 0;
  while ((loadcell_1_rdy + loadcell_2_rdy) < 2) { //run startup, stabilization and tare, both modules simultaniously
    if (!loadcell_1_rdy) loadcell_1_rdy = LoadCell_1.startMultiple(stabilizingtime, _tare);
    if (!loadcell_2_rdy) loadcell_2_rdy = LoadCell_2.startMultiple(stabilizingtime, _tare);
  }
  if (LoadCell_1.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 no.1 wiring and pin designations");
    loadcell_fail = 1;
  }
  if (LoadCell_2.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 no.2 wiring and pin designations");
    loadcell_fail = 1;
  }

  // If there was a load cell failure, stop the program. Otherwise, set the calibration values
  if (loadcell_fail) {
    while (true);
  } else {
    LoadCell_1.setCalFactor(CALIBRATION_VALUE_1); // user set calibration value (float)
    LoadCell_2.setCalFactor(CALIBRATION_VALUE_2); // user set calibration value (float)
    while (!LoadCell_1.update());
    while (!LoadCell_2.update());
    Serial.println("Load Cell startup complete.");
  }

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
    while(!csvLogFile) {
      csvLogFile = SD.open(csv_filename, O_CREAT);
      delay(1000);
      if (millis() > 10000) break;
    }
    if (!csvLogFile) {
      Serial.println("Error opening CSV file. Please check the filename and try again");
      while (true); // Stop if file opening fails
    }
  }

  // Write headers to the CSV file
  csvLogFile.println("Time (ms), Load Cell 1 Weight (g), Load Cell 2 Weight (g)");
  csvLogFile.flush();

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

  static bool newDataReady = 0; // used to identify that new data is ready for retrieval

  // Check for new data/start next conversion:
  if (LoadCell_1.update()) newDataReady = true;
  LoadCell_2.update();

  // Get smoothed values from the dataset:
  if (newDataReady) {
    // If there is a write delay, check to make sure that amount of time has passed
    if (millis() > t + csv_write_delay) {
      // Round the load cell values to an integer (this will be in grams)
      int loadcell_1_val = round(LoadCell_1.getData());
      int loadcell_2_val = round(LoadCell_2.getData());
      // Write alternating values to the CSV file continuously
      if (csvLogFile) {
        currentIndex = currentIndex + 1;
        // Write the current milisecond value followed by the load cell value
        csvLogFile.print(millis());
        csvLogFile.print(",");
        csvLogFile.print(loadcell_1_val);
        csvLogFile.print(",");
        csvLogFile.println(loadcell_2_val);

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