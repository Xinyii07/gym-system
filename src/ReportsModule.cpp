#include "ReportsModule.h"
#include "UIUtils.h"
#include "BillingModule.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>

using namespace std;

namespace {

// Formats a number as "RM 12.50"
string fmtMoney(float v) {
    ostringstream oss;
    oss << fixed << setprecision(2) << v;
    return "RM " + oss.str();
}

// Formats a number as "12.5 %"
string fmtPercent(double v) {
    ostringstream oss;
    oss << fixed << setprecision(1) << v << " %";
    return oss.str();
}

// Prints the 3-section sales report (Service Breakdown, Financial Summary,
// KPIs) for either the current week or a chosen month.
// Only "Confirmed" payments count as real sales. Each Payment record is one
// transaction, so "quantity sold" for a service is just the number of
// matching confirmed payments.
void printSalesReport(const string& periodTitle, bool isWeekly, int targetMonth,
                       const GymService services[], int serviceCount,
                       const Payment paymentList[], int paymentCount) {
    clearScreen();
    displayAppHeader(periodTitle);

    if (paymentCount == 0) {
        printBoxLine("[INFO] No transaction records found in the billing system.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    // For the weekly report, "this week" = the last 7 days of the current month.
    int today, curMonth, curYear;
    getCurrentDate(today, curMonth, curYear);
    int weekStart = today - 6;

    printBoxLine("[ 1. SERVICE SALES BREAKDOWN ]", "left", 4);
    printBoxDivider();

    ostringstream header;
    header << left << setw(20) << "Service" << right << setw(12) << "Unit Price" << setw(8) << "Qty"
           << setw(14) << "Gross (RM)" << setw(14) << "Discount" << setw(14) << "Net (RM)";
    printBoxLine(header.str(), "left", 4);
    printBoxDivider();

    // Tally each confirmed, in-period payment exactly once: attribute it to
    // a known service when the name matches, otherwise to "Other / Manual
    // Charges". This guarantees every confirmed sale shows up somewhere and
    // the Financial Summary totals always match what's printed above them.
    float serviceGross[MAX_SERVICES] = {};
    float serviceDiscount[MAX_SERVICES] = {};
    int serviceQty[MAX_SERVICES] = {};
    float otherGross = 0.0f, otherDiscount = 0.0f;
    int otherQty = 0;

    float totalGross = 0.0f, totalDiscount = 0.0f, totalSST = 0.0f;
    int totalUnitsSold = 0, totalReceipts = 0, maxUnits = 0;
    string topService = "-";

    for (int j = 0; j < paymentCount; ++j) {
        const Payment& p = paymentList[j];

        bool inPeriod;
        if (isWeekly) {
            inPeriod = (p.month == curMonth && p.year == curYear && p.day >= weekStart && p.day <= today);
        }
        else {
            inPeriod = (p.month == targetMonth);
        }

        if (p.status != "Confirmed" || !inPeriod) continue;

        totalSST += p.sstAmount;
        totalReceipts++;

        int matchIndex = -1;
        for (int i = 0; i < serviceCount; ++i) {
            if (p.serviceType == services[i].name) {
                matchIndex = i;
                break;
            }
        }

        if (matchIndex != -1) {
            serviceQty[matchIndex]++;
            serviceGross[matchIndex] += p.subtotal;
            serviceDiscount[matchIndex] += p.discountAmount;
        }
        else {
            otherQty++;
            otherGross += p.subtotal;
            otherDiscount += p.discountAmount;
        }
    }

    for (int i = 0; i < serviceCount; ++i) {
        if (serviceQty[i] > 0) {
            float net = serviceGross[i] - serviceDiscount[i];
            ostringstream row;
            row << fixed << setprecision(2);
            row << left << setw(20) << services[i].name
                << right << setw(12) << services[i].price
                << setw(8) << serviceQty[i]
                << setw(14) << serviceGross[i]
                << setw(14) << serviceDiscount[i]
                << setw(14) << net;
            printBoxLine(row.str(), "left", 4);

            totalGross += serviceGross[i];
            totalDiscount += serviceDiscount[i];
            totalUnitsSold += serviceQty[i];

            if (serviceQty[i] > maxUnits) {
                maxUnits = serviceQty[i];
                topService = services[i].name;
            }
        }
    }

    if (otherQty > 0) {
        float net = otherGross - otherDiscount;
        ostringstream row;
        row << fixed << setprecision(2);
        row << left << setw(20) << "Other / Manual" << right << setw(12) << "-"
            << setw(8) << otherQty
            << setw(14) << otherGross
            << setw(14) << otherDiscount
            << setw(14) << net;
        printBoxLine(row.str(), "left", 4);

        totalGross += otherGross;
        totalDiscount += otherDiscount;
        totalUnitsSold += otherQty;

        if (otherQty > maxUnits) {
            maxUnits = otherQty;
            topService = "Other / Manual Charges";
        }
    }

    printBoxDivider();
    printBoxLine("Total Units Sold: " + to_string(totalUnitsSold), "left", 4);

    if (totalReceipts == 0) {
        printBoxLine(" ", "center");
        printBoxLine("[INFO] No confirmed sales transactions recorded for this period.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    float totalNet = totalGross - totalDiscount;
    float totalCash = totalNet + totalSST;
    float averageSpend = totalCash / totalReceipts;

    printBoxDivider();
    printBoxLine("[ 2. FINANCIAL SUMMARY ]", "left", 4);
    printBoxDivider();
    printBoxLine("Total Gross Sales        : " + fmtMoney(totalGross), "left", 4);
    printBoxLine("Total Discounts Applied  : " + fmtMoney(totalDiscount), "left", 4);
    printBoxLine("Net Gym Earnings         : " + fmtMoney(totalNet), "left", 4);
    printBoxLine("Total SST (6%) Collected : " + fmtMoney(totalSST), "left", 4);
    printBoxLine("Total Payments Received  : " + fmtMoney(totalCash), "left", 4);

    printBoxDivider();
    printBoxLine("[ 3. BUSINESS PERFORMANCE INDICATORS (KPIs) ]", "left", 4);
    printBoxDivider();
    printBoxLine("Total Receipts Issued    : " + to_string(totalReceipts), "left", 4);
    printBoxLine("Average Spend / Txn      : " + fmtMoney(averageSpend), "left", 4);
    printBoxLine("Top-Performing Service   : " + topService, "left", 4);

    printBoxBorder();
    pauseScreen();
}

void weeklySalesReport(const GymService services[], int serviceCount, const Payment paymentList[], int paymentCount) {
    printSalesReport("WEEKLY SALES REPORT (LAST 7 DAYS)", true, 0, services, serviceCount, paymentList, paymentCount);
}

void monthlySalesReport(const GymService services[], int serviceCount, const Payment paymentList[], int paymentCount) {
    if (paymentCount == 0) {
        clearScreen();
        displayAppHeader("MONTHLY SALES REPORT");
        printBoxLine("[INFO] No transaction records found in the billing system.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    int targetMonth = 0;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter target month (1-12): ";
    while (!(cin >> targetMonth) || targetMonth < 1 || targetMonth > 12) {
        cin.clear();
        cin.ignore((numeric_limits<streamsize>::max)(), '\n');
        cout << string(getLeftMargin() + 6, ' ') << "[ERROR] Invalid month! Please enter a valid month (1-12): ";
    }

    printSalesReport("MONTHLY SALES REPORT - MONTH " + to_string(targetMonth), false, targetMonth,
                      services, serviceCount, paymentList, paymentCount);
}

void generateSalesReport(const GymService services[], int serviceCount, const Payment paymentList[], int paymentCount) {
    int reportChoice = -1;
    while (reportChoice != 0) {
        clearScreen();
        displayAppHeader("SALES REPORT GENERATOR");
        printBoxLine("[1] Weekly Sales Report", "left", 6);
        printBoxLine("[2] Monthly Sales Report", "left", 6);
        printBoxLine("[0] Back", "left", 6);
        printBoxBorder();

        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select report type: ";
        if (!(cin >> reportChoice)) {
            cin.clear();
            cin.ignore((numeric_limits<streamsize>::max)(), '\n');
            reportChoice = -1;
            continue;
        }

        if (reportChoice == 1) {
            weeklySalesReport(services, serviceCount, paymentList, paymentCount);
        }
        else if (reportChoice == 2) {
            monthlySalesReport(services, serviceCount, paymentList, paymentCount);
        }
        else if (reportChoice != 0) {
            cout << string(getLeftMargin() + 6, ' ') << "[ERROR] Invalid choice! Please enter 1, 2, or 0.\n";
            pauseScreen();
        }
    }
}

// Active/expired mirrors the same expiryDate-vs-today convention used by
// viewAllMembers(); Member has no join-date field, so new-registration
// counts are not reported here.
void calculateMemberStatistics(const Member members[], int memberCount) {
    clearScreen();
    displayAppHeader("MEMBERSHIP STATISTICS");

    if (memberCount == 0) {
        printBoxLine("[INFO] No member records found in the system.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    string today = getCurrentDateStr();
    int activeCount = 0, expiredCount = 0;

    for (int i = 0; i < memberCount; ++i) {
        if (members[i].expiryDate >= today) {
            activeCount++;
        }
        else {
            expiredCount++;
        }
    }

    double retentionRate = (static_cast<double>(activeCount) / memberCount) * 100.0;

    printBoxLine("[ MEMBERSHIP DASHBOARD ]", "left", 4);
    printBoxDivider();
    printBoxLine("Total Registered Members : " + to_string(memberCount), "left", 4);
    printBoxLine("Active Members           : " + to_string(activeCount), "left", 4);
    printBoxLine("Expired Members          : " + to_string(expiredCount), "left", 4);
    printBoxLine("Active Retention Rate    : " + fmtPercent(retentionRate), "left", 4);
    printBoxBorder();
    pauseScreen();
}

} // namespace

void menuReportsAndStatistics(const Member members[], int memberCount,
                               const GymService services[], int serviceCount,
                               const Payment paymentList[], int paymentCount) {
    int choice = -1;
    while (choice != 0) {
        clearScreen();
        displayAppHeader("ADMIN REPORTING & STATISTICS MODULE");
        printBoxLine("[1] Generate Sales & Revenue Report", "left", 6);
        printBoxLine("[2] View Membership Statistics", "left", 6);
        printBoxLine("[0] Return to Main Menu", "left", 6);
        printBoxBorder();

        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter your choice: ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore((numeric_limits<streamsize>::max)(), '\n');
            choice = -1;
            continue;
        }

        switch (choice) {
            case 1:
                generateSalesReport(services, serviceCount, paymentList, paymentCount);
                break;
            case 2:
                calculateMemberStatistics(members, memberCount);
                break;
            case 0:
                cout << string(getLeftMargin() + 6, ' ') << "Returning to Main Menu...\n";
                break;
            default:
                cout << string(getLeftMargin() + 6, ' ') << "[ERROR] Invalid selection! Please enter 1, 2, or 0.\n";
                pauseScreen();
        }
    }
}
