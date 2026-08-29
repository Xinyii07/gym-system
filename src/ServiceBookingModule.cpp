#include "ServiceBookingModule.h"
#include "UIUtils.h"
#include "Config.h"
#include "DataStore.h"
#include "MemberModule.h"
#include "LockerModule.h"
#include "BillingModule.h"
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

// ================= WEEKLY SCHEDULE TABLE =================
void displayWeeklyClassTable(const GymService services[], int serviceCount) {
    displayAppHeader("WEEKLY CLASS SCHEDULE (MON - SUN)");

    string header = " TIME SLOT     | MON       | TUE     | WED     | THU       | FRI       | SAT       | SUN       ";
    printBoxLine(header, "left", 1);
    printBoxDivider();

    for (int s = 0; s < SLOTS; ++s) {
        string timeLabel = SLOT_TIMES[s];
        string row = " " + timeLabel + " ";

        for (int d = 0; d < DAYS; ++d) {
            string item = "";
            for (int i = 0; i < serviceCount; ++i) {
                if (services[i].isActive && services[i].capacity[d][s] > 0) {
                    if (!item.empty()) item += ",";
                    item += services[i].name;
                }
            }
            if (item.empty()) item = "-";
            if (item.length() > 9) item = item.substr(0, 7) + "..";

            int padL = (10 - (int)item.length()) / 2;
            int padR = 10 - (int)item.length() - padL;
            row += "|" + string(padL, ' ') + item + string(padR, ' ');
        }
        printBoxLine(row, "left", 1);
        if (s < SLOTS - 1) printBoxDivider();
    }
    printBoxBorder();
}

void displaySingleSchedule(const GymService& srv, bool isAdminView) {
    displayAppHeader(srv.category + ": " + srv.name);

    string header = " Slot |  Time Slot    |   Mon   |   Tue   |   Wed   |   Thu   |   Fri   |   Sat   |   Sun   ";
    printBoxLine(header, "left", 1);
    printBoxDivider();

    for (int s = 0; s < SLOTS; ++s) {
        string row = " [" + to_string(s + 1) + "]  | " + SLOT_TIMES[s] + " ";

        for (int d = 0; d < DAYS; ++d) {
            int cap = srv.capacity[d][s];
            int bk = srv.booked[d][s];
            string status;

            if (cap == 0) {
                status = "N/A";
            }
            else {
                if (bk >= cap) status = "FULL";
                else status = "[" + to_string(bk) + "/" + to_string(cap) + "]";
            }

            int padL = (9 - (int)status.length()) / 2;
            int padR = 9 - (int)status.length() - padL;
            row += "|" + string(padL, ' ') + status + string(padR, ' ');
        }
        printBoxLine(row, "left", 1);
    }
    printBoxBorder();
}

