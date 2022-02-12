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

#include "fields.h"

using namespace warmtelink;
using namespace warmtelink::fields;

// Since C++11 it is possible to define the initial values for static
// const members in the class declaration, but if their address is
// taken, they still need a normal definition somewhere (to allocate
// storage).
constexpr char units::none[];
constexpr char units::GJ[];

constexpr ObisId identification::id;
constexpr char identification::name[];

constexpr ObisId p1_version::id;
constexpr char p1_version::name[];

constexpr ObisId timestamp::id;
constexpr char timestamp::name[];

constexpr ObisId equipment_id::id;
constexpr char equipment_id::name[];

constexpr ObisId message_short::id;
constexpr char message_short::name[];

constexpr ObisId message_long::id;
constexpr char message_long::name[];

constexpr ObisId thermal_device_type::id;
constexpr char thermal_device_type::name[];

constexpr ObisId thermal_equipment_id::id;
constexpr char thermal_equipment_id::name[];

constexpr ObisId thermal_valve_position::id;
constexpr char thermal_valve_position::name[];

constexpr ObisId thermal_delivered::id;
constexpr char thermal_delivered::name[];
