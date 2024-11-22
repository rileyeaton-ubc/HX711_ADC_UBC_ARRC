#include <SD.h>

const int chipSelect = 4;
File myFile;

// Values to alternate between
int values[] = {1, 2, 3};
int currentIndex = 0;

void setup() {
  // Start Serial communication
  Serial.begin(9600);
  // Wait for the Serial Monitor to connect (for native USB boards)
  while (!Serial);

  Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelect)) {
    Serial.println("Initialization failed. Please check the SD card and try again.");
    while (true); // Stop execution if SD initialization fails
  }

  Serial.println("SD card initialized.");

  // Open the file in append mode
  myFile = SD.open("data69.csv", FILE_WRITE);
  if (!myFile) {
    Serial.println("Error opening data.csv");
    while (true); // Stop if file opening fails
  }
}

void loop() {
  currentIndex = currentIndex + 1;
  // Write alternating values to the CSV file continuously
  if (myFile) {
    // Write the current value followed by a comma
    myFile.println("1,2,3");

    // Flush the file buffer to ensure data is written to the SD card
    myFile.flush();

    // Optional: Print to Serial for monitoring
    Serial.print("Printed line ");
    Serial.println(currentIndex);

    // Add a delay for readability (adjust as needed)
    delay(1000); // 1-second delay between writes
  } else {
    // If the file isn't open, try reopening it
    myFile = SD.open("data.csv", FILE_WRITE);
    if (!myFile) {
      Serial.println("Error reopening data.csv");
      while (true); // Stop if file opening fails again
    }
  }
}