// ================= ORIGINAL MEMBER PORTAL LOGIC =================
void createBooking(const string& memberID,
    GymService services[],
    int serviceCount,
    Booking bookingList[],
    int& bookingCount,
    Payment paymentList[],
    int& paymentCount) {
    clearScreen();
    displayAppHeader("CREATE NEW BOOKING");
    printBoxLine("[1] Group Fitness Activity", "left", 6);
    printBoxLine("[2] 1-on-1 Personal Trainer", "left", 6);
    printBoxLine("[0] Back", "left", 6);
    printBoxBorder();

    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select category: ";
    int catChoice;
    cin >> catChoice;

    if (catChoice == 0) return;
    string targetCat = (catChoice == 1) ? "Group Activity" : "Personal Trainer";

    clearScreen();
    displayAppHeader("CHOOSE A " + targetCat);
    int validIndices[MAX_SERVICES];
    int count = 0;
    for (int i = 0; i < serviceCount; ++i) {
        if (services[i].isActive && services[i].category == targetCat) {
            validIndices[count] = i;
            printBoxLine("[" + to_string(count + 1) + "] " + services[i].name, "left", 6);
            count++;
        }
    }
    printBoxLine("[0] Back", "left", 6);
    printBoxBorder();

    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select target: ";
    int choice;
    cin >> choice;
    if (choice < 1 || choice > count) return;

    int sIdx = validIndices[choice - 1];

    clearScreen();
    displaySingleSchedule(services[sIdx], false);

    int dayChoice, slotChoice;
    int promptMargin = getLeftMargin() + 2;

    cout << "\n" << string(promptMargin, ' ') << "Select Day  (1:Mon, 2:Tue, 3:Wed, 4:Thu, 5:Fri, 6:Sat, 7:Sun, 0:Cancel): ";
    cin >> dayChoice;
    if (dayChoice < 1 || dayChoice > DAYS) return;

    cout << string(promptMargin, ' ') << "Select Slot (1 to 7): ";
    cin >> slotChoice;
    if (slotChoice < 1 || slotChoice > SLOTS) return;

    int d = dayChoice - 1;
    int s = slotChoice - 1;

    int curCap = services[sIdx].capacity[d][s];
    int curBooked = services[sIdx].booked[d][s];

    if (curCap == 0 || curBooked >= curCap) {
        cout << "\n" << string(promptMargin, ' ') << "[!] Selected slot is CLOSED/UNAVAILABLE or FULL!\n";
        pauseScreen();
        return;
    }

    services[sIdx].booked[d][s]++;

    string newID = "BK" + to_string(1001 + bookingCount);
    bookingList[bookingCount] = { newID, memberID, targetCat, services[sIdx].name, d, s, true, "CONFIRMED" };
    bookingCount++;
    DataStore::saveBookings(bookingList, bookingCount);
    DataStore::saveServices(services, serviceCount);

    // Confirmed bookings bill the member: a priced service creates a pending
    // payment. Unpriced (RM 0) services - e.g. legacy data - skip billing.
    bool billed = false;
    if (services[sIdx].price > 0.0f) {
        processPayment(memberID, newID, services[sIdx].name, services[sIdx].price, 0.0f, paymentList, paymentCount);
        billed = true;
    }

    clearScreen();
    displayAppHeader("BOOKING CONFIRMED");
    printBoxLine(" ", "center");
    printBoxLine("[SUCCESS] Session booked successfully!", "center");
    printBoxLine("Booking ID   : " + newID, "left", 20);
    printBoxLine("Session Type : " + targetCat + " (" + services[sIdx].name + ")", "left", 20);
    printBoxLine("Day & Slot   : " + DAY_NAMES[d] + " | Slot [" + to_string(s + 1) + "] " + SLOT_TIMES[s], "left", 20);
    if (billed) {
        ostringstream priceStream;
        priceStream << fixed << setprecision(2) << services[sIdx].price;
        printBoxLine("Amount Due   : RM " + priceStream.str() + " (Pending - see Billing & Payment)", "left", 20);
    }
    printBoxLine(" ", "center");
    printBoxBorder();
    pauseScreen();
}

void viewGymService(GymService services[], int serviceCount, const string& memberID, Booking bookingList[], int& bookingCount, Payment paymentList[], int& paymentCount) {
    int choice;
    do {
        clearScreen();
        displayAppHeader("VIEW GYM SERVICES & TIMETABLE");
        printBoxLine("[1] Weekly Class Schedule Overview (Table)", "left", 6);
        printBoxLine("[2] Group Fitness Activities Details", "left", 6);
        printBoxLine("[3] Personal Trainers Timetable Details", "left", 6);
        printBoxLine("[4] -> Go to CREATE BOOKING Page <-", "left", 6);
        printBoxLine("[0] Back to Menu", "left", 6);
        printBoxBorder();

        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select option: ";
        cin >> choice;

        if (choice == 1) {
            clearScreen();
            displayWeeklyClassTable(services, serviceCount);
            pauseScreen();
        }
        else if (choice == 2 || choice == 3) {
            string targetCat = (choice == 2) ? "Group Activity" : "Personal Trainer";
            clearScreen();
            displayAppHeader("CHOOSE A " + targetCat + " TO VIEW");

            int validIndices[MAX_SERVICES];
            int count = 0;
            for (int i = 0; i < serviceCount; ++i) {
                if (services[i].isActive && services[i].category == targetCat) {
                    validIndices[count] = i;
                    printBoxLine("[" + to_string(count + 1) + "] " + services[i].name, "left", 6);
                    count++;
                }
            }
            printBoxLine("[0] Back", "left", 6);
            printBoxBorder();

            cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select item: ";
            int sel;
            cin >> sel;
            if (sel >= 1 && sel <= count) {
                clearScreen();
                displaySingleSchedule(services[validIndices[sel - 1]], false);
                pauseScreen();
            }
        }
        else if (choice == 4) {
            createBooking(memberID, services, serviceCount, bookingList, bookingCount, paymentList, paymentCount);
        }
    } while (choice != 0);
}

