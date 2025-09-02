#pragma once
#include <FS.h>
#include <SD.h>

class DataLogger {
public:
    bool begin(int csPin);
    void log(const String& data);
    bool readAllAndUpload(std::function<bool(const String&)> uploader);
};
