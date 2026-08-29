#include "LoyaltyModule.h"
#include "UIUtils.h"
#include "Config.h"
#include "DataStore.h"
#include <iostream>
#include <limits>
#include <cctype>
#include <vector>

using namespace std;

namespace {

// Tier thresholds also mirror the billing discount % a member currently qualifies for.
void getTierInfo(int points, string& tierName, int& tierDiscount) {
    if (points >= 500) {
        tierName = "Platinum";
        tierDiscount = 10;
    }
    else if (points >= 250) {
        tierName = "Gold";
        tierDiscount = 8;
    }
    else if (points >= 150) {
        tierName = "Silver";
        tierDiscount = 5;
    }
    else {
        tierName = "Basic";
        tierDiscount = 0;
    }
}

// Auto-generate the next "G###" gift ID, based on the highest existing ID
string generateNextGiftID(const Gift giftList[], int giftCount) {
    int maxNum = 0;
    for (int i = 0; i < giftCount; ++i) {
        const string& id = giftList[i].giftID;
        if (id.size() > 1 && id[0] == 'G') {
            try {
                int num = stoi(id.substr(1));
                if (num > maxNum) maxNum = num;
            }
            catch (...) {
                // Non-numeric suffix - ignore it.
            }
        }
    }
    string numStr = to_string(maxNum + 1);
    while (numStr.length() < 3) numStr = "0" + numStr;
    return "G" + numStr;
}

// Reads a whole line and reports whether it is a valid positive whole number.
// Using getline (rather than `cin >> int`) avoids leaving a bad character in
// the stream buffer, which previously caused an endless input loop.
bool readPositiveInt(const string& input, int& value) {
    if (input.empty()) return false;
    for (char c : input) {
        if (!isdigit(static_cast<unsigned char>(c))) return false;
    }
    value = stoi(input);
    return value > 0;
}

// 5.7.2 View Member Points & Tier Status. A customer sees their own account
// directly; an admin picks a member via the arrow-key selector (Esc/cancel
// returns straight back to the Loyalty menu).
void awardPaymentPoint(const Member members[], int memberCount, bool isAdmin, const string& currentMemberID) {
    if (memberCount == 0) {
        clearScreen();
        displayAppHeader("MEMBER POINTS & TIER STATUS");
        printBoxLine("[INFO] No member records found in the system.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    string memberID = currentMemberID;
    if (isAdmin) {
        vector<string> options;
        for (int i = 0; i < memberCount; ++i) {
            options.push_back(members[i].memberID + " - " + members[i].fullName);
        }
        int selIdx = selectFromList("SELECT MEMBER TO VIEW POINTS & TIER", options);
        if (selIdx == -1) return; // canceled - back to the Loyalty menu
        memberID = members[selIdx].memberID;
    }

    int idx = -1;
    for (int i = 0; i < memberCount; ++i) {
        if (members[i].memberID == memberID) { idx = i; break; }
    }

    clearScreen();
    displayAppHeader("MEMBER POINTS & TIER STATUS");

    if (idx == -1) {
        printBoxLine("[ERROR] Member ID not found.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    string tierName;
    int tierDiscount;
    getTierInfo(members[idx].points, tierName, tierDiscount);

    printBoxLine("Member           : " + members[idx].fullName + " (" + members[idx].memberID + ")", "left", 4);
    printBoxDivider();
    printBoxLine("Total Points     : " + to_string(members[idx].points), "left", 4);
    printBoxLine("Tier Status      : " + tierName, "left", 4);
    printBoxLine("Billing Discount : " + to_string(tierDiscount) + " %", "left", 4);
    printBoxBorder();
    pauseScreen();
}

// 5.7.4 Process Gift Redemption (Admin only). Shows the full gift catalog
// and member list up front so the admin can read off IDs before typing them.
void redeemGift(Member members[], int memberCount, Gift giftList[], int giftCount) {
    clearScreen();
    displayAppHeader("PROCESS GIFT REDEMPTION");

    if (memberCount == 0 || giftCount == 0) {
        printBoxLine("[ERROR] Member or gift records are missing.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    printBoxLine("[ AVAILABLE GIFTS ]", "left", 4);
    printBoxDivider();
    bool anyAvailable = false;
    for (int i = 0; i < giftCount; ++i) {
        if (giftList[i].status == "AVAILABLE") {
            printBoxLine(giftList[i].giftID + " - " + giftList[i].name + " (" + to_string(giftList[i].pointCost) + " pts)", "left", 4);
            anyAvailable = true;
        }
    }
    if (!anyAvailable) {
        printBoxLine("No gifts currently available for redemption.", "left", 4);
    }
    printBoxDivider();
    printBoxLine("[ MEMBER LIST ]", "left", 4);
    printBoxDivider();
    for (int i = 0; i < memberCount; ++i) {
        printBoxLine(members[i].memberID + " - " + members[i].fullName + " (" + to_string(members[i].points) + " pts)", "left", 4);
    }
    printBoxBorder();

    string memberID;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter Member ID (or '0' to Cancel): ";
    cin >> memberID;
    if (memberID == "0") return;

    int mIdx = -1;
    for (int i = 0; i < memberCount; ++i) {
        if (members[i].memberID == memberID) { mIdx = i; break; }
    }

    if (mIdx == -1) {
        clearScreen();
        displayAppHeader("PROCESS GIFT REDEMPTION");
        printBoxLine("[ERROR] Member ID not found.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    string giftID;
    cout << string(getLeftMargin() + 6, ' ') << "Enter Gift ID to redeem (or '0' to Cancel): ";
    cin >> giftID;
    if (giftID == "0") return;

    int gIdx = -1;
    for (int i = 0; i < giftCount; ++i) {
        if (giftList[i].giftID == giftID) { gIdx = i; break; }
    }

    clearScreen();
    displayAppHeader("PROCESS GIFT REDEMPTION");

    if (gIdx == -1) {
        printBoxLine("[ERROR] Gift ID not found.", "center");
    }
    else if (giftList[gIdx].status != "AVAILABLE") {
        printBoxLine("[ERROR] This gift is currently unavailable.", "center");
    }
    else if (members[mIdx].points < giftList[gIdx].pointCost) {
        printBoxLine("[ERROR] Insufficient loyalty points for this gift.", "center");
    }
    else {
        members[mIdx].points -= giftList[gIdx].pointCost;
        DataStore::saveMembers(members, memberCount);

        printBoxLine("[SUCCESS] " + giftList[gIdx].name + " redeemed for " + members[mIdx].fullName + ".", "center");
        printBoxLine("Remaining Points: " + to_string(members[mIdx].points), "left", 4);
    }
    printBoxBorder();
    pauseScreen();
}

// Add a new item to the gift catalog (used by rewardCatalog's admin view).
// Point cost is read as a full line and validated manually so a stray
// non-numeric character can never desync cin and cause an endless loop.
void addGift(Gift giftList[], int& giftCount) {
    clearScreen();
    displayAppHeader("ADD NEW GIFT");

    if (giftCount >= MAX_GIFTS) {
        printBoxLine("[!] Gift catalog is full.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }
    printBoxBorder();

    cin.ignore((numeric_limits<streamsize>::max)(), '\n');
    string giftName;
    int pointCost = 0;

    while (true) {
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter Gift Name (or 0 to cancel): ";
        getline(cin, giftName);
        if (giftName == "0") {
            cout << string(getLeftMargin() + 6, ' ') << "Add gift cancelled.\n";
            pauseScreen();
            return;
        }

        cout << string(getLeftMargin() + 6, ' ') << "Enter Required Point Cost (or 0 to cancel): ";
        string costInput;
        getline(cin, costInput);
        if (costInput == "0") {
            cout << string(getLeftMargin() + 6, ' ') << "Add gift cancelled.\n";
            pauseScreen();
            return;
        }

        if (!giftName.empty() && readPositiveInt(costInput, pointCost)) break;
        cout << string(getLeftMargin() + 6, ' ') << "[ERROR] Gift name cannot be empty and point cost must be a positive whole number.\n";
    }

    string newID = generateNextGiftID(giftList, giftCount);
    giftList[giftCount] = { newID, giftName, pointCost, "AVAILABLE" };
    giftCount++;
    DataStore::saveGifts(giftList, giftCount);

    clearScreen();
    displayAppHeader("GIFT ADDED");
    printBoxLine("[SUCCESS] " + giftName + " (" + newID + ") added to the catalog!", "center");
    printBoxBorder();
    pauseScreen();
}

// Edit the point cost (redemption "discount" value) of an existing gift.
void editGiftPointCost(Gift giftList[], int giftCount) {
    clearScreen();
    displayAppHeader("EDIT GIFT DISCOUNT AMOUNT");

    if (giftCount == 0) {
        printBoxLine("[!] The gift catalog is empty.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    for (int i = 0; i < giftCount; ++i) {
        printBoxLine(giftList[i].giftID + " - " + giftList[i].name + " (" + to_string(giftList[i].pointCost)
            + " pts) [" + giftList[i].status + "]", "left", 4);
    }
    printBoxBorder();

    string giftID;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter Gift ID to edit (or '0' to Cancel): ";
    cin >> giftID;
    if (giftID == "0") return;

    int idx = -1;
    for (int i = 0; i < giftCount; ++i) {
        if (giftList[i].giftID == giftID) { idx = i; break; }
    }

    clearScreen();
    displayAppHeader("EDIT GIFT DISCOUNT AMOUNT");

    if (idx == -1) {
        printBoxLine("[ERROR] Gift ID not found.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    printBoxLine("Gift               : " + giftList[idx].name + " (" + giftList[idx].giftID + ")", "left", 4);
    printBoxLine("Current Point Cost : " + to_string(giftList[idx].pointCost), "left", 4);
    printBoxBorder();

    cin.ignore((numeric_limits<streamsize>::max)(), '\n');
    int newCost = 0;
    while (true) {
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter New Point Cost (or 0 to cancel): ";
        string costInput;
        getline(cin, costInput);
        if (costInput == "0") {
            cout << string(getLeftMargin() + 6, ' ') << "Edit cancelled.\n";
            pauseScreen();
            return;
        }
        if (readPositiveInt(costInput, newCost)) break;
        cout << string(getLeftMargin() + 6, ' ') << "[ERROR] Point cost must be a positive whole number.\n";
    }

    giftList[idx].pointCost = newCost;
    DataStore::saveGifts(giftList, giftCount);

    clearScreen();
    displayAppHeader("GIFT UPDATED");
    printBoxLine("[SUCCESS] " + giftList[idx].name + " point cost updated to " + to_string(newCost) + " pts.", "center");
    printBoxBorder();
    pauseScreen();
}

// Cancel/remove a gift (used by rewardCatalog's admin view). The admin picks
// between permanently deleting the catalog entry, or toggling it between
// AVAILABLE and UNAVAILABLE so it can be brought back later.
void cancelGift(Gift giftList[], int& giftCount) {
    clearScreen();
    displayAppHeader("CANCEL / REMOVE GIFT");

    if (giftCount == 0) {
        printBoxLine("[!] The gift catalog is empty.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    for (int i = 0; i < giftCount; ++i) {
        printBoxLine(giftList[i].giftID + " - " + giftList[i].name + " (" + to_string(giftList[i].pointCost)
            + " pts) [" + giftList[i].status + "]", "left", 4);
    }
    printBoxBorder();

    string giftID;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter Gift ID to cancel/remove (or '0' to Cancel): ";
    cin >> giftID;
    if (giftID == "0") return;

    int idx = -1;
    for (int i = 0; i < giftCount; ++i) {
        if (giftList[i].giftID == giftID) { idx = i; break; }
    }

    if (idx == -1) {
        clearScreen();
        displayAppHeader("CANCEL / REMOVE GIFT");
        printBoxLine("[ERROR] Gift ID not found.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    bool alreadyUnavailable = (giftList[idx].status == "UNAVAILABLE" || giftList[idx].status == "CANCELLED");

    clearScreen();
    displayAppHeader("CANCEL / REMOVE GIFT");
    printBoxLine("Gift : " + giftList[idx].name + " (" + giftList[idx].giftID + ") [" + giftList[idx].status + "]", "left", 4);
    printBoxDivider();
    printBoxLine("[1] Permanently remove from catalog", "left", 6);
    printBoxLine(string("[2] ") + (alreadyUnavailable ? "Mark as Available again" : "Set as temporarily unavailable"), "left", 6);
    printBoxLine("[0] Back (no changes)", "left", 6);
    printBoxBorder();

    int subChoice = -1;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter your choice: ";
    if (!(cin >> subChoice)) {
        cin.clear();
        cin.ignore((numeric_limits<streamsize>::max)(), '\n');
        subChoice = -1;
    }

    clearScreen();
    displayAppHeader("CANCEL / REMOVE GIFT");

    if (subChoice == 1) {
        string name = giftList[idx].name;
        for (int j = idx; j < giftCount - 1; ++j) {
            giftList[j] = giftList[j + 1];
        }
        giftCount--;
        DataStore::saveGifts(giftList, giftCount);
        printBoxLine("[SUCCESS] " + name + " has been permanently removed from the catalog.", "center");
    }
    else if (subChoice == 2) {
        giftList[idx].status = alreadyUnavailable ? "AVAILABLE" : "UNAVAILABLE";
        DataStore::saveGifts(giftList, giftCount);
        printBoxLine("[SUCCESS] " + giftList[idx].name + " is now " + giftList[idx].status + ".", "center");
    }
    else {
        printBoxLine("No changes made.", "center");
    }
    printBoxBorder();
    pauseScreen();
}

// 5.7.3 Browse Reward Catalog & Discounts. Customers get a read-only view;
// admins get the same view plus inline gift-catalog management (add / edit
// point cost / cancel), reachable without leaving this screen's menu.
void rewardCatalog(Gift giftList[], int& giftCount, bool isAdmin) {
    if (!isAdmin) {
        clearScreen();
        displayAppHeader("REWARD CATALOG & DISCOUNTS");

        if (giftCount == 0) {
            printBoxLine("[INFO] The gift catalog is currently empty.", "center");
            printBoxBorder();
            pauseScreen();
            return;
        }

        printBoxLine("[ TIER DISCOUNT BREAKDOWN ]", "left", 4);
        printBoxDivider();
        printBoxLine("Silver   (150 pts) : 5% billing discount", "left", 4);
        printBoxLine("Gold     (250 pts) : 8% billing discount", "left", 4);
        printBoxLine("Platinum (500 pts) : 10% billing discount", "left", 4);
        printBoxDivider();
        printBoxLine("Earn Points: Every RM1 spent = 1 point, credited once your payment is confirmed.", "left", 4);
        printBoxDivider();
        printBoxLine("[ AVAILABLE GIFTS ]", "left", 4);
        printBoxDivider();

        bool anyAvailable = false;
        for (int i = 0; i < giftCount; ++i) {
            if (giftList[i].status == "AVAILABLE") {
                printBoxLine(giftList[i].giftID + " - " + giftList[i].name + " (" + to_string(giftList[i].pointCost) + " pts)", "left", 4);
                anyAvailable = true;
            }
        }
        if (!anyAvailable) {
            printBoxLine("No gifts currently available for redemption.", "left", 4);
        }
        printBoxBorder();
        pauseScreen();
        return;
    }

    // Admin view: browse + manage, always able to return to the Loyalty menu via [0].
    int choice = -1;
    while (choice != 0) {
        clearScreen();
        displayAppHeader("BROWSE & MANAGE GIFT CATALOG & DISCOUNT");

        printBoxLine("[ TIER DISCOUNT BREAKDOWN ]", "left", 4);
        printBoxDivider();
        printBoxLine("Silver   (150 pts) : 5% billing discount", "left", 4);
        printBoxLine("Gold     (250 pts) : 8% billing discount", "left", 4);
        printBoxLine("Platinum (500 pts) : 10% billing discount", "left", 4);
        printBoxDivider();
        printBoxLine("Earn Points: Every RM1 spent = 1 point, credited once a payment is confirmed.", "left", 4);
        printBoxDivider();
        printBoxLine("[ GIFT CATALOG ]", "left", 4);
        printBoxDivider();

        if (giftCount == 0) {
            printBoxLine("The gift catalog is currently empty.", "left", 4);
        }
        else {
            for (int i = 0; i < giftCount; ++i) {
                printBoxLine(giftList[i].giftID + " - " + giftList[i].name + " (" + to_string(giftList[i].pointCost)
                    + " pts) [" + giftList[i].status + "]", "left", 4);
            }
        }
        printBoxDivider();
        printBoxLine("[1] Add New Gift", "left", 6);
        printBoxLine("[2] Edit Gift Discount Amount (Point Cost)", "left", 6);
        printBoxLine("[3] Cancel / Remove Gift", "left", 6);
        printBoxLine("[0] Back", "left", 6);
        printBoxBorder();

        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter your choice: ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore((numeric_limits<streamsize>::max)(), '\n');
            choice = -1;
            continue;
        }

        if (choice == 1) {
            addGift(giftList, giftCount);
        }
        else if (choice == 2) {
            editGiftPointCost(giftList, giftCount);
        }
        else if (choice == 3) {
            cancelGift(giftList, giftCount);
        }
        else if (choice != 0) {
            cout << string(getLeftMargin() + 6, ' ') << "[ERROR] Invalid selection! Please enter 0, 1, 2, or 3.\n";
            pauseScreen();
        }
    }
}

} // namespace

// 5.7.1 Main Loyalty & Rewards menu
void menuLoyalty(const string& role, Member members[], int memberCount,
                  const string& currentMemberID, Gift giftList[], int& giftCount) {
    bool isAdmin = (role == "admin");
    int choice = -1;
    while (choice != 0) {
        clearScreen();
        displayAppHeader("MEMBER LOYALTY & REWARDS MODULE");
        printBoxLine("[1] View Member Points & Tier Status", "left", 6);
        if (isAdmin) {
            printBoxLine("[2] Browse & Manage Gift Catalog & Discount", "left", 6);
            printBoxLine("[3] Process Gift Redemption (Admin)", "left", 6);
        }
        else {
            printBoxLine("[2] Browse Reward Catalog & Discounts", "left", 6);
        }
        printBoxLine("[0] Return to Main Menu", "left", 6);
        printBoxBorder();

        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter your choice: ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore((numeric_limits<streamsize>::max)(), '\n');
            choice = -1;
            continue;
        }

        if (choice == 1) {
            awardPaymentPoint(members, memberCount, isAdmin, currentMemberID);
        }
        else if (choice == 2) {
            rewardCatalog(giftList, giftCount, isAdmin);
        }
        else if (choice == 3 && isAdmin) {
            redeemGift(members, memberCount, giftList, giftCount);
        }
        else if (choice != 0) {
            cout << string(getLeftMargin() + 6, ' ') << "[ERROR] Invalid selection! Please enter a valid option.\n";
            pauseScreen();
        }
    }
}
