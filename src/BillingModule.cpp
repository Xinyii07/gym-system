#include "BillingModule.h"
#include "UIUtils.h"
#include "Config.h"
#include "DataStore.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <vector>
#include <utility>
#include <limits>

using namespace std;

namespace {
    string fmtMoney(float amt) {
        ostringstream oss;
        oss << fixed << setprecision(2) << amt;
        return oss.str();
    }
}

// Helper function to get current system date (Day, Month, Year)
void getCurrentDate(int& day, int& month, int& year) {
    time_t now = time(0);
    tm ltm;
#ifdef _WIN32
    localtime_s(&ltm, &now);
#else
    localtime_r(&now, &ltm);
#endif
    day = ltm.tm_mday;
    month = 1 + ltm.tm_mon;
    year = 1900 + ltm.tm_year;
}

float calculateTotalCharge(float baseCharge, float discountPercent, float& discountAmt, float& sstAmt) {
    if (discountPercent < 0) {
        discountPercent = 0.0f; // Default to no discount if invalid/none supplied
    }

    discountAmt = baseCharge * (discountPercent / 100.0f);
    float subtotalAfterDiscount = baseCharge - discountAmt;
    sstAmt = subtotalAfterDiscount * SST_RATE;
    float totalPayable = subtotalAfterDiscount + sstAmt;

    return totalPayable;
}

int processPayment(const string& memberID, const string& bookingID, const string& serviceType, float baseCharge, float discountPercent, Payment paymentList[], int& paymentCount) {
    if (paymentCount >= MAX_PAYMENTS) {
        cout << "\nError: Payment records full, cannot process.\n";
        return -1;
    }

    float discountAmt = 0.0f;
    float sstAmt = 0.0f;
    float total = calculateTotalCharge(baseCharge, discountPercent, discountAmt, sstAmt);

    Payment newPayment;
    newPayment.paymentID = 1001 + paymentCount;
    newPayment.memberID = memberID;
    newPayment.bookingID = bookingID;
    newPayment.serviceType = serviceType;
    newPayment.subtotal = baseCharge;
    newPayment.discountAmount = discountAmt;
    newPayment.sstAmount = sstAmt;
    newPayment.totalAmount = total;
    newPayment.status = "Pending";
    newPayment.paymentMethod = "";
    newPayment.cashReceived = 0.0f;
    newPayment.changeGiven = 0.0f;
    getCurrentDate(newPayment.day, newPayment.month, newPayment.year);
    newPayment.confirmedBy = "";

    paymentList[paymentCount] = newPayment;
    paymentCount++;
    DataStore::savePayments(paymentList, paymentCount);

    return newPayment.paymentID;
}

// Voids any still-pending charge tied to a canceled booking so it stops
// showing up as owed. Confirmed (already-paid) charges are left untouched.
void voidPaymentsForBooking(Payment paymentList[], int paymentCount, const string& bookingID) {
    if (bookingID.empty()) return;
    bool changed = false;
    for (int i = 0; i < paymentCount; ++i) {
        if (paymentList[i].bookingID == bookingID && paymentList[i].status == "Pending") {
            paymentList[i].status = "Void";
            changed = true;
        }
    }
    if (changed) {
        DataStore::savePayments(paymentList, paymentCount);
    }
}

void generateInvoice(const Payment paymentList[], int paymentCount, int paymentID) {
    clearScreen();
    displayAppHeader("INVOICE");

    bool found = false;
    for (int i = 0; i < paymentCount; ++i) {
        if (paymentList[i].paymentID == paymentID) {
            found = true;
            const Payment& p = paymentList[i];

            printBoxLine("Payment ID       : " + to_string(p.paymentID), "left", 4);
            printBoxLine("Member ID        : " + p.memberID, "left", 4);
            printBoxLine("Booking ID       : " + (p.bookingID.empty() ? "-" : p.bookingID), "left", 4);
            printBoxLine("Service Type     : " + p.serviceType, "left", 4);
            printBoxDivider();
            printBoxLine("Subtotal         : RM " + fmtMoney(p.subtotal), "left", 4);
            printBoxLine("Discount         : RM " + fmtMoney(p.discountAmount), "left", 4);
            printBoxLine("SST (6%)         : RM " + fmtMoney(p.sstAmount), "left", 4);
            printBoxLine("Total Payable    : RM " + fmtMoney(p.totalAmount), "left", 4);
            printBoxDivider();
            printBoxLine("Payment Method   : " + p.paymentMethod, "left", 4);
            if (p.paymentMethod == "Cash") {
                printBoxLine("Cash Received    : RM " + fmtMoney(p.cashReceived), "left", 4);
                printBoxLine("Change Given     : RM " + fmtMoney(p.changeGiven), "left", 4);
            }
            printBoxLine("Status           : " + p.status, "left", 4);
            printBoxLine("Confirmed By     : " + (p.confirmedBy.empty() ? "-" : p.confirmedBy), "left", 4);
            printBoxLine("Date             : " + to_string(p.day) + "/" + to_string(p.month) + "/" + to_string(p.year), "left", 4);
            break;
        }
    }

    if (!found) {
        printBoxLine("[ERROR] Invoice not found.", "center");
    }
    printBoxBorder();
    pauseScreen();
}

