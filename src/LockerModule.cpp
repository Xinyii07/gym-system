#include "LockerModule.h"
#include "UIUtils.h"
#include "DataStore.h"
#include <iostream>

using namespace std;

void displayLockerStatusTable(LockerDevice lockers[], int totalLockers, bool isAdmin, const string& currentMemberID) {
    if (isAdmin) {
        string header = " Locker No. | Locker Type   | Status       | Occupied By ";
        printBoxLine(header, "left", 6);
        printBoxDivider();

        for (int i = 0; i < totalLockers; ++i) {
            string statusStr = lockers[i].isOccupied ? "[OCCUPIED]" : "[EMPTY]";
            string userStr = lockers[i].isOccupied ? lockers[i].occupiedBy : "-";

            string row = " Locker " + to_string(lockers[i].lockerNo) + string(4 - (int)to_string(lockers[i].lockerNo).length(), ' ') + " | "
                + lockers[i].lockerType + string(13 - (int)lockers[i].lockerType.length(), ' ') + " | "
                + statusStr + string(12 - (int)statusStr.length(), ' ') + " | "
                + userStr;

            printBoxLine(row, "left", 6);
        }
    }
    else {
        string header = " Locker No. | Locker Type   | Status       ";
        printBoxLine(header, "left", 6);
        printBoxDivider();

        for (int i = 0; i < totalLockers; ++i) {
            string statusStr = "[EMPTY]";
            if (lockers[i].isOccupied) {
                if (lockers[i].occupiedBy == currentMemberID) {
                    statusStr = "[YOUR BOOKING]";
                }
                else {
                    statusStr = "[OCCUPIED]";
                }
            }

            string row = " Locker " + to_string(lockers[i].lockerNo) + string(4 - (int)to_string(lockers[i].lockerNo).length(), ' ') + " | "
                + lockers[i].lockerType + string(13 - (int)lockers[i].lockerType.length(), ' ') + " | "
                + statusStr;

            printBoxLine(row, "left", 6);
        }
    }
    printBoxBorder();
}

void menuLockerService(const string& memberID,
    LockerDevice lockers[],
    int totalLockers,
    Booking bookingList[],
    int& bookingCount) {
    int choice;
    do {
        clearScreen();
        displayAppHeader("LOCKER SERVICE");
        printBoxLine("Logged User: [ " + memberID + " ]", "left", 6);
        printBoxDivider();
        printBoxLine(" REAL-TIME LOCKER AVAILABILITY STATUS", "center");
        printBoxDivider();

        displayLockerStatusTable(lockers, totalLockers, false, memberID);

        printBoxLine("ACTION OPTIONS:", "left", 6);
        printBoxLine("[1 - " + to_string(totalLockers) + "] Enter Locker Number to BOOK Directly", "left", 6);
        printBoxLine("[99] Release / Cancel My Current Locker", "left", 6);
        printBoxLine("[ 0] Back to Service Menu", "left", 6);
        printBoxBorder();

        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select action / Locker No: ";
        cin >> choice;

        if (choice >= 1 && choice <= totalLockers) {
            int targetIdx = choice - 1;

            if (lockers[targetIdx].isOccupied) {
                clearScreen();
                displayAppHeader("BOOKING FAILED");
                printBoxLine("[!] Locker " + to_string(choice) + " is already OCCUPIED!", "center");
                printBoxLine("Please select another locker with [EMPTY] status.", "center");
                printBoxBorder();
                pauseScreen();
            }
            else {
                lockers[targetIdx].isOccupied = true;
                lockers[targetIdx].occupiedBy = memberID;

                string newID = "BK" + to_string(1001 + bookingCount);
                bookingList[bookingCount] = { newID, memberID, "Locker Service", "Locker " + to_string(choice), 0, 0, true, "CONFIRMED" };
                bookingCount++;
                DataStore::saveLockers(lockers, totalLockers);
                DataStore::saveBookings(bookingList, bookingCount);

                clearScreen();
                displayAppHeader("LOCKER RESERVED SUCCESSFULLY");
                printBoxLine("[SUCCESS] You have successfully rented Locker " + to_string(choice) + "!", "center");
                printBoxLine("Booking ID  : " + newID, "left", 20);
                printBoxLine("Locker No   : Locker " + to_string(choice) + " (" + lockers[targetIdx].lockerType + ")", "left", 20);
                printBoxLine("Assigned To : " + memberID, "left", 20);
                printBoxBorder();
                pauseScreen();
            }
        }
        else if (choice == 99) {
            int myLockerIdx = -1;
            for (int i = 0; i < totalLockers; ++i) {
                if (lockers[i].isOccupied && lockers[i].occupiedBy == memberID) {
                    myLockerIdx = i;
                    break;
                }
            }

            if (myLockerIdx == -1) {
                clearScreen();
                displayAppHeader("RELEASE FAILED");
                printBoxLine("[!] You do not currently own any rented locker.", "center");
                printBoxBorder();
                pauseScreen();
            }
            else {
                lockers[myLockerIdx].isOccupied = false;
                lockers[myLockerIdx].occupiedBy = "";
                DataStore::saveLockers(lockers, totalLockers);

                clearScreen();
                displayAppHeader("LOCKER RELEASED");
                printBoxLine("[SUCCESS] Locker " + to_string(lockers[myLockerIdx].lockerNo) + " has been freed up!", "center");
                printBoxBorder();
                pauseScreen();
            }
        }

    } while (choice != 0);
}

void menuAdminLockerManagement(LockerDevice lockers[], int totalLockers) {
    int choice;
    do {
        clearScreen();
        displayAppHeader("ADMIN - LOCKER MANAGEMENT");
        printBoxLine("Logged User: [ ADMIN ]", "left", 6);
        printBoxDivider();
        printBoxLine(" ALL LOCKERS REAL-TIME MONITORING", "center");
        printBoxDivider();

        displayLockerStatusTable(lockers, totalLockers, true, "");

        printBoxLine("ADMIN ACTIONS:", "left", 6);
        printBoxLine("[1] Force Release / Reset a Locker", "left", 6);
        printBoxLine("[0] Back to Management Menu", "left", 6);
        printBoxBorder();

        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select action: ";
        cin >> choice;

        if (choice == 1) {
            cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter Locker No. to RESET (1-" + to_string(totalLockers) + "): ";
            int lockerNo;
            cin >> lockerNo;

            if (lockerNo >= 1 && lockerNo <= totalLockers) {
                int idx = lockerNo - 1;
                if (!lockers[idx].isOccupied) {
                    cout << "\n" << string(getLeftMargin() + 6, ' ') << "[!] Locker " << lockerNo << " is already EMPTY!\n";
                    pauseScreen();
                }
                else {
                    string previousUser = lockers[idx].occupiedBy;
                    lockers[idx].isOccupied = false;
                    lockers[idx].occupiedBy = "";
                    DataStore::saveLockers(lockers, totalLockers);

                    clearScreen();
                    displayAppHeader("ADMIN LOCKER RESET");
                    printBoxLine("[SUCCESS] Locker " + to_string(lockerNo) + " has been reset to EMPTY!", "center");
                    printBoxLine("Previous Member: " + previousUser, "left", 20);
                    printBoxBorder();
                    pauseScreen();
                }
            }
        }
    } while (choice != 0);
}
