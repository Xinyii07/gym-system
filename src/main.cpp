#include <iostream>
#include "Config.h"
#include "Types.h"
#include "UIUtils.h"
#include "DataStore.h"
#include "MemberModule.h"
#include "BillingModule.h"
#include "RecommendationModule.h"
#include "ServiceBookingModule.h"
#include "LockerModule.h"
#include "ReportsModule.h"
#include "LoyaltyModule.h"

using namespace std;

// ================= MODULE 5.0 MAIN FUNCTION =================
int main() {
    DataStore::ensureDataDir();

    // Service Initializations (loaded from data/services.dat if present)
    GymService services[MAX_SERVICES];
    int serviceCount = DataStore::loadServices(services, MAX_SERVICES);
    if (serviceCount == 0) {
        services[0] = { "S101", "Yoga", "Group Activity", 30.0f, {}, {}, true };
        services[0].capacity[0][0] = 15; // Mon 8:00 Yoga

        services[1] = { "S102", "HIIT", "Group Activity", 35.0f, {}, {}, true };
        services[1].capacity[1][1] = 15; // Tue 10:00 HIIT

        services[2] = { "S103", "Zumba", "Group Activity", 35.0f, {}, {}, true };
        services[2].capacity[3][2] = 15; // Thu 12:00 Zumba

        services[3] = { "S104", "Master Alex", "Personal Trainer", 100.0f, {}, {}, true };
        services[3].capacity[0][0] = 1;  // Mon 8:00 Master Alex

        serviceCount = 4;
        DataStore::saveServices(services, serviceCount);
    }

    // Member Initializations (loaded from data/members.dat if present)
    Member members[MAX_MEMBERS];
    int memberCount = DataStore::loadMembers(members, MAX_MEMBERS);
    if (memberCount == 0) {
        members[0] = { "M1001", "Alice", "123", "2026-12-31", "alice@example.com", "0123456789", 520 };
        members[1] = { "M1002", "Bob", "123", "2026-12-31", "bob@example.com", "0198765432", 260 };
        members[2] = { "M1003", "Charlie", "123", "2026-12-31", "charlie@example.com", "0111223344", 160 };
        members[3] = { "M1004", "David", "123", "2026-12-31", "david@example.com", "0155566778", 0 };
        memberCount = 4;
        DataStore::saveMembers(members, memberCount);
    }

    // Master Initializations (loaded from data/masters.dat if present)
    Master masters[MAX_MASTERS];
    int masterCount = DataStore::loadMasters(masters, MAX_MASTERS);
    if (masterCount == 0) {
        masters[0] = { "MS001", "Master Alan", "Yoga & Fitness", 8, 4.9 };
        masters[1] = { "MS002", "Dr. Sarah", "Nutrition & Diet", 5, 4.8 };
        masterCount = 2;
        DataStore::saveMasters(masters, masterCount);
    }

    // Locker Initializations (loaded from data/lockers.dat if present)
    LockerDevice lockers[TOTAL_LOCKERS];
    int loadedLockers = DataStore::loadLockers(lockers, TOTAL_LOCKERS);
    if (loadedLockers == 0) {
        lockers[0] = {1, "Standard", false, ""};
        lockers[1] = {2, "Standard", false, ""};
        lockers[2] = {3, "Standard", false, ""};
        lockers[3] = {4, "Standard", false, ""};
        lockers[4] = {5, "VIP Locker", false, ""};
        lockers[5] = {6, "VIP Locker", false, ""};
        lockers[6] = {7, "VIP Locker", false, ""};
        lockers[7] = {8, "VIP Locker", false, ""};
        DataStore::saveLockers(lockers, TOTAL_LOCKERS);
    }

    // Booking Records (loaded from data/bookings.dat if present)
    Booking bookingList[MAX_BOOKINGS];
    int bookingCount = DataStore::loadBookings(bookingList, MAX_BOOKINGS);

    // Payment Records (loaded from data/payments.dat if present)
    Payment paymentList[MAX_PAYMENTS];
    int paymentCount = DataStore::loadPayments(paymentList, MAX_PAYMENTS);

    // Gift Catalog (loaded from data/gifts.dat if present)
    Gift giftList[MAX_GIFTS];
    int giftCount = DataStore::loadGifts(giftList, MAX_GIFTS);
    if (giftCount == 0) {
        giftList[0] = { "G001", "Gym Water Bottle", 100, "AVAILABLE" };
        giftList[1] = { "G002", "Gym T-Shirt", 200, "AVAILABLE" };
        giftList[2] = { "G003", "Protein Shaker", 150, "AVAILABLE" };
        giftCount = 3;
        DataStore::saveGifts(giftList, giftCount);
    }

    // Outer Main System Loop matching 5.0 Pseudocode logic
    while (true) {
        string loggedID = "";
        string role = loginUser(members, memberCount, loggedID);

        if (role.empty()) {
            // Exit system loop
            break;
        }

        if (role == "customer") {
            int choice;
            do {
                clearScreen();
                displayAppHeader("CUSTOMER MAIN MENU");
                printBoxLine("Logged User: [ " + loggedID + " ]", "left", 6);
                printBoxDivider();
                printBoxLine("[1] Member management", "left", 6);
                printBoxLine("[2] Service", "left", 6);
                printBoxLine("[3] Payment", "left", 6);
                printBoxLine("[4] Bmi", "left", 6);
                printBoxLine("[5] Loyalty", "left", 6);
                printBoxLine("[0] Exit", "left", 6);
                printBoxBorder();

                cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter choice: ";
                if (!(cin >> choice)) {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    continue;
                }

                if (choice == 1) {
                    menuCustomerManagement(role, members, memberCount, loggedID, masters, masterCount);
                }
                else if (choice == 2) {
                    menuServiceBooking(loggedID, services, serviceCount, lockers, TOTAL_LOCKERS, bookingList, bookingCount, paymentList, paymentCount);
                }
                else if (choice == 3) {
                    menuBillingAndPayment(role, loggedID, paymentList, paymentCount, loggedID, members, memberCount);
                }
                else if (choice == 4) {
                    menuRecommendation();
                }
                else if (choice == 5) {
                    menuLoyalty(role, members, memberCount, loggedID, giftList, giftCount);
                }
                else if (choice == 0) {
                    break;
                }
            } while (choice != 0);
        }
        else if (role == "admin") {
            int choice;
            do {
                clearScreen();
                displayAppHeader("ADMIN MAIN MENU");
                printBoxLine("Logged User: [ ADMIN ]", "left", 6);
                printBoxDivider();
                printBoxLine("[1] Member management", "left", 6);
                printBoxLine("[2] Appointment management", "left", 6);
                printBoxLine("[3] Reporting and statistics", "left", 6);
                printBoxLine("[4] Manage gift", "left", 6);
                printBoxLine("[5] Billing and Payment", "left", 6);
                printBoxLine("[0] Exit", "left", 6);
                printBoxBorder();

                cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter choice: ";
                if (!(cin >> choice)) {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    continue;
                }

                if (choice == 1) {
                    menuCustomerManagement(role, members, memberCount, loggedID, masters, masterCount);
                }
                else if (choice == 2) {
                    menuScheduleManagement(services, serviceCount, members, memberCount, bookingList, bookingCount, lockers, TOTAL_LOCKERS, paymentList, paymentCount);
                }
                else if (choice == 3) {
                    menuReportsAndStatistics(members, memberCount, services, serviceCount, paymentList, paymentCount);
                }
                else if (choice == 4) {
                    menuLoyalty(role, members, memberCount, loggedID, giftList, giftCount);
                }
                else if (choice == 5) {
                    menuBillingAndPayment(role, loggedID, paymentList, paymentCount, loggedID, members, memberCount);
                }
                else if (choice == 0) {
                    break;
                }
            } while (choice != 0);
        }
    }

    return 0;
}