static bool isAdminRole(const string& role) {
    return (role == "Admin" || role == "admin" || role == "ADMIN");
}

int viewPendingPayment(const Payment paymentList[], int paymentCount, const string& currentUserID, const string& userRole) {
    bool isAdmin = isAdminRole(userRole);
    string targetMemberID = currentUserID;

    if (isAdmin) {
        clearScreen();
        displayAppHeader("PENDING PAYMENTS");
        printBoxBorder();
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter Member ID to check (or 0 for ALL members): ";
        cin >> targetMemberID;
    }

    clearScreen();
    displayAppHeader("PENDING PAYMENTS");

    ostringstream headerLine;
    headerLine << left << setw(12) << "Payment ID" << setw(12) << "Member ID" << setw(12) << "Booking ID"
        << setw(22) << "Service Type" << setw(14) << "Subtotal" << setw(14) << "SST (6%)" << setw(14) << "Total (RM)";
    printBoxLine(headerLine.str(), "left", 2);
    printBoxDivider();

    int pendingCount = 0;
    float totalPendingAmount = 0.00f;

    for (int i = 0; i < paymentCount; ++i) {
        if (paymentList[i].status == "Pending") {
            if (isAdmin ? (targetMemberID == "0" || paymentList[i].memberID == targetMemberID)
                        : (paymentList[i].memberID == targetMemberID)) {
                const Payment& p = paymentList[i];
                ostringstream row;
                row << left << setw(12) << p.paymentID
                    << setw(12) << p.memberID
                    << setw(12) << (p.bookingID.empty() ? "-" : p.bookingID)
                    << setw(22) << p.serviceType
                    << fixed << setprecision(2)
                    << setw(14) << p.subtotal
                    << setw(14) << p.sstAmount
                    << setw(14) << p.totalAmount;
                printBoxLine(row.str(), "left", 2);

                totalPendingAmount += p.totalAmount;
                pendingCount++;
            }
        }
    }

    if (pendingCount == 0) {
        printBoxLine("No pending payments found.", "left", 4);
    }
    else {
        printBoxDivider();
        printBoxLine("Total Pending Items     : " + to_string(pendingCount), "left", 4);
        printBoxLine("Grand Total Outstanding : RM " + fmtMoney(totalPendingAmount), "left", 4);
    }
    printBoxBorder();
    pauseScreen();
    return pendingCount;
}

