#pragma once
#include <string>

// ================= CONSTANT CONFIGURATION =================
const int BOX_WIDTH = 110;
const int INNER_WIDTH = BOX_WIDTH - 2;

const int DAYS = 7;
const int SLOTS = 7;
const int MAX_SERVICES = 30;
const int MAX_MEMBERS = 50;
const int MAX_BOOKINGS = 100;
const int MAX_MASTERS = 20;
const int TOTAL_LOCKERS = 8;
const float SST_RATE = 0.06f;
const int MAX_PAYMENTS = 100;
const int MAX_GIFTS = 20;

const std::string DAY_NAMES[DAYS] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

const std::string SLOT_TIMES[SLOTS] = {
    "08:00 - 10:00",
    "10:00 - 12:00",
    "12:00 - 14:00",
    "14:00 - 16:00",
    "16:00 - 18:00",
    "18:00 - 20:00",
    "20:00 - 22:00"
};
