/*
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * Example that shows how to parse a P1 message and automatically print
 * the result.
*/

#include "dsmr.h"

// Data to parse
const char msg[] =
  "/NWB-WARMTELINK\r\n"
  "\r\n"
  "1-3:0.2.8(50)\r\n"
  "0-0:1.0.0(220208231051W)\r\n"
  "0-0:96.1.1(ADB3100000043021)\r\n"
  "0-0:96.13.1()\r\n"
  "0-0:96.13.0()\r\n"
  "0-1:24.1.0(004)\r\n"
  "0-1:96.1.0(717285092D2C340C)\r\n"
  "0-1:24.2.1(220208231051W)(46.720*GJ)\r\n"
  "!4FAC\r\n";

/**
 * Define the data we're interested in, as well as the datastructure to
 * hold the parsed data.
 * Each template argument below results in a field of the same name.
 */
using MyData = ParsedData<
  /* String */ identification,
  /* String */ p1_version,
  /* String */ timestamp,
  /* String */ equipment_id,
  /* String */ message_short,
  /* String */ message_long,
  /* uint16_t */ thermal_device_type,
  /* String */ thermal_equipment_id,
  /* FixedValue */ thermal_delivered
>;

void setup() {
  Serial.begin(115200);

  MyData data;
  ParseResult<void> res = P1Parser::parse(&data, msg, lengthof(msg));
  if (res.err) {
    // Parsing error, show it
    Serial.println(res.fullError(msg, msg + lengthof(msg)));
  } else if (!data.all_present()) {
    Serial.println("Some fields are missing");
  } else {
    // Succesfully parsed, print results:
    Serial.println(data.identification);
    Serial.print(data.thermal_delivered.int_val());
    Serial.println("GJ");
  }
}

void loop () {
}
