#pragma once
#include <string>
#include "Types.h"

// ================= LOCKER SERVICE =================
void displayLockerStatusTable(LockerDevice lockers[], int totalLockers, bool isAdmin, const std::string& currentMemberID = "");
void menuLockerService(const std::string& memberID, LockerDevice lockers[], int totalLockers, Booking bookingList[], int& bookingCount);
void menuAdminLockerManagement(LockerDevice lockers[], int totalLockers);