void viewPaymentHistory(const Payment paymentList[], int paymentCount, const string& memberID, const string& userRole) {
    bool isAdmin = isAdminRole(userRole);
    vector<pair<int, int>> periods; // (month, year)

    for (int i = 0; i < paymentCount; ++i) {
        if (paymentList[i].status == "Confirmed") {
            if (isAdmin || paymentList[i].memberID == memberID) {
                bool alreadyAdded = false;
                for (const auto& period : periods) {
                    if (period.first == paymentList[i].month && period.second == paymentList[i].year) {
                        alreadyAdded = true;
                        break;
                    }
                }
                if (!alreadyAdded) {
                    periods.push_back({ paymentList[i].month, paymentList[i].year });
                }
            }
        }
    }

    clearScreen();
    displayAppHeader("PAYMENT HISTORY");

    if (periods.empty()) {
        printBoxLine("[INFO] No payment history found for this account.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    printBoxLine("[ AVAILABLE HISTORY MONTHS ]", "left", 4);
    printBoxDivider();
    for (size_t i = 0; i < periods.size(); ++i) {
        string mStr = (periods[i].first < 10 ? "0" : "") + to_string(periods[i].first);
        printBoxLine(mStr + "/" + to_string(periods[i].second), "left", 4);
    }
    printBoxBorder();

    int selectedMonth = 0, selectedYear = 0;

    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter Month (MM, e.g., 8): ";
    while (!(cin >> selectedMonth) || selectedMonth < 1 || selectedMonth > 12) {
        cin.clear();
        cin.ignore((numeric_limits<streamsize>::max)(), '\n');
        cout << string(getLeftMargin() + 6, ' ') << "Invalid month. Enter (1-12): ";
    }

    cout << string(getLeftMargin() + 6, ' ') << "Enter Year (YYYY, e.g., 2026): ";
    while (!(cin >> selectedYear) || selectedYear < 2000 || selectedYear > 2100) {
        cin.clear();
        cin.ignore((numeric_limits<streamsize>::max)(), '\n');
        cout << string(getLeftMargin() + 6, ' ') << "Invalid year. Enter (e.g., 2026): ";
    }

    string selMonthStr = (selectedMonth < 10 ? "0" : "") + to_string(selectedMonth);

    clearScreen();
    displayAppHeader("PAYMENT HISTORY");
    printBoxLine("[ TRANSACTIONS FOR " + selMonthStr + "/" + to_string(selectedYear) + " ]", "left", 4);
    printBoxDivider();

    ostringstream headerLine;
    headerLine << left << setw(14) << "Date" << setw(14) << "Payment ID";
    if (isAdmin) headerLine << setw(12) << "Member ID";
    headerLine << setw(24) << "Service Type" << setw(14) << "Total (RM)" << setw(12) << "Status";
    printBoxLine(headerLine.str(), "left", 2);
    printBoxDivider();

    int countFound = 0;

    for (int i = 0; i < paymentCount; ++i) {
        if (paymentList[i].status == "Confirmed" &&
            paymentList[i].month == selectedMonth &&
            paymentList[i].year == selectedYear) {

            if (isAdmin || paymentList[i].memberID == memberID) {
                countFound++;
                const Payment& p = paymentList[i];

                string dayStr = (p.day < 10 ? "0" : "") + to_string(p.day);
                string monthStr = (p.month < 10 ? "0" : "") + to_string(p.month);
                string dateStr = dayStr + "/" + monthStr + "/" + to_string(p.year);

                ostringstream row;
                row << left << setw(14) << dateStr << setw(14) << p.paymentID;
                if (isAdmin) row << setw(12) << p.memberID;
                row << setw(24) << p.serviceType
                    << fixed << setprecision(2) << setw(14) << p.totalAmount
                    << setw(12) << p.status;
                printBoxLine(row.str(), "left", 2);
            }
        }
    }

    if (countFound == 0) {
        printBoxLine("No transactions found for " + selMonthStr + "/" + to_string(selectedYear) + ".", "left", 4);
        printBoxBorder();
        pauseScreen();
        return;
    }

    printBoxDivider();
    printBoxLine("Total Records Found: " + to_string(countFound), "left", 4);
    printBoxBorder();

    int targetPaymentID = 0;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter Payment ID to view full invoice (or 0 to exit): ";
    while (!(cin >> targetPaymentID)) {
        cin.clear();
        cin.ignore((numeric_limits<streamsize>::max)(), '\n');
        cout << string(getLeftMargin() + 6, ' ') << "Invalid input. Enter numeric Payment ID (or 0 to exit): ";
    }

    if (targetPaymentID != 0) {
        generateInvoice(paymentList, paymentCount, targetPaymentID);
    }
    else {
        pauseScreen();
    }
}

void confirmPayment(Payment paymentList[], int& paymentCount, const string& adminName, Member members[], int memberCount) {
    clearScreen();
    displayAppHeader("CONFIRM PENDING PAYMENT");

    ostringstream headerLine;
    headerLine << left << setw(12) << "Payment ID" << setw(12) << "Member ID" << setw(12) << "Booking ID"
        << setw(22) << "Service Type" << setw(14) << "Total (RM)";
    printBoxLine(headerLine.str(), "left", 2);
    printBoxDivider();

    int pendingCount = 0;
    for (int i = 0; i < paymentCount; ++i) {
        if (paymentList[i].status == "Pending") {
            const Payment& p = paymentList[i];
            ostringstream row;
            row << left << setw(12) << p.paymentID
                << setw(12) << p.memberID
                << setw(12) << (p.bookingID.empty() ? "-" : p.bookingID)
                << setw(22) << p.serviceType
                << fixed << setprecision(2) << setw(14) << p.totalAmount;
            printBoxLine(row.str(), "left", 2);
            pendingCount++;
        }
    }
    if (pendingCount == 0) {
        printBoxLine("No pending payments found.", "left", 4);
        printBoxBorder();
        pauseScreen();
        return;
    }
    printBoxBorder();

    int targetID = 0;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter Payment ID to confirm (or 0 to cancel): ";
    cin >> targetID;
    if (targetID == 0) return;

    int idx = -1;
    for (int i = 0; i < paymentCount; ++i) {
        if (paymentList[i].paymentID == targetID && paymentList[i].status == "Pending") {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        clearScreen();
        displayAppHeader("CONFIRM PENDING PAYMENT");
        printBoxLine("[ERROR] Pending Payment ID not found.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    Payment& p = paymentList[idx];

    clearScreen();
    displayAppHeader("CONFIRM PENDING PAYMENT");
    printBoxLine("Member ID  : " + p.memberID, "left", 4);
    printBoxLine("Amount Due : RM " + fmtMoney(p.totalAmount), "left", 4);
    printBoxBorder();

    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select Payment Method [1] Cash [2] Card: ";
    int methodChoice;
    cin >> methodChoice;

    if (methodChoice == 1) {
        p.paymentMethod = "Cash";
        cout << string(getLeftMargin() + 6, ' ') << "Enter Cash Received: RM ";
        cin >> p.cashReceived;
        p.changeGiven = p.cashReceived - p.totalAmount;
    }
    else {
        p.paymentMethod = "Card";
        p.cashReceived = p.totalAmount;
        p.changeGiven = 0.0f;
    }

    p.status = "Confirmed";
    p.confirmedBy = adminName;
    DataStore::savePayments(paymentList, paymentCount);

    // Loyalty points are earned only once a payment is actually confirmed as
    // paid, at a fixed rate of RM1 spent = 1 point.
    int pointsEarned = static_cast<int>(p.totalAmount + 0.5f);
    int mIdx = -1;
    for (int i = 0; i < memberCount; ++i) {
        if (members[i].memberID == p.memberID) { mIdx = i; break; }
    }
    if (mIdx != -1 && pointsEarned > 0) {
        members[mIdx].points += pointsEarned;
        DataStore::saveMembers(members, memberCount);
    }

    clearScreen();
    displayAppHeader("PAYMENT CONFIRMED");
    printBoxLine("[SUCCESS] Payment #" + to_string(p.paymentID) + " confirmed by " + adminName + ".", "center");
    printBoxDivider();
    if (p.paymentMethod == "Cash") {
        printBoxLine("Change Given : RM " + fmtMoney(p.changeGiven), "left", 4);
    }
    if (mIdx != -1 && pointsEarned > 0) {
        printBoxLine("Loyalty Points Earned : +" + to_string(pointsEarned) + " pts (RM1 = 1 pt)", "left", 4);
        printBoxLine("New Points Balance    : " + to_string(members[mIdx].points), "left", 4);
    }
    printBoxBorder();
    pauseScreen();
}

void menuBillingAndPayment(const string& role, const string& userID, Payment paymentList[], int& paymentCount, const string& adminName, Member members[], int memberCount) {
    bool isAdmin = isAdminRole(role);
    int choice;
    do {
        clearScreen();
        displayAppHeader("BILLING & PAYMENT MENU");
        printBoxLine("Logged User: [ " + userID + " ]", "left", 6);
        printBoxDivider();
        if (isAdmin) {
            printBoxLine("[1] Confirm Pending Payment", "left", 6);
            printBoxLine("[2] View Pending Payments (All Members)", "left", 6);
            printBoxLine("[3] View Payment History", "left", 6);
        }
        else {
            printBoxLine("[1] View My Payment History", "left", 6);
            printBoxLine("[2] View My Pending Payments", "left", 6);
        }
        printBoxLine("[0] Back to Menu", "left", 6);
        printBoxBorder();

        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select choice: ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        if (isAdmin) {
            switch (choice) {
            case 1: confirmPayment(paymentList, paymentCount, adminName, members, memberCount); break;
            case 2: viewPendingPayment(paymentList, paymentCount, userID, role); break;
            case 3: viewPaymentHistory(paymentList, paymentCount, userID, role); break;
            case 0: break;
            default: break;
            }
        }
        else {
            switch (choice) {
            case 1: viewPaymentHistory(paymentList, paymentCount, userID, role); break;
            case 2: viewPendingPayment(paymentList, paymentCount, userID, role); break;
            case 0: break;
            default: break;
            }
        }
    } while (choice != 0);
}
