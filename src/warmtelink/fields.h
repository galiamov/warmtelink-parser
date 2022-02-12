/**
 * Arduino Warmtelink parser.
 *
 * This software is licensed under the MIT License.
 *
 * Copyright (c) 2015 Matthijs Kooijman <matthijs@stdin.nl>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Field parsing functions
 */

#pragma once

#include "util.h"
#include "parser.h"

#ifndef WARMTELINK_THERMAL_MBUS_ID
#define WARMTELINK_THERMAL_MBUS_ID 1
#endif

namespace warmtelink
{

  /**
 * Superclass for data items in a P1 message.
 */
  template <typename T>
  struct ParsedField
  {
    template <typename F>
    void apply(F &f) { f.apply(*static_cast<T *>(this)); }
    // By defaults, fields have no unit
    static const char *unit() { return ""; }
  };

  template <typename T, size_t minlen, size_t maxlen>
  struct StringField : ParsedField<T>
  {
    ParseResult<void> parse(const char *str, const char *end)
    {
      ParseResult<String> res = StringParser::parse_string(minlen, maxlen, str, end);
      if (!res.err)
        static_cast<T *>(this)->val() = res.result;
      return res;
    }
  };

  // A timestamp is essentially a string using YYMMDDhhmmssX format (where
  // X is W or S for wintertime or summertime). Parsing this into a proper
  // (UNIX) timestamp is hard to do generically. Parsing it into a
  // single integer needs > 4 bytes top fit and isn't very useful (you
  // cannot really do any calculation with those values). So we just parse
  // into a string for now.
  template <typename T>
  struct TimestampField : StringField<T, 13, 13>
  {
  };

  // Value that is parsed as a three-decimal float, but stored as an
  // integer (by multiplying by 1000). Supports val() (or implicit cast to
  // float) to get the original value, and int_val() to get the more
  // efficient integer value. The unit() and int_unit() methods on
  // FixedField return the corresponding units for these values.
  struct FixedValue
  {
    operator float() { return val(); }
    float val() { return _value / 1000.0; }
    uint32_t int_val() { return _value; }

    uint32_t _value;
  };

  // Floating point numbers in the message never have more than 3 decimal
  // digits. To prevent inefficient floating point operations, we store
  // them as a fixed-point number: an integer that stores the value in
  // thousands. For example, a value of 1.234 kWh is stored as 1234. This
  // effectively means that the integer value is het value in Wh. To allow
  // automatic printing of these values, both the original unit and the
  // integer unit is passed as a template argument.
  template <typename T, const char *_unit, const char *_int_unit>
  struct FixedField : ParsedField<T>
  {
    ParseResult<void> parse(const char *str, const char *end)
    {
      ParseResult<uint32_t> res = NumParser::parse(3, _unit, str, end);
      if (!res.err)
        static_cast<T *>(this)->val()._value = res.result;
      return res;
    }

    static const char *unit() { return _unit; }
    static const char *int_unit() { return _int_unit; }
  };

  struct TimestampedFixedValue : public FixedValue
  {
    String timestamp;
  };

  // Some numerical values are prefixed with a timestamp. This is simply
  // both of them concatenated, e.g. 0-1:24.2.1(150117180000W)(00473.789*m3)
  template <typename T, const char *_unit, const char *_int_unit>
  struct TimestampedFixedField : public FixedField<T, _unit, _int_unit>
  {
    ParseResult<void> parse(const char *str, const char *end)
    {
      // First, parse timestamp
      ParseResult<String> res = StringParser::parse_string(13, 13, str, end);
      if (res.err)
        return res;

      static_cast<T *>(this)->val().timestamp = res.result;

      // Which is immediately followed by the numerical value
      return FixedField<T, _unit, _int_unit>::parse(res.next, end);
    }
  };

  // A integer number is just represented as an integer.
  template <typename T, const char *_unit>
  struct IntField : ParsedField<T>
  {
    ParseResult<void> parse(const char *str, const char *end)
    {
      ParseResult<uint32_t> res = NumParser::parse(0, _unit, str, end);
      if (!res.err)
        static_cast<T *>(this)->val() = res.result;
      return res;
    }

    static const char *unit() { return _unit; }
  };

  // A RawField is not parsed, the entire value (including any
  // parenthesis around it) is returned as a string.
  template <typename T>
  struct RawField : ParsedField<T>
  {
    ParseResult<void> parse(const char *str, const char *end)
    {
      // Just copy the string verbatim value without any parsing
      concat_hack(static_cast<T *>(this)->val(), str, end - str);
      return ParseResult<void>().until(end);
    }
  };

  namespace fields
  {

    struct units
    {
      // These variables are inside a struct, since that allows us to make
      // them constexpr and define their values here, but define the storage
      // in a cpp file. Global const(expr) variables have implicitly
      // internal linkage, meaning each cpp file that includes us will have
      // its own copy of the variable. Since we take the address of these
      // variables (passing it as a template argument), this would cause a
      // compiler warning. By putting these in a struct, this is prevented.
      static constexpr char none[] = "";
      static constexpr char GJ[] = "GJ";
      static constexpr char MJ[] = "MJ";
    };

    const uint8_t THERMAL_MBUS_ID = WARMTELINK_THERMAL_MBUS_ID;

#define DEFINE_FIELD(fieldname, value_t, obis, field_t, field_args...) \
  struct fieldname : field_t<fieldname, ##field_args>                  \
  {                                                                    \
    value_t fieldname;                                                 \
    bool fieldname##_present = false;                                  \
    static constexpr ObisId id = obis;                                 \
    static constexpr char name[] = #fieldname;                         \
    value_t &val() { return fieldname; }                               \
    bool &present() { return fieldname##_present; }                    \
  }

    /* Meter identification. This is not a normal field, but a
    * specially-formatted first line of the message */
    DEFINE_FIELD(identification, String, ObisId(255, 255, 255, 255, 255, 255), RawField);

    /* Version information for P1 output */
    DEFINE_FIELD(p1_version, String, ObisId(1, 3, 0, 2, 8), StringField, 2, 2);

    /* Date-time stamp of the P1 message */
    DEFINE_FIELD(timestamp, String, ObisId(0, 0, 1, 0, 0), TimestampField);

    /* Equipment identifier */
    DEFINE_FIELD(equipment_id, String, ObisId(0, 0, 96, 1, 1), StringField, 0, 96);

    /* Text message codes: numeric 8 digits (Note: Missing from 5.0 spec) */
    DEFINE_FIELD(message_short, String, ObisId(0, 0, 96, 13, 1), StringField, 0, 16);

    /* Text message max 2048 characters (Note: Spec says 1024 in comment and
    * 2048 in format spec, so we stick to 2048). */
    DEFINE_FIELD(message_long, String, ObisId(0, 0, 96, 13, 0), StringField, 0, 2048);

    /* Device-Type */
    DEFINE_FIELD(thermal_device_type, uint16_t, ObisId(0, THERMAL_MBUS_ID, 24, 1, 0), IntField, units::none);

    /* Equipment identifier (Thermal: heat or cold) */
    DEFINE_FIELD(thermal_equipment_id, String, ObisId(0, THERMAL_MBUS_ID, 96, 1, 0), StringField, 0, 96);

    DEFINE_FIELD(thermal_valve_position, uint8_t, ObisId(0, THERMAL_MBUS_ID, 24, 4, 0), IntField, units::none);

    DEFINE_FIELD(thermal_delivered, TimestampedFixedValue, ObisId(0, THERMAL_MBUS_ID, 24, 2, 1), TimestampedFixedField,
                 units::GJ, units::MJ);

  } // namespace fields

} // namespace warmtelink