void viewMemberBooking(const string& memberID, const Booking bookingList[], int bookingCount) {
    clearScreen();
    displayAppHeader("MY PERSONAL BOOKINGS");

    string header = " Booking ID  | Type           | Activity/Master | Day       | Slot & Time         | Status";
    printBoxLine(header, "left", 1);
    printBoxDivider();

    int count = 0;
    for (int i = 0; i < bookingCount; ++i) {
        if (bookingList[i].memberID == memberID) {
            string row = " " + bookingList[i].bookingID + string(12 - (int)bookingList[i].bookingID.length(), ' ') + "| "
                + bookingList[i].category.substr(0, 14) + string(15 - (int)bookingList[i].category.substr(0, 14).length(), ' ') + "| "
                + bookingList[i].targetName + string(15 - (int)bookingList[i].targetName.length(), ' ') + "| "
                + DAY_NAMES[bookingList[i].dayIndex].substr(0, 9) + string(10 - (int)DAY_NAMES[bookingList[i].dayIndex].substr(0, 9).length(), ' ') + "| "
                + "[" + to_string(bookingList[i].slotIndex + 1) + "] " + SLOT_TIMES[bookingList[i].slotIndex] + " | "
                + (bookingList[i].isActive ? bookingList[i].status : "CANCELED");

            printBoxLine(row, "left", 1);
            count++;
        }
    }

    if (count == 0) {
        printBoxLine("No active or historical booking records found.", "center");
    }
    printBoxBorder();
    pauseScreen();
}

