#include "Uploader.h"

bool Uploader::upload(const String& url) {
    HTTPClient http;
    http.begin(url);
    int code = http.GET();
    if (code > 0) {
        Serial.printf("Uploaded: %s [code=%d]\n", url.c_str(), code);
        http.end();
        return true;
    } else {
        Serial.printf("Upload failed: %s [code=%d]\n", url.c_str(), code);
        http.end();
        return false;
    }
}
