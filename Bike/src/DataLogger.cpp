#include "DataLogger.h"
#include "Config.h"

bool DataLogger::begin(int csPin) {
    if (!SD.begin(csPin)) {
        Serial.println("Card mount failed");
        return false;
    }
    if (SD.cardType() == CARD_NONE) {
        Serial.println("No SD card attached");
        return false;
    }
    return true;
}

void DataLogger::log(const String& data) {
    File file = SD.open(DATA_FILE, FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    file.println(data);
    file.close();
    Serial.println("Logged: " + data);
}

bool DataLogger::readAllAndUpload(std::function<bool(const String&)> uploader) {
    File file = SD.open(DATA_FILE);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return false;
    }

    bool allUploaded = true;
    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (!line.isEmpty()) {
            bool success = uploader(line);
            if (!success) allUploaded = false;
        }
    }
    file.close();

    if (allUploaded) {
        if (SD.remove(DATA_FILE)) {
            Serial.println("All data uploaded, file deleted");
        } else {
            Serial.println("Failed to delete log file");
        }
    }
    return allUploaded;
}