void editOrCancelBooking(const string& memberID,
    GymService services[],
    int serviceCount,
    Booking bookingList[],
    int bookingCount,
    Payment paymentList[],
    int paymentCount) {

    clearScreen();
    displayAppHeader("MY BOOKINGS - EDIT OR CANCEL");

    string header = " Booking ID  | Type           | Activity/Master | Day       | Slot & Time         | Status";
    printBoxLine(header, "left", 1);
    printBoxDivider();

    int activeCount = 0;
    for (int i = 0; i < bookingCount; ++i) {
        if (bookingList[i].memberID == memberID && bookingList[i].isActive && bookingList[i].status == "CONFIRMED") {
            string row = " " + bookingList[i].bookingID + string(12 - (int)bookingList[i].bookingID.length(), ' ') + "| "
                + bookingList[i].category.substr(0, 14) + string(15 - (int)bookingList[i].category.substr(0, 14).length(), ' ') + "| "
                + bookingList[i].targetName + string(15 - (int)bookingList[i].targetName.length(), ' ') + "| "
                + DAY_NAMES[bookingList[i].dayIndex].substr(0, 9) + string(10 - (int)DAY_NAMES[bookingList[i].dayIndex].substr(0, 9).length(), ' ') + "| "
                + "[" + to_string(bookingList[i].slotIndex + 1) + "] " + SLOT_TIMES[bookingList[i].slotIndex] + " | "
                + bookingList[i].status;

            printBoxLine(row, "left", 1);
            activeCount++;
        }
    }

    if (activeCount == 0) {
        printBoxLine("No active confirmed bookings available to edit or cancel.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }
    printBoxBorder();

    string targetID;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter Booking ID to Manage (or '0' to Cancel): ";
    cin >> targetID;

    if (targetID == "0") return;

    int bIdx = -1;
    for (int i = 0; i < bookingCount; ++i) {
        if (bookingList[i].bookingID == targetID && bookingList[i].memberID == memberID && bookingList[i].isActive) {
            bIdx = i;
            break;
        }
    }

    if (bIdx == -1) {
        clearScreen();
        displayAppHeader("OPERATION FAILED");
        printBoxLine("[ERROR] Active Booking ID not found.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    clearScreen();
    displayAppHeader("MANAGE BOOKING: " + targetID);
    printBoxLine("Selected: " + bookingList[bIdx].targetName + " (" + DAY_NAMES[bookingList[bIdx].dayIndex] + " Slot [" + to_string(bookingList[bIdx].slotIndex + 1) + "])", "left", 6);
    printBoxDivider();
    printBoxLine("[1] Edit Time Slot (Reschedule)", "left", 6);
    printBoxLine("[2] Cancel Booking", "left", 6);
    printBoxLine("[0] Back", "left", 6);
    printBoxBorder();

    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select action: ";
    int actionChoice;
    cin >> actionChoice;

    int targetServiceIdx = -1;
    for (int i = 0; i < serviceCount; ++i) {
        if (services[i].name == bookingList[bIdx].targetName && services[i].isActive) {
            targetServiceIdx = i;
            break;
        }
    }

    if (actionChoice == 1) {
        if (targetServiceIdx == -1) {
            cout << "\n[!] Target service is no longer available.\n";
            pauseScreen();
            return;
        }

        clearScreen();
        displaySingleSchedule(services[targetServiceIdx], false);

        int newDay, newSlot;
        int promptMargin = getLeftMargin() + 2;
        cout << "\n" << string(promptMargin, ' ') << "Select New Day (1:Mon to 7:Sun, 0:Cancel): ";
        cin >> newDay;
        if (newDay < 1 || newDay > DAYS) return;

        cout << string(promptMargin, ' ') << "Select New Slot (1 to 7): ";
        cin >> newSlot;
        if (newSlot < 1 || newSlot > SLOTS) return;

        int nD = newDay - 1;
        int nS = newSlot - 1;

        int nextCap = services[targetServiceIdx].capacity[nD][nS];
        int nextBooked = services[targetServiceIdx].booked[nD][nS];

        if (nextCap == 0 || nextBooked >= nextCap) {
            cout << "\n" << string(promptMargin, ' ') << "[!] Selected new slot is CLOSED/UNAVAILABLE or FULL!\n";
            pauseScreen();
            return;
        }

        int oldD = bookingList[bIdx].dayIndex;
        int oldS = bookingList[bIdx].slotIndex;
        services[targetServiceIdx].booked[oldD][oldS]--;
        services[targetServiceIdx].booked[nD][nS]++;

        bookingList[bIdx].dayIndex = nD;
        bookingList[bIdx].slotIndex = nS;
        DataStore::saveServices(services, serviceCount);
        DataStore::saveBookings(bookingList, bookingCount);

        clearScreen();
        displayAppHeader("RESCHEDULE SUCCESSFUL");
        printBoxLine(" ", "center");
        printBoxLine("[SUCCESS] Booking " + targetID + " has been updated!", "center");
        printBoxLine("New Schedule: " + DAY_NAMES[nD] + " | Slot [" + to_string(nS + 1) + "] " + SLOT_TIMES[nS], "center");
        printBoxLine(" ", "center");
        printBoxBorder();
        pauseScreen();
    }
    else if (actionChoice == 2) {
        int oldD = bookingList[bIdx].dayIndex;
        int oldS = bookingList[bIdx].slotIndex;

        if (targetServiceIdx != -1) {
            services[targetServiceIdx].booked[oldD][oldS]--;
        }

        bookingList[bIdx].isActive = false;
        bookingList[bIdx].status = "CANCELED";
        DataStore::saveServices(services, serviceCount);
        DataStore::saveBookings(bookingList, bookingCount);
        voidPaymentsForBooking(paymentList, paymentCount, bookingList[bIdx].bookingID);

        clearScreen();
        displayAppHeader("CANCELLATION SUCCESSFUL");
        printBoxLine("[SUCCESS] Booking " + targetID + " has been canceled.", "center");
        printBoxBorder();
        pauseScreen();
    }
}

// Service Booking Navigation for Members
void menuServiceBooking(const string& memberID,
    GymService services[],
    int serviceCount,
    LockerDevice lockers[],
    int totalLockers,
    Booking bookingList[],
    int& bookingCount,
    Payment paymentList[],
    int& paymentCount) {
    int choice;
    do {
        clearScreen();
        displayAppHeader("MEMBER - SERVICE & BOOKING MENU");
        printBoxLine("Logged User: [ " + memberID + " ]", "left", 6);
        printBoxDivider();
        printBoxLine("[1] View Gym Services & Weekly Timetable", "left", 6);
        printBoxLine("[2] Create New Booking", "left", 6);
        printBoxLine("[3] View My Personal Bookings", "left", 6);
        printBoxLine("[4] Edit or Cancel Booking", "left", 6);
        printBoxLine("[5] Locker Service", "left", 6);
        printBoxLine("[0] Back to Member Dashboard", "left", 6);
        printBoxBorder();

        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select option: ";
        cin >> choice;

        switch (choice) {
        case 1: viewGymService(services, serviceCount, memberID, bookingList, bookingCount, paymentList, paymentCount); break;
        case 2: createBooking(memberID, services, serviceCount, bookingList, bookingCount, paymentList, paymentCount); break;
        case 3: viewMemberBooking(memberID, bookingList, bookingCount); break;
        case 4: editOrCancelBooking(memberID, services, serviceCount, bookingList, bookingCount, paymentList, paymentCount); break;
        case 5: menuLockerService(memberID, lockers, totalLockers, bookingList, bookingCount); break;
        case 0: break;
        default: break;
        }
    } while (choice != 0);
}

// ================= ORIGINAL ADMINISTRATOR LOGIC =================
void addGymService(GymService services[], int& serviceCount) {
    clearScreen();
    displayAppHeader("ADD NEW SERVICE & INITIAL SLOT");

    if (serviceCount >= MAX_SERVICES) {
        printBoxLine("[!] Service limit reached.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    printBoxLine("Select Category:", "left", 6);
    printBoxLine("[1] Group Activity", "left", 6);
    printBoxLine("[2] Personal Trainer", "left", 6);
    printBoxLine("[0] Cancel", "left", 6);
    printBoxBorder();

    int typeChoice;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter choice: ";
    if (!(cin >> typeChoice) || typeChoice == 0) {
        return;
    }

    if (typeChoice != 1 && typeChoice != 2) {
        clearScreen();
        displayAppHeader("ADD SERVICE FAILED");
        printBoxLine("[FAILED] Invalid category selection.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    cin.ignore(10000, '\n');

    string category = (typeChoice == 1) ? "Group Activity" : "Personal Trainer";
    int defaultCap = (typeChoice == 1) ? 15 : 1;

    string serviceName = "";
    string trainerName = "";

    if (typeChoice == 1) {
        cout << string(getLeftMargin() + 6, ' ') << "Enter Service Name: ";
        getline(cin, serviceName);
        cout << string(getLeftMargin() + 6, ' ') << "Enter Trainer Name: ";
        getline(cin, trainerName);
    }
    else {
        cout << string(getLeftMargin() + 6, ' ') << "Enter Trainer Name: ";
        getline(cin, trainerName);
    }

    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Assign initial day (1:Mon - 7:Sun): ";
    int day;
    cin >> day;
    cout << string(getLeftMargin() + 6, ' ') << "Assign initial Slot (1 - 7): ";
    int slot;
    cin >> slot;

    float price = 0.0f;
    while (true) {
        cout << string(getLeftMargin() + 6, ' ') << "Enter Price per Booking (RM): ";
        if ((cin >> price) && price >= 0.0f) break;
        cin.clear();
        cin.ignore(10000, '\n');
        cout << string(getLeftMargin() + 6, ' ') << "[ERROR] Price must be a non-negative number.\n";
    }

    bool isGroupValid = (typeChoice == 1 && !serviceName.empty() && !trainerName.empty());
    bool isPersonalValid = (typeChoice == 2 && !trainerName.empty());
    bool isSlotValid = (day >= 1 && day <= DAYS && slot >= 1 && slot <= SLOTS);

    if ((!isGroupValid && !isPersonalValid) || !isSlotValid) {
        clearScreen();
        displayAppHeader("ADD SERVICE FAILED");
        printBoxLine("[FAILED] Incomplete or invalid information provided.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    string displayName = (typeChoice == 1)
        ? serviceName + " (Trainer: " + trainerName + ")"
        : "PT: " + trainerName;

    string sID = "S" + to_string(101 + serviceCount);
    services[serviceCount].serviceID = sID;
    services[serviceCount].name = displayName;
    services[serviceCount].category = category;
    services[serviceCount].price = price;
    services[serviceCount].isActive = true;

    for (int d = 0; d < DAYS; ++d) {
        for (int s = 0; s < SLOTS; ++s) {
            services[serviceCount].capacity[d][s] = 0;
            services[serviceCount].booked[d][s] = 0;
        }
    }

    services[serviceCount].capacity[day - 1][slot - 1] = defaultCap;
    serviceCount++;
    DataStore::saveServices(services, serviceCount);

    clearScreen();
    displayAppHeader("ADD SERVICE SUCCESS");
    printBoxLine("[SUCCESS] Added New Service: " + displayName + " (" + sID + ")", "center");
    printBoxBorder();
    pauseScreen();
}

void updateServiceDetails(GymService services[], int serviceCount) {
    clearScreen();
    displayAppHeader("UPDATE SERVICE DETAILS & SLOTS");

    int activeIdx[MAX_SERVICES];
    int count = 0;
    for (int i = 0; i < serviceCount; ++i) {
        if (services[i].isActive) {
            activeIdx[count] = i;
            printBoxLine("[" + to_string(count + 1) + "] " + services[i].serviceID + " - " + services[i].name + " (" + services[i].category + ")", "left", 6);
            count++;
        }
    }
    printBoxLine("[0] Cancel", "left", 6);
    printBoxBorder();

    int sel;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select service to update: ";
    cin >> sel;

    if (sel < 1 || sel > count) return;

    GymService& srv = services[activeIdx[sel - 1]];

    clearScreen();
    displaySingleSchedule(srv, true);

    printBoxLine("Choose what to update:", "left", 6);
    printBoxLine("[1] Update Service / Trainer Name", "left", 6);
    printBoxLine("[2] Add / Edit / Open a Slot", "left", 6);
    printBoxLine("[3] Close a Slot (Set Capacity to 0)", "left", 6);
    printBoxLine("[4] Update Price", "left", 6);
    printBoxLine("[0] Back", "left", 6);
    printBoxBorder();

    int opt;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select option: ";
    cin >> opt;

    if (opt == 1) {
        string newName;
        cout << string(getLeftMargin() + 6, ' ') << "Enter New Name: ";
        cin >> newName;
        srv.name = newName;
        DataStore::saveServices(services, serviceCount);
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "[SUCCESS] Name updated successfully!\n";
        pauseScreen();
    }
    else if (opt == 4) {
        float newPrice;
        while (true) {
            cout << string(getLeftMargin() + 6, ' ') << "Enter New Price (RM): ";
            if ((cin >> newPrice) && newPrice >= 0.0f) break;
            cin.clear();
            cin.ignore(10000, '\n');
            cout << string(getLeftMargin() + 6, ' ') << "[ERROR] Price must be a non-negative number.\n";
        }
        srv.price = newPrice;
        DataStore::saveServices(services, serviceCount);
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "[SUCCESS] Price updated successfully!\n";
        pauseScreen();
    }
    else if (opt == 2 || opt == 3) {
        int d, s;
        cout << string(getLeftMargin() + 6, ' ') << "Enter Day (1:Mon to 7:Sun): ";
        cin >> d;
        cout << string(getLeftMargin() + 6, ' ') << "Enter Slot (1 to 7): ";
        cin >> s;

        if (d >= 1 && d <= DAYS && s >= 1 && s <= SLOTS) {
            if (opt == 2) {
                int cap = (srv.category == "Personal Trainer") ? 1 : 15;
                cout << string(getLeftMargin() + 6, ' ') << "Enter Slot Capacity (Default " + to_string(cap) + "): ";
                cin >> cap;
                srv.capacity[d - 1][s - 1] = cap;
            }
            else {
                srv.capacity[d - 1][s - 1] = 0;
            }
            DataStore::saveServices(services, serviceCount);
            cout << "\n" << string(getLeftMargin() + 6, ' ') << "[SUCCESS] Time Slot updated successfully!\n";
            pauseScreen();
        }
    }
}

void deleteGymService(GymService services[], int serviceCount, Booking bookingList[], int bookingCount, Payment paymentList[], int paymentCount) {
    clearScreen();
    displayAppHeader("DELETE GYM SERVICE");

    int activeIdx[MAX_SERVICES];
    int count = 0;
    for (int i = 0; i < serviceCount; ++i) {
        if (services[i].isActive) {
            activeIdx[count] = i;
            printBoxLine("[" + to_string(count + 1) + "] " + services[i].serviceID + " - " + services[i].name, "left", 6);
            count++;
        }
    }
    printBoxLine("[0] Cancel", "left", 6);
    printBoxBorder();

    int sel;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select service to delete: ";
    cin >> sel;

    if (sel < 1 || sel > count) return;

    int targetIdx = activeIdx[sel - 1];
    services[targetIdx].isActive = false;

    int canceledCount = 0;
    for (int i = 0; i < bookingCount; ++i) {
        if (bookingList[i].targetName == services[targetIdx].name && bookingList[i].isActive) {
            bookingList[i].isActive = false;
            bookingList[i].status = "CANCELED";
            voidPaymentsForBooking(paymentList, paymentCount, bookingList[i].bookingID);
            canceledCount++;
        }
    }
    DataStore::saveServices(services, serviceCount);
    DataStore::saveBookings(bookingList, bookingCount);

    clearScreen();
    displayAppHeader("DELETE SUCCESS");
    printBoxLine("[SUCCESS] Service deactivated completely.", "center");
    printBoxLine("Canceled " + to_string(canceledCount) + " associated member bookings.", "center");
    printBoxBorder();
    pauseScreen();
}

void viewMasterSchedule(const GymService services[], int serviceCount) {
    clearScreen();
    displayWeeklyClassTable(services, serviceCount);
    pauseScreen();
}

void viewAllMemberBookings(const Booking bookingList[], int bookingCount) {
    clearScreen();
    displayAppHeader("ALL MEMBER BOOKINGS");

    string header = " ID      | Member | Category       | Service/Master   | Day | Slot | Status";
    printBoxLine(header, "left", 2);
    printBoxDivider();

    for (int i = 0; i < bookingCount; ++i) {
        string row = " " + bookingList[i].bookingID + " | "
            + bookingList[i].memberID + "  | "
            + bookingList[i].category.substr(0, 14) + string(15 - (int)bookingList[i].category.substr(0, 14).length(), ' ') + "| "
            + bookingList[i].targetName + string(16 - (int)bookingList[i].targetName.length(), ' ') + "| "
            + DAY_NAMES[bookingList[i].dayIndex].substr(0, 3) + " | ["
            + to_string(bookingList[i].slotIndex + 1) + "]  | "
            + (bookingList[i].isActive ? bookingList[i].status : "CANCELED");

        printBoxLine(row, "left", 2);
    }

    if (bookingCount == 0) {
        printBoxLine("No bookings recorded.", "center");
    }
    printBoxBorder();
    pauseScreen();
}

void markAppointmentAsDone(Booking bookingList[], int bookingCount) {
    clearScreen();
    displayAppHeader("MARK ATTENDED APPOINTMENTS AS DONE");

    int confirmedIndices[MAX_BOOKINGS];
    int pendingCount = 0;

    for (int i = 0; i < bookingCount; ++i) {
        if (bookingList[i].status == "CONFIRMED" && bookingList[i].isActive) {
            confirmedIndices[pendingCount] = i;
            pendingCount++;
        }
    }

    if (pendingCount == 0) {
        printBoxLine(" ", "center");
        printBoxLine("No pending (CONFIRMED) bookings found to process.", "center");
        printBoxLine(" ", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    printBoxLine("Found " + to_string(pendingCount) + " pending appointment(s).", "center");
    printBoxLine("Review each booking below and mark attendance:", "center");
    printBoxLine("Press [Y] = Attended (Mark as DONE) | [N] = Skip / Absent", "center");
    printBoxBorder();
    pauseScreen();

    int processedCount = 0;
    int promptMargin = getLeftMargin() + 6;

    for (int k = 0; k < pendingCount; ++k) {
        int idx = confirmedIndices[k];

        clearScreen();
        displayAppHeader("PROCESSING ATTENDANCE (" + to_string(k + 1) + "/" + to_string(pendingCount) + ")");
        printBoxLine("Booking ID    : " + bookingList[idx].bookingID, "left", 6);
        printBoxLine("Member ID     : " + bookingList[idx].memberID, "left", 6);
        printBoxLine("Service Type  : " + bookingList[idx].category, "left", 6);
        printBoxLine("Activity/Master: " + bookingList[idx].targetName, "left", 6);
        printBoxLine("Schedule      : " + DAY_NAMES[bookingList[idx].dayIndex] + " | " + SLOT_TIMES[bookingList[idx].slotIndex], "left", 6);
        printBoxLine("Current Status: CONFIRMED", "left", 6);
        printBoxBorder();

        char choice;
        cout << "\n" << string(promptMargin, ' ') << "Did member [" << bookingList[idx].memberID << "] attend this session? (Y/N): ";
        cin >> choice;

        if (choice == 'y' || choice == 'Y') {
            bookingList[idx].status = "DONE";
            processedCount++;
            DataStore::saveBookings(bookingList, bookingCount);
            cout << string(promptMargin, ' ') << ">> [SUCCESS] Marked Booking " << bookingList[idx].bookingID << " as DONE!\n";
        }
        else {
            cout << string(promptMargin, ' ') << ">> [SKIPPED] Booking " << bookingList[idx].bookingID << " remains CONFIRMED.\n";
        }

        pauseScreen();
    }

    clearScreen();
    displayAppHeader("ATTENDANCE PROCESSING COMPLETE");
    printBoxLine(" ", "center");
    printBoxLine("[SUCCESS] Attendance verification finished!", "center");
    printBoxLine("Total Pending Reviewed : " + to_string(pendingCount), "left", 16);
    printBoxLine("Marked as DONE         : " + to_string(processedCount), "left", 16);
    printBoxLine("Skipped / Remaining    : " + to_string(pendingCount - processedCount), "left", 16);
    printBoxLine(" ", "center");
    printBoxBorder();
    pauseScreen();
}

// Appointment Management Menu for Admin
void menuScheduleManagement(GymService services[], int& serviceCount, Member members[], int memberCount, Booking bookingList[], int& bookingCount, LockerDevice lockers[], int totalLockers, Payment paymentList[], int paymentCount) {
    int choice;
    do {
        clearScreen();
        displayAppHeader("ADMIN - SCHEDULE & SERVICE MANAGEMENT");
        printBoxLine("[1] Add Gym Service & Assign Slot", "left", 6);
        printBoxLine("[2] Update Service / Slots      ", "left", 6);
        printBoxLine("[3] Delete Gym Service           ", "left", 6);
        printBoxLine("[4] View Master Schedule Table   ", "left", 6);
        printBoxLine("[5] View All Member Bookings     ", "left", 6);
        printBoxLine("[6] Mark Appointment As Done     ", "left", 6);
        printBoxLine("[7] Renew Membership             ", "left", 6);
        printBoxLine("[8] View & Manage Locker Status  ", "left", 6);
        printBoxLine("[0] Return to Admin Dashboard", "left", 6);
        printBoxBorder();

        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select Option: ";
        cin >> choice;

        switch (choice) {
        case 1: addGymService(services, serviceCount); break;
        case 2: updateServiceDetails(services, serviceCount); break;
        case 3: deleteGymService(services, serviceCount, bookingList, bookingCount, paymentList, paymentCount); break;
        case 4: viewMasterSchedule(services, serviceCount); break;
        case 5: viewAllMemberBookings(bookingList, bookingCount); break;
        case 6: markAppointmentAsDone(bookingList, bookingCount); break;
        case 7: renewMembership(members, memberCount); break;
        case 8: menuAdminLockerManagement(lockers, totalLockers); break;
        case 0: break;
        default: break;
        }
    } while (choice != 0);
}
