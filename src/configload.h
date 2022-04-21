

// Loads the configuration from a file
void loadConfiguration(const char *filename, Config &config) {
  // Open file for reading
  File file = SPIFFS.open(filename);
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

    strlcpy(config.cartname,                  // <- destination
            doc["cartname"] | "TarantlCart",  // <- source
            sizeof(config.cartname));         // <- destination's capacity

    strlcpy(config.ssid,                    // <- destination
        doc["ssid"] | "TarantlVR",          // <- source
        sizeof(config.ssid));               // <- destination's capacity

    strlcpy(config.passwd,                    // <- destination
        doc["passwd"] | "chillfumml",          // <- source
        sizeof(config.passwd));               // <- destination's capacity

  JsonObject speedprofile_0 = doc["speedprofile"][0];

  config.speed_max = speedprofile_0["speed_max"] | -100;
  config.speed_min = speedprofile_0["speed_min"] | 100;
  config.steer_max = speedprofile_0["steer_max"] | -100;
  config.steer_min = speedprofile_0["steer_min"] | 100;
  config.accel_min = speedprofile_0["accel_min"] | 200;
  config.boost_max = speedprofile_0["boost"] | 100;



  file.close();
}

// Prints the content of a file to the Serial
void printFile(const char *filename) {
  // Open file for reading
  File file = SPIFFS.open(filename);
  if (!file) {
    Serial.println(F("Failed to read file"));
    return;
  }

  // Extract each characters by one by one
  while (file.available()) {
    Serial.print((char)file.read());
  }
  Serial.println();

  // Close the file
  file.close();
}