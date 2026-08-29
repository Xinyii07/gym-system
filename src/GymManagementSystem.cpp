// ============================================================================
// SJY FITNESS - GYM SERVICE MANAGEMENT SYSTEM
// Single-file build (all modules combined for submission)
// ============================================================================

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cctype>
#include <algorithm>
#include <limits>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#endif

using namespace std;

// ============================================================================
// SECTION: CONFIG (was Config.h)
// ============================================================================
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

const string DAY_NAMES[DAYS] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

const string SLOT_TIMES[SLOTS] = {
    "08:00 - 10:00",
    "10:00 - 12:00",
    "12:00 - 14:00",
    "14:00 - 16:00",
    "16:00 - 18:00",
    "18:00 - 20:00",
    "20:00 - 22:00"
};

// ============================================================================
// SECTION: TYPES (was Types.h)
// ============================================================================
struct LockerDevice {
    int lockerNo;
    string lockerType;
    bool isOccupied;
    string occupiedBy;
};

struct GymService {
    string serviceID;
    string name;
    string category; // "Group Activity" or "Personal Trainer"
    float price;          // Charge per booking (RM); billed as a pending payment on booking
    int capacity[DAYS][SLOTS];
    int booked[DAYS][SLOTS];
    bool isActive;
};

struct Member {
    string memberID;
    string fullName;
    string password;
    string expiryDate;
    string email;
    string phone;
    int points; // Loyalty points balance, spent via the Loyalty & Rewards module
};

struct Payment {
    int paymentID;
    string memberID;
    string bookingID;     // Links back to the Booking that generated this charge; "" for manual/admin charges
    string serviceType;
    float subtotal;
    float discountAmount;
    float sstAmount;
    float totalAmount;
    string status;        // "Pending", "Confirmed", or "Void" (booking canceled before payment)
    string paymentMethod; // "Cash" or "Card"
    float cashReceived;
    float changeGiven;
    int day, month, year;
    string confirmedBy;
};

struct Booking {
    string bookingID;
    string memberID;
    string category;
    string targetName;
    int dayIndex;
    int slotIndex;
    bool isActive;
    string status;     // "CONFIRMED", "DONE", "CANCELED"
};

// Data structure for a Master / Trainer profile
struct Master {
    string masterID;
    string name;
    string specialization;
    int experience;
    double rating;
};

// Data structure for a redeemable loyalty catalog item
struct Gift {
    string giftID;
    string name;
    int pointCost;
    string status; // "AVAILABLE" or "CANCELLED"
};

// ============================================================================
// SECTION: FORWARD DECLARATIONS (was UIUtils.h, DataStore.h, BillingModule.h,
// LockerModule.h, LoyaltyModule.h, MemberModule.h, RecommendationModule.h,
// ReportsModule.h, ServiceBookingModule.h)
// ============================================================================

// ---- UI RENDERING & BUFFER UTILITIES ----
void clearScreen();
int getLeftMargin();
void printBoxBorder();
void printBoxDivider();
void printBoxLine(const string& text, const string& align = "left", int indent = 4);
void displayAppHeader(const string& subtitle);
void pauseScreen();
string getCurrentDateStr();
bool isValidEmail(const string& email);
bool isValidPhone(const string& phone);
bool isValidSpecialization(const string& text);
bool isValidExperience(const string& text);
string readMaskedInput();
void setCurrentRoleLabel(const string& label);
int selectFromList(const string& title, const vector<string>& items);

// ---- FILE-BASED DATA PERSISTENCE ----
namespace DataStore {
    void ensureDataDir();

    void saveMembers(const Member members[], int memberCount);
    int loadMembers(Member members[], int maxMembers);

    void saveMasters(const Master masters[], int masterCount);
    int loadMasters(Master masters[], int maxMasters);

    void saveServices(const GymService services[], int serviceCount);
    int loadServices(GymService services[], int maxServices);

    void saveBookings(const Booking bookingList[], int bookingCount);
    int loadBookings(Booking bookingList[], int maxBookings);

    void saveLockers(const LockerDevice lockers[], int totalLockers);
    int loadLockers(LockerDevice lockers[], int totalLockers);

    void savePayments(const Payment paymentList[], int paymentCount);
    int loadPayments(Payment paymentList[], int maxPayments);

    void saveGifts(const Gift giftList[], int giftCount);
    int loadGifts(Gift giftList[], int maxGifts);
}

// ---- BILLING & PAYMENT MANAGEMENT ----
void getCurrentDate(int& day, int& month, int& year);
float calculateTotalCharge(float baseCharge, float discountPercent, float& discountAmt, float& sstAmt);
int processPayment(const string& memberID, const string& bookingID, const string& serviceType, float baseCharge, float discountPercent, Payment paymentList[], int& paymentCount);
void voidPaymentsForBooking(Payment paymentList[], int paymentCount, const string& bookingID);
void generateInvoice(const Payment paymentList[], int paymentCount, int paymentID);
int viewPendingPayment(const Payment paymentList[], int paymentCount, const string& currentUserID, const string& userRole);
void viewPaymentHistory(const Payment paymentList[], int paymentCount, const string& memberID, const string& userRole);
void confirmPayment(Payment paymentList[], int& paymentCount, const string& adminName, Member members[], int memberCount);
void menuBillingAndPayment(const string& role, const string& userID, Payment paymentList[], int& paymentCount, const string& adminName, Member members[], int memberCount);

// ---- LOCKER SERVICE ----
void displayLockerStatusTable(LockerDevice lockers[], int totalLockers, bool isAdmin, const string& currentMemberID = "");
void menuLockerService(const string& memberID, LockerDevice lockers[], int totalLockers, Booking bookingList[], int& bookingCount);
void menuAdminLockerManagement(LockerDevice lockers[], int totalLockers);

// ---- LOYALTY & GIFT MANAGEMENT ----
void menuLoyalty(const string& role, Member members[], int memberCount,
                  const string& currentMemberID, Gift giftList[], int& giftCount);

// ---- MEMBER & COACH MANAGEMENT ----
void registerMember(Member members[], int& memberCount);
string loginUser(const Member members[], int memberCount, string& loggedID);
void forgotPassword(Member members[], int memberCount);
void editMemberProfile(Member members[], int memberCount, const string& currentMemberID);
void viewMemberProfile(Member members[], int memberCount, const string& currentMemberID);
void viewAllMembers(Member members[], int& memberCount);
void viewMasterDetails(const Master masters[], int masterCount, int startIndex);
void viewMasterList(const string& role, Master masters[], int& masterCount);
void menuCustomerManagement(const string& role, Member members[], int& memberCount, const string& currentMemberID, Master masters[], int& masterCount);
void renewMembership(Member members[], int memberCount);

// ---- BMI & RECOMMENDATION SYSTEM ----
void calculateBMI();
void recommendDietPlan();
void generateWorkoutPlan();
void menuRecommendation();

// ---- REPORTING & STATISTICS ----
void menuReportsAndStatistics(const Member members[], int memberCount,
                               const GymService services[], int serviceCount,
                               const Payment paymentList[], int paymentCount);

// ---- SERVICE, SCHEDULE & BOOKING MANAGEMENT ----
void displayWeeklyClassTable(const GymService services[], int serviceCount);
void displaySingleSchedule(const GymService& srv, bool isAdminView = false);

void createBooking(const string& memberID, GymService services[], int serviceCount, Booking bookingList[], int& bookingCount, Payment paymentList[], int& paymentCount);
void viewGymService(GymService services[], int serviceCount, const string& memberID, Booking bookingList[], int& bookingCount, Payment paymentList[], int& paymentCount);
void viewMemberBooking(const string& memberID, const Booking bookingList[], int bookingCount);
void editOrCancelBooking(const string& memberID, GymService services[], int serviceCount, Booking bookingList[], int bookingCount, Payment paymentList[], int paymentCount);

void menuServiceBooking(const string& memberID, GymService services[], int serviceCount, LockerDevice lockers[], int totalLockers, Booking bookingList[], int& bookingCount, Payment paymentList[], int& paymentCount);

void addGymService(GymService services[], int& serviceCount);
void updateServiceDetails(GymService services[], int serviceCount);
void deleteGymService(GymService services[], int serviceCount, Booking bookingList[], int bookingCount, Payment paymentList[], int paymentCount);
void viewMasterSchedule(const GymService services[], int serviceCount);
void viewAllMemberBookings(const Booking bookingList[], int bookingCount);
void markAppointmentAsDone(Booking bookingList[], int bookingCount);

void menuScheduleManagement(GymService services[], int& serviceCount, Member members[], int memberCount, Booking bookingList[], int& bookingCount, LockerDevice lockers[], int totalLockers, Payment paymentList[], int paymentCount);


// ============================================================================
// SECTION: UI RENDERING & BUFFER UTILITIES (was src/UIUtils.cpp)
// ============================================================================

// Helper function to get current system date (YYYY-MM-DD)
string getCurrentDateStr() {
    time_t now = time(0);
    tm ltm;
#ifdef _WIN32
    localtime_s(&ltm, &now);
#else
    localtime_r(&now, &ltm);
#endif
    char buf[11];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d", 1900 + ltm.tm_year, 1 + ltm.tm_mon, ltm.tm_mday);
    return string(buf);
}

// Helper function to validate email format
bool isValidEmail(const string& email) {
    size_t atPos = email.find('@');
    size_t dotPos = email.rfind('.');
    return (atPos != string::npos && dotPos != string::npos && atPos < dotPos && atPos > 0 && dotPos < email.length() - 1);
}

// Helper function to validate a phone number: digits only, 10-11 digits long
bool isValidPhone(const string& phone) {
    if (phone.length() < 10 || phone.length() > 11) return false;
    for (char c : phone) {
        if (!isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

// Words only: letters, spaces, and '&' (e.g. "Yoga & Fitness"). No digits/symbols.
bool isValidSpecialization(const string& text) {
    if (text.empty()) return false;
    for (char c : text) {
        if (!isalpha(static_cast<unsigned char>(c)) && c != ' ' && c != '&') return false;
    }
    return true;
}

// Numbers only, 1-2 digits (0-99 years)
bool isValidExperience(const string& text) {
    if (text.empty() || text.length() > 2) return false;
    for (char c : text) {
        if (!isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

string readMaskedInput() {
    string input;
#ifdef _WIN32
    char ch;
    while ((ch = _getch()) != '\r' && ch != '\n') {
        if (ch == '\b') {
            if (!input.empty()) {
                input.pop_back();
                cout << "\b \b";
            }
        }
        else {
            input += ch;
            cout << '*';
        }
    }
    cout << "\n";
#else
    cin.ignore((numeric_limits<streamsize>::max)(), '\n');
    getline(cin, input);
#endif
    return input;
}

namespace {
    string g_roleLabel;
}

void setCurrentRoleLabel(const string& label) {
    g_roleLabel = label;
}

int selectFromList(const string& title, const vector<string>& items) {
    if (items.empty()) return -1;

#ifdef _WIN32
    int selected = 0;
    while (true) {
        clearScreen();
        displayAppHeader(title);
        for (size_t i = 0; i < items.size(); ++i) {
            string marker = (static_cast<int>(i) == selected) ? "> " : "  ";
            printBoxLine(marker + items[i], "left", 6);
        }
        printBoxDivider();
        printBoxLine("Up/Down = move | Enter = select | Esc = cancel", "center");
        printBoxBorder();

        int key = _getch();
        if (key == 0 || key == 224) { // extended key prefix (arrows, F-keys, ...)
            int key2 = _getch();
            if (key2 == 72) { // Up
                selected = (selected - 1 + (int)items.size()) % (int)items.size();
            }
            else if (key2 == 80) { // Down
                selected = (selected + 1) % (int)items.size();
            }
        }
        else if (key == '\r' || key == '\n') {
            return selected;
        }
        else if (key == 27) { // Esc
            return -1;
        }
    }
#else
    clearScreen();
    displayAppHeader(title);
    for (size_t i = 0; i < items.size(); ++i) {
        printBoxLine("[" + to_string(i + 1) + "] " + items[i], "left", 6);
    }
    printBoxLine("[0] Cancel", "left", 6);
    printBoxBorder();

    int choice;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select: ";
    if (!(cin >> choice) || choice < 1 || choice > (int)items.size()) return -1;
    return choice - 1;
#endif
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

int getLeftMargin() {
    int winWidth = 115;
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        winWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
#endif
    return max(0, (winWidth - BOX_WIDTH) / 2);
}

void printBoxBorder() {
    cout << string(getLeftMargin(), ' ') << "+" << string(INNER_WIDTH, '-') << "+\n";
}

void printBoxDivider() {
    cout << string(getLeftMargin(), ' ') << "|" << string(INNER_WIDTH, '-') << "|\n";
}

void printBoxLine(const string& text, const string& align, int indent) {
    string content = "";
    int len = (int)text.length();

    if (align == "center") {
        int leftPad = max(0, (INNER_WIDTH - len) / 2);
        int rightPad = max(0, INNER_WIDTH - len - leftPad);
        content = string(leftPad, ' ') + text + string(rightPad, ' ');
    }
    else if (align == "right") {
        int rightPad = indent;
        int leftPad = max(0, INNER_WIDTH - len - rightPad);
        content = string(leftPad, ' ') + text + string(rightPad, ' ');
    }
    else {
        int leftPad = indent;
        int rightPad = max(0, INNER_WIDTH - len - leftPad);
        content = string(leftPad, ' ') + text + string(rightPad, ' ');
    }

    cout << string(getLeftMargin(), ' ') << "|" << content << "|\n";
}

void displayAppHeader(const string& subtitle) {
    printBoxBorder();
    if (!g_roleLabel.empty()) {
        printBoxLine("Role: " + g_roleLabel, "right", 2);
        printBoxDivider();
    }
    printBoxLine("  ____   ___  __   __  _____ _ _                     ", "center");
    printBoxLine(" / ___| |_  | \\ \\ / / |  ___(_) |_ _ __   ___  ___ ___ ", "center");
    printBoxLine(" \\___ \\   | |  \\ V /  | |_  | | __| '_ \\ / _ \\/ __/ __|", "center");
    printBoxLine("  ___) |  | |   | |   |  _| | | |_| | | |  __/\\__ \\__ \\", "center");
    printBoxLine(" |____/__/ |   |_|   |_|   |_|\\__|_| |_|\\___||___/___/", "center");
    printBoxLine(" ", "center");
    printBoxLine("SJY FITNESS - SERVICE MANAGEMENT SYSTEM", "center");
    printBoxLine("Your Health, Our Passion", "center");
    printBoxDivider();
    printBoxLine(">> " + subtitle + " <<", "center");
    printBoxDivider();
}

void pauseScreen() {
    cout << "\n" << string(getLeftMargin(), ' ') << ">> Press Enter to continue...";
    cin.clear();
    cin.ignore((numeric_limits<streamsize>::max)(), '\n');
    cin.get();
}


// ============================================================================
// SECTION: FILE-BASED DATA PERSISTENCE (was src/DataStore.cpp)
// ============================================================================
namespace {
    const string DATA_DIR = "data";

    // Manual split that preserves trailing empty fields (e.g. "a|b|" -> ["a","b",""]),
    // which stringstream+getline drops.
    vector<string> splitFields(const string& line, char delim) {
        vector<string> fields;
        size_t start = 0;
        while (true) {
            size_t pos = line.find(delim, start);
            if (pos == string::npos) {
                fields.push_back(line.substr(start));
                break;
            }
            fields.push_back(line.substr(start, pos - start));
            start = pos + 1;
        }
        return fields;
    }

    string joinInts(const int arr[], int count) {
        string out;
        for (int i = 0; i < count; ++i) {
            if (i > 0) out += ",";
            out += to_string(arr[i]);
        }
        return out;
    }

    void splitInts(const string& field, int arr[], int count) {
        int idx = 0;
        size_t start = 0;
        while (idx < count) {
            size_t pos = field.find(',', start);
            string val = (pos == string::npos) ? field.substr(start) : field.substr(start, pos - start);
            arr[idx++] = val.empty() ? 0 : stoi(val);
            if (pos == string::npos) break;
            start = pos + 1;
        }
    }
}

namespace DataStore {

void ensureDataDir() {
    filesystem::create_directories(DATA_DIR);
}

// ---------------- Members ----------------
void saveMembers(const Member members[], int memberCount) {
    ensureDataDir();
    ofstream out(DATA_DIR + "/members.dat");
    for (int i = 0; i < memberCount; ++i) {
        out << members[i].memberID << "|" << members[i].fullName << "|" << members[i].password << "|"
            << members[i].expiryDate << "|" << members[i].email << "|" << members[i].phone << "|"
            << members[i].points << "\n";
    }
}

int loadMembers(Member members[], int maxMembers) {
    ifstream in(DATA_DIR + "/members.dat");
    if (!in.is_open()) return 0;
    int count = 0;
    string line;
    while (count < maxMembers && getline(in, line)) {
        if (line.empty()) continue;
        vector<string> f = splitFields(line, '|');
        if (f.size() < 6) continue;
        Member& m = members[count];
        m.memberID = f[0];
        m.fullName = f[1];
        m.password = f[2];
        m.expiryDate = f[3];
        m.email = f[4];
        m.phone = f[5];
        // Files saved before the "points" field was added have 6 fields
        // (no points); default those to 0.
        m.points = (f.size() >= 7) ? stoi(f[6]) : 0;
        count++;
    }
    return count;
}

// ---------------- Masters ----------------
void saveMasters(const Master masters[], int masterCount) {
    ensureDataDir();
    ofstream out(DATA_DIR + "/masters.dat");
    for (int i = 0; i < masterCount; ++i) {
        out << masters[i].masterID << "|" << masters[i].name << "|" << masters[i].specialization << "|"
            << masters[i].experience << "|" << masters[i].rating << "\n";
    }
}

int loadMasters(Master masters[], int maxMasters) {
    ifstream in(DATA_DIR + "/masters.dat");
    if (!in.is_open()) return 0;
    int count = 0;
    string line;
    while (count < maxMasters && getline(in, line)) {
        if (line.empty()) continue;
        vector<string> f = splitFields(line, '|');
        if (f.size() < 5) continue;
        masters[count] = { f[0], f[1], f[2], stoi(f[3]), stod(f[4]) };
        count++;
    }
    return count;
}

// ---------------- Gym Services ----------------
void saveServices(const GymService services[], int serviceCount) {
    ensureDataDir();
    ofstream out(DATA_DIR + "/services.dat");
    for (int i = 0; i < serviceCount; ++i) {
        out << services[i].serviceID << "|" << services[i].name << "|" << services[i].category << "|"
            << (services[i].isActive ? 1 : 0) << "|"
            << services[i].price << "|"
            << joinInts(&services[i].capacity[0][0], DAYS * SLOTS) << "|"
            << joinInts(&services[i].booked[0][0], DAYS * SLOTS) << "\n";
    }
}

int loadServices(GymService services[], int maxServices) {
    ifstream in(DATA_DIR + "/services.dat");
    if (!in.is_open()) return 0;
    int count = 0;
    string line;
    while (count < maxServices && getline(in, line)) {
        if (line.empty()) continue;
        vector<string> f = splitFields(line, '|');
        if (f.size() < 6) continue;
        GymService& s = services[count];
        s.serviceID = f[0];
        s.name = f[1];
        s.category = f[2];
        s.isActive = (f[3] == "1");
        // Files saved before the "price" field was added have 6 fields
        // (no price); default those to 0 so an admin can set it via Update Service.
        if (f.size() >= 7) {
            s.price = stof(f[4]);
            splitInts(f[5], &s.capacity[0][0], DAYS * SLOTS);
            splitInts(f[6], &s.booked[0][0], DAYS * SLOTS);
        }
        else {
            s.price = 0.0f;
            splitInts(f[4], &s.capacity[0][0], DAYS * SLOTS);
            splitInts(f[5], &s.booked[0][0], DAYS * SLOTS);
        }
        count++;
    }
    return count;
}

// ---------------- Bookings ----------------
void saveBookings(const Booking bookingList[], int bookingCount) {
    ensureDataDir();
    ofstream out(DATA_DIR + "/bookings.dat");
    for (int i = 0; i < bookingCount; ++i) {
        out << bookingList[i].bookingID << "|" << bookingList[i].memberID << "|" << bookingList[i].category << "|"
            << bookingList[i].targetName << "|" << bookingList[i].dayIndex << "|" << bookingList[i].slotIndex << "|"
            << (bookingList[i].isActive ? 1 : 0) << "|" << bookingList[i].status << "\n";
    }
}

int loadBookings(Booking bookingList[], int maxBookings) {
    ifstream in(DATA_DIR + "/bookings.dat");
    if (!in.is_open()) return 0;
    int count = 0;
    string line;
    while (count < maxBookings && getline(in, line)) {
        if (line.empty()) continue;
        vector<string> f = splitFields(line, '|');
        if (f.size() < 8) continue;
        bookingList[count] = { f[0], f[1], f[2], f[3], stoi(f[4]), stoi(f[5]), f[6] == "1", f[7] };
        count++;
    }
    return count;
}

// ---------------- Lockers ----------------
void saveLockers(const LockerDevice lockers[], int totalLockers) {
    ensureDataDir();
    ofstream out(DATA_DIR + "/lockers.dat");
    for (int i = 0; i < totalLockers; ++i) {
        out << lockers[i].lockerNo << "|" << lockers[i].lockerType << "|"
            << (lockers[i].isOccupied ? 1 : 0) << "|" << lockers[i].occupiedBy << "\n";
    }
}

int loadLockers(LockerDevice lockers[], int totalLockers) {
    ifstream in(DATA_DIR + "/lockers.dat");
    if (!in.is_open()) return 0;
    int count = 0;
    string line;
    while (count < totalLockers && getline(in, line)) {
        if (line.empty()) continue;
        vector<string> f = splitFields(line, '|');
        if (f.size() < 4) continue;
        lockers[count] = { stoi(f[0]), f[1], f[2] == "1", f[3] };
        count++;
    }
    return count;
}

// ---------------- Payments ----------------
void savePayments(const Payment paymentList[], int paymentCount) {
    ensureDataDir();
    ofstream out(DATA_DIR + "/payments.dat");
    for (int i = 0; i < paymentCount; ++i) {
        const Payment& p = paymentList[i];
        out << p.paymentID << "|" << p.memberID << "|" << p.bookingID << "|" << p.serviceType << "|" << p.subtotal << "|"
            << p.discountAmount << "|" << p.sstAmount << "|" << p.totalAmount << "|" << p.status << "|"
            << p.paymentMethod << "|" << p.cashReceived << "|" << p.changeGiven << "|"
            << p.day << "|" << p.month << "|" << p.year << "|" << p.confirmedBy << "\n";
    }
}

int loadPayments(Payment paymentList[], int maxPayments) {
    ifstream in(DATA_DIR + "/payments.dat");
    if (!in.is_open()) return 0;
    int count = 0;
    string line;
    while (count < maxPayments && getline(in, line)) {
        if (line.empty()) continue;
        vector<string> f = splitFields(line, '|');
        if (f.size() < 15) continue;
        Payment& p = paymentList[count];
        // Files saved before the "bookingID" field was added have 15 fields
        // (no bookingID); default those to "" (manual/legacy charge).
        int off = (f.size() >= 16) ? 1 : 0;
        p.paymentID = stoi(f[0]);
        p.memberID = f[1];
        p.bookingID = off ? f[2] : "";
        p.serviceType = f[2 + off];
        p.subtotal = stof(f[3 + off]);
        p.discountAmount = stof(f[4 + off]);
        p.sstAmount = stof(f[5 + off]);
        p.totalAmount = stof(f[6 + off]);
        p.status = f[7 + off];
        p.paymentMethod = f[8 + off];
        p.cashReceived = stof(f[9 + off]);
        p.changeGiven = stof(f[10 + off]);
        p.day = stoi(f[11 + off]);
        p.month = stoi(f[12 + off]);
        p.year = stoi(f[13 + off]);
        p.confirmedBy = f[14 + off];
        count++;
    }
    return count;
}

// ---------------- Gifts ----------------
void saveGifts(const Gift giftList[], int giftCount) {
    ensureDataDir();
    ofstream out(DATA_DIR + "/gifts.dat");
    for (int i = 0; i < giftCount; ++i) {
        out << giftList[i].giftID << "|" << giftList[i].name << "|" << giftList[i].pointCost << "|"
            << giftList[i].status << "\n";
    }
}

int loadGifts(Gift giftList[], int maxGifts) {
    ifstream in(DATA_DIR + "/gifts.dat");
    if (!in.is_open()) return 0;
    int count = 0;
    string line;
    while (count < maxGifts && getline(in, line)) {
        if (line.empty()) continue;
        vector<string> f = splitFields(line, '|');
        if (f.size() < 4) continue;
        giftList[count] = { f[0], f[1], stoi(f[2]), f[3] };
        count++;
    }
    return count;
}

} // namespace DataStore


// ============================================================================
// SECTION: BILLING & PAYMENT MANAGEMENT (was src/BillingModule.cpp)
// ============================================================================
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


// ============================================================================
// SECTION: LOCKER SERVICE (was src/LockerModule.cpp)
// ============================================================================
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


// ============================================================================
// SECTION: LOYALTY & GIFT MANAGEMENT (was src/LoyaltyModule.cpp)
// ============================================================================
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


// ============================================================================
// SECTION: MEMBER & COACH MANAGEMENT (was src/MemberModule.cpp)
// ============================================================================

// Auto-generate the next "M####" member ID, based on the highest existing ID
// (not just memberCount), so removing then adding members never reissues an ID.
static string generateNextMemberID(const Member members[], int memberCount) {
    int maxNum = 1000;
    for (int i = 0; i < memberCount; ++i) {
        const string& id = members[i].memberID;
        if (id.size() > 1 && id[0] == 'M') {
            try {
                int num = stoi(id.substr(1));
                if (num > maxNum) maxNum = num;
            }
            catch (...) {
                // Non-numeric suffix (e.g. legacy/custom ID) - ignore it.
            }
        }
    }
    return "M" + to_string(maxNum + 1);
}

// 5.1.1 Register Member
void registerMember(Member members[], int& memberCount) {
    clearScreen();
    displayAppHeader("REGISTER NEW MEMBER");

    if (memberCount >= MAX_MEMBERS) {
        printBoxLine("[!] Member database is full.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    string fullName, password, email, phone;
    printBoxLine("=== REGISTER NEW MEMBER ===", "left", 6);
    printBoxLine("(Enter 0 as your name at any time to cancel and go back)", "left", 6);
    printBoxBorder();

    cout << string(getLeftMargin() + 6, ' ') << "Enter Full Name           : ";
    cin >> fullName;
    if (fullName == "0") {
        return;
    }

    cout << string(getLeftMargin() + 6, ' ') << "Enter Password            : ";
    password = readMaskedInput();

    while (true) {
        cout << string(getLeftMargin() + 6, ' ') << "Enter Email               : ";
        cin >> email;
        if (isValidEmail(email)) break;
        cout << string(getLeftMargin() + 6, ' ') << "[ERROR] Invalid email format! (e.g., user@example.com)\n";
    }

    while (true) {
        cout << string(getLeftMargin() + 6, ' ') << "Enter Contact Number      : ";
        cin >> phone;
        if (isValidPhone(phone)) break;
        cout << string(getLeftMargin() + 6, ' ') << "[ERROR] Phone number must be 10-11 digits, numbers only.\n";
    }

    string newID = generateNextMemberID(members, memberCount);
    members[memberCount] = { newID, fullName, password, "2026-12-31", email, phone, 0 };
    memberCount++;
    DataStore::saveMembers(members, memberCount);

    clearScreen();
    displayAppHeader("REGISTRATION SUCCESSFUL");
    printBoxLine("Member " + fullName + " registered successfully!", "center");
    printBoxLine("Your Member ID: " + newID, "center");
    printBoxBorder();
    pauseScreen();
}

// 5.1.3 Forgot Password (Added exist check)
void forgotPassword(Member members[], int memberCount) {
    clearScreen();
    displayAppHeader("FORGOT PASSWORD");

    string email;
    cout << string(getLeftMargin() + 6, ' ') << "Enter your registered Email: ";
    cin >> email;

    int foundIdx = -1;
    for (int i = 0; i < memberCount; ++i) {
        if (members[i].email == email) {
            foundIdx = i;
            break;
        }
    }

    if (foundIdx == -1) {
        clearScreen();
        displayAppHeader("ERROR");
        printBoxLine("[ERROR] Registered email not found!", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    string code, newPassword;
    cout << string(getLeftMargin() + 6, ' ') << "Enter Security Verification Code sent to email: ";
    cin >> code;
    cout << string(getLeftMargin() + 6, ' ') << "Enter New Password: ";
    newPassword = readMaskedInput();

    members[foundIdx].password = newPassword;
    DataStore::saveMembers(members, memberCount);

    clearScreen();
    displayAppHeader("PASSWORD UPDATED");
    printBoxLine("Password updated successfully! Please login with your new password.", "center");
    printBoxBorder();
    pauseScreen();
}

// 5.1.2 Login User (Refactored to handle Admin & Customer verification)
string loginUser(const Member members[], int memberCount, string& loggedID) {
    setCurrentRoleLabel(""); // no role while on the login screen (e.g. after a logout)
    while (true) {
        clearScreen();
        displayAppHeader("LOGIN SYSTEM");
        printBoxLine("[1] Login", "left", 6);
        printBoxLine("[2] Forgot Password", "left", 6);
        printBoxLine("[0] Exit System", "left", 6);
        printBoxBorder();

        int choice;
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select option: ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        if (choice == 1) {
            string username, password;
            cout << string(getLeftMargin() + 6, ' ') << "Enter User ID / Username: ";
            cin >> username;
            cout << string(getLeftMargin() + 6, ' ') << "Enter Password          : ";
            password = readMaskedInput();

            // Check Admin Credentials
            if (username == "admin" && password == "admin123") {
                setCurrentRoleLabel("ADMIN");
                clearScreen();
                displayAppHeader("LOGIN SUCCESS");
                printBoxLine("Login Successful as Admin.", "center");
                printBoxBorder();
                pauseScreen();
                loggedID = "admin";
                return "admin";
            }

            // Check Customer / Member Credentials
            for (int i = 0; i < memberCount; ++i) {
                if ((members[i].memberID == username || members[i].fullName == username) && members[i].password == password) {
                    setCurrentRoleLabel("CUSTOMER (" + members[i].memberID + ")");
                    clearScreen();
                    displayAppHeader("LOGIN SUCCESS");
                    printBoxLine("Login Successful as Customer.", "center");
                    printBoxBorder();
                    pauseScreen();
                    loggedID = members[i].memberID;
                    return "customer";
                }
            }

            clearScreen();
            displayAppHeader("LOGIN FAILED");
            printBoxLine("[ERROR] Invalid username or password! Please try again.", "center");
            printBoxBorder();
            pauseScreen();
        }
        else if (choice == 2) {
            forgotPassword(const_cast<Member*>(members), memberCount);
        }
        else if (choice == 0) {
            return "";
        }
    }
}

// 5.1.5 Edit Member Profile fields, given a resolved index (shared by
// self-service "Edit Profile" and admin "Edit Member" from the members list)
static void editMemberFields(Member members[], int memberCount, int mIdx) {
    clearScreen();
    displayAppHeader("EDIT MEMBER PROFILE");
    printBoxLine("Editing: " + members[mIdx].fullName + " (" + members[mIdx].memberID + ")", "left", 6);
    printBoxDivider();
    printBoxLine("[1] Update Email", "left", 6);
    printBoxLine("[2] Update Contact Number", "left", 6);
    printBoxLine("[3] Change Password", "left", 6);
    printBoxLine("[0] Cancel", "left", 6);
    printBoxBorder();

    int choice;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select choice: ";
    cin >> choice;

    if (choice == 1) {
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Current Email: " << members[mIdx].email << "\n";
        string newEmail;
        cout << string(getLeftMargin() + 6, ' ') << "Enter New Email: ";
        cin >> newEmail;

        if (!isValidEmail(newEmail)) {
            cout << "\n" << string(getLeftMargin() + 6, ' ') << "[ERROR] Invalid email format! (e.g., user@example.com)\n";
        } else if (newEmail == members[mIdx].email) {
            cout << "\n" << string(getLeftMargin() + 6, ' ') << "[ERROR] New email must be different from the current email.\n";
        } else {
            members[mIdx].email = newEmail;
            DataStore::saveMembers(members, memberCount);
            cout << "\n" << string(getLeftMargin() + 6, ' ') << "Email updated to: " << newEmail << "\n";
        }
        pauseScreen();
    }
    else if (choice == 2) {
        string newPhone;
        cout << string(getLeftMargin() + 6, ' ') << "Enter New Contact Number: ";
        cin >> newPhone;
        members[mIdx].phone = newPhone;
        DataStore::saveMembers(members, memberCount);
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Contact Number updated to: " << newPhone << "\n";
        pauseScreen();
    }
    else if (choice == 3) {
        string newPass;
        cout << string(getLeftMargin() + 6, ' ') << "Enter New Password: ";
        newPass = readMaskedInput();
        members[mIdx].password = newPass;
        DataStore::saveMembers(members, memberCount);
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Password changed successfully.\n";
        pauseScreen();
    }
    else if (choice == 0) {
        return;
    }
}

void editMemberProfile(Member members[], int memberCount, const string& currentMemberID) {
    int mIdx = -1;
    for (int i = 0; i < memberCount; ++i) {
        if (members[i].memberID == currentMemberID) {
            mIdx = i;
            break;
        }
    }
    if (mIdx == -1) {
        clearScreen();
        displayAppHeader("ERROR");
        printBoxLine("[ERROR] Profile data not found.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }
    editMemberFields(members, memberCount, mIdx);
}

// Edit an arbitrary member picked from the All Members List (admin action):
// show the full member list, then let the admin type which Member ID to edit.
static void editMemberByID(Member members[], int memberCount) {
    clearScreen();
    displayAppHeader("EDIT MEMBER");

    if (memberCount == 0) {
        printBoxLine("[!] No members to edit.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    for (int i = 0; i < memberCount; ++i) {
        printBoxLine("ID: " + members[i].memberID + " | Name: " + members[i].fullName, "left", 6);
    }
    printBoxBorder();

    string targetID;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter Member ID to Edit (or '0' to Cancel): ";
    cin >> targetID;

    if (targetID == "0") return;

    int mIdx = -1;
    for (int i = 0; i < memberCount; ++i) {
        if (members[i].memberID == targetID) { mIdx = i; break; }
    }

    if (mIdx == -1) {
        clearScreen();
        displayAppHeader("NOT FOUND");
        printBoxLine("[ERROR] Member ID does not exist.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    editMemberFields(members, memberCount, mIdx);
}

// 5.1.4 View Member Profile
void viewMemberProfile(Member members[], int memberCount, const string& currentMemberID) {
    clearScreen();
    displayAppHeader("MY PROFILE");

    int mIdx = -1;
    for (int i = 0; i < memberCount; ++i) {
        if (members[i].memberID == currentMemberID) {
            mIdx = i;
            break;
        }
    }

    if (mIdx != -1) {
        printBoxLine("ID    : " + members[mIdx].memberID, "left", 6);
        printBoxLine("Name  : " + members[mIdx].fullName, "left", 6);
        printBoxLine("Email : " + members[mIdx].email, "left", 6);
        printBoxLine("Phone : " + members[mIdx].phone, "left", 6);
        printBoxLine("Membership Status: Active", "left", 6);
    }
    else {
        printBoxLine("[ERROR] Profile data not found.", "center");
    }

    printBoxDivider();
    printBoxLine("[1] Edit Profile", "left", 6);
    printBoxLine("[0] Back", "left", 6);
    printBoxBorder();

    int choice;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select choice: ";
    cin >> choice;

    if (choice == 1) {
        editMemberProfile(members, memberCount, currentMemberID);
    }
    else if (choice == 0) {
        return;
    }
}

// 5.1.7 Remove Member (used by viewAllMembers) - arrow-key select, then confirm
static void removeMember(Member members[], int& memberCount) {
    if (memberCount == 0) {
        clearScreen();
        displayAppHeader("REMOVE MEMBER");
        printBoxLine("[!] No members to remove.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    vector<string> options;
    for (int i = 0; i < memberCount; ++i) {
        options.push_back(members[i].memberID + " - " + members[i].fullName);
    }
    int selIdx = selectFromList("SELECT MEMBER TO REMOVE", options);
    if (selIdx == -1) return; // canceled - back to the members list

    string memberID = members[selIdx].memberID;

    clearScreen();
    displayAppHeader("REMOVE MEMBER");
    cout << string(getLeftMargin() + 6, ' ') << "Are you sure you want to remove Member ID " << memberID << "? (Y/N): ";
    char confirm;
    cin >> confirm;
    if (confirm == 'Y' || confirm == 'y') {
        bool found = false;
        for (int i = 0; i < memberCount; ++i) {
            if (members[i].memberID == memberID) {
                for (int j = i; j < memberCount - 1; ++j) {
                    members[j] = members[j + 1];
                }
                memberCount--;
                found = true;
                break;
            }
        }
        if (found) {
            DataStore::saveMembers(members, memberCount);
            cout << string(getLeftMargin() + 6, ' ') << "Member ID " << memberID << " removed successfully.\n";
        }
        else {
            cout << string(getLeftMargin() + 6, ' ') << "Member ID not found.\n";
        }
    }
    else {
        cout << string(getLeftMargin() + 6, ' ') << "Removal cancelled.\n";
    }
    pauseScreen();
}

// 5.1.6 View All Members
void viewAllMembers(Member members[], int& memberCount) {
    int choice;
    do {
        clearScreen();
        displayAppHeader("ALL MEMBERS LIST");

        for (int i = 0; i < memberCount; ++i) {
            printBoxLine("ID: " + members[i].memberID + " | Name: " + members[i].fullName + " | Status: Active", "left", 6);
        }
        printBoxDivider();
        printBoxLine("[1] Add Member", "left", 6);
        printBoxLine("[2] Remove Member", "left", 6);
        printBoxLine("[3] Edit Member", "left", 6);
        printBoxLine("[0] Back", "left", 6);
        printBoxBorder();

        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select choice: ";
        cin >> choice;

        if (choice == 1) {
            registerMember(members, memberCount);
        }
        else if (choice == 2) {
            removeMember(members, memberCount);
        }
        else if (choice == 3) {
            editMemberByID(members, memberCount);
        }
    } while (choice != 0);
}

// Auto-generate the next "MS###" master ID, based on the highest existing ID
static string generateNextMasterID(const Master masters[], int masterCount) {
    int maxNum = 0;
    for (int i = 0; i < masterCount; ++i) {
        const string& id = masters[i].masterID;
        if (id.size() > 2 && id[0] == 'M' && id[1] == 'S') {
            try {
                int num = stoi(id.substr(2));
                if (num > maxNum) maxNum = num;
            }
            catch (...) {
                // Non-numeric suffix - ignore it.
            }
        }
    }
    string numStr = to_string(maxNum + 1);
    while (numStr.length() < 3) numStr = "0" + numStr;
    return "MS" + numStr;
}

// 5.1.10a Add Master (Admin only) - now lives on the master list screen, not
// inside master details.
static void addMaster(Master masters[], int& masterCount) {
    clearScreen();
    displayAppHeader("ADD NEW MASTER");

    if (masterCount >= MAX_MASTERS) {
        printBoxLine("[!] Master limit reached.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    cin.ignore(10000, '\n');
    string masterName, specialization, expStr;
    cout << string(getLeftMargin() + 6, ' ') << "Enter Master Name (or 0 to cancel): ";
    getline(cin, masterName);
    if (masterName == "0") return;

    while (true) {
        cout << string(getLeftMargin() + 6, ' ') << "Enter Specialization (letters only): ";
        getline(cin, specialization);
        if (isValidSpecialization(specialization)) break;
        cout << string(getLeftMargin() + 6, ' ') << "[ERROR] Specialization must contain letters only.\n";
    }

    int experience = 0;
    while (true) {
        cout << string(getLeftMargin() + 6, ' ') << "Enter Experience (Years, numbers only): ";
        getline(cin, expStr);
        if (isValidExperience(expStr)) {
            experience = stoi(expStr);
            break;
        }
        cout << string(getLeftMargin() + 6, ' ') << "[ERROR] Experience must be a number (0-99).\n";
    }

    string newID = generateNextMasterID(masters, masterCount);
    masters[masterCount] = { newID, masterName, specialization, experience, 5.0 };
    masterCount++;
    DataStore::saveMasters(masters, masterCount);

    clearScreen();
    displayAppHeader("MASTER ADDED");
    printBoxLine("[SUCCESS] Master " + masterName + " (" + newID + ") added successfully!", "center");
    printBoxBorder();
    pauseScreen();
}

// 5.1.10b Remove Master (Admin only) - arrow-key select, then confirm; Esc/
// canceling the picker just returns to the master list (no ID typing needed).
static void removeMaster(Master masters[], int& masterCount) {
    if (masterCount == 0) {
        clearScreen();
        displayAppHeader("REMOVE MASTER");
        printBoxLine("[!] No masters to remove.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    vector<string> options;
    for (int i = 0; i < masterCount; ++i) {
        options.push_back(masters[i].masterID + " - " + masters[i].name);
    }
    int idx = selectFromList("SELECT MASTER TO REMOVE", options);
    if (idx == -1) return; // canceled - back to the master list

    clearScreen();
    displayAppHeader("REMOVE MASTER");
    cout << string(getLeftMargin() + 6, ' ') << "Remove " << masters[idx].name << " (" << masters[idx].masterID << ")? (Y/N): ";
    char confirm;
    cin >> confirm;
    if (confirm == 'Y' || confirm == 'y') {
        for (int j = idx; j < masterCount - 1; ++j) {
            masters[j] = masters[j + 1];
        }
        masterCount--;
        DataStore::saveMasters(masters, masterCount);
        cout << string(getLeftMargin() + 6, ' ') << "Master removed successfully.\n";
    }
    else {
        cout << string(getLeftMargin() + 6, ' ') << "Removal cancelled.\n";
    }
    pauseScreen();
}

// 5.1.9 View Master Details - Left/Right arrows cycle between masters
void viewMasterDetails(const Master masters[], int masterCount, int startIndex) {
    if (masterCount == 0) {
        clearScreen();
        displayAppHeader("MASTER DETAILS");
        printBoxLine("[!] No masters available.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    int idx = (startIndex >= 0 && startIndex < masterCount) ? startIndex : 0;

    while (true) {
        clearScreen();
        displayAppHeader("MASTER DETAILS (" + to_string(idx + 1) + "/" + to_string(masterCount) + ")");
        printBoxLine("Master ID     : " + masters[idx].masterID, "left", 6);
        printBoxLine("Name          : " + masters[idx].name, "left", 6);
        printBoxLine("Specialization: " + masters[idx].specialization, "left", 6);
        printBoxLine("Experience    : " + to_string(masters[idx].experience) + " Years", "left", 6);
        printBoxLine("Rating        : " + to_string(masters[idx].rating).substr(0, 3) + " / 5.0", "left", 6);
        printBoxDivider();
        printBoxLine("Left/Right = switch master | 0 = Back", "center");
        printBoxBorder();

#ifdef _WIN32
        int key = _getch();
        if (key == 0 || key == 224) {
            int key2 = _getch();
            if (key2 == 75) { // Left
                idx = (idx - 1 + masterCount) % masterCount;
            }
            else if (key2 == 77) { // Right
                idx = (idx + 1) % masterCount;
            }
        }
        else if (key == 27 || key == '0') {
            return;
        }
#else
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "[1] Next  [2] Previous  [0] Back: ";
        int choice;
        cin >> choice;
        if (choice == 1) idx = (idx + 1) % masterCount;
        else if (choice == 2) idx = (idx - 1 + masterCount) % masterCount;
        else if (choice == 0) return;
#endif
    }
}

// 5.1.8 View Master List
void viewMasterList(const string& role, Master masters[], int& masterCount) {
    bool isAdmin = (role == "admin");
    int choice;
    do {
        clearScreen();
        displayAppHeader("MASTER LIST");

        for (int i = 0; i < masterCount; ++i) {
            printBoxLine(to_string(i + 1) + ". Master ID: " + masters[i].masterID + " | Name: " + masters[i].name + " | Specialization: " + masters[i].specialization, "left", 6);
        }
        printBoxDivider();
        printBoxLine("[1] View Master Details", "left", 6);
        if (isAdmin) {
            printBoxLine("[2] Add Master", "left", 6);
            printBoxLine("[3] Remove Master", "left", 6);
        }
        printBoxLine("[0] Back", "left", 6);
        printBoxBorder();

        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select choice: ";
        cin >> choice;

        if (choice == 1) {
            viewMasterDetails(masters, masterCount, 0);
        }
        else if (choice == 2 && isAdmin) {
            addMaster(masters, masterCount);
        }
        else if (choice == 3 && isAdmin) {
            removeMaster(masters, masterCount);
        }
    } while (choice != 0);
}

// Menu Customer Management (Submenu for Member & Master operations)
void menuCustomerManagement(const string& role, Member members[], int& memberCount, const string& currentMemberID, Master masters[], int& masterCount) {
    bool isAdmin = (role == "admin");
    int choice;
    do {
        clearScreen();
        displayAppHeader("MEMBER MANAGEMENT MENU");
        if (isAdmin) {
            printBoxLine("[1] View All Members List", "left", 6);
        }
        else {
            printBoxLine("[1] View My Profile", "left", 6);
        }
        printBoxLine("[2] View Master List", "left", 6);
        printBoxLine("[0] Back", "left", 6);
        printBoxBorder();

        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select choice: ";
        cin >> choice;

        if (choice == 1) {
            if (isAdmin) {
                viewAllMembers(members, memberCount);
            }
            else {
                viewMemberProfile(members, memberCount, currentMemberID);
            }
        }
        else if (choice == 2) {
            viewMasterList(role, masters, masterCount);
        }
        else if (choice == 0) {
            return;
        }
    } while (choice != 0);
}

// 5.1.11 Renew Membership (Admin action on a member's expiry date)
void renewMembership(Member members[], int memberCount) {
    clearScreen();
    displayAppHeader("RENEW MEMBERSHIP");

    string today = getCurrentDateStr();

    printBoxLine("--- CURRENT MEMBER STATUS & EXPIRY LIST ---", "center");
    printBoxLine("System Today's Date: " + today, "center");
    printBoxDivider();
    string header = " Member ID | Full Name        | Expiry Date   | Status";
    printBoxLine(header, "left", 6);
    printBoxDivider();

    for (int i = 0; i < memberCount; ++i) {
        string status = (members[i].expiryDate >= today) ? "ACTIVE" : "EXPIRED";
        string row = " " + members[i].memberID + string(10 - (int)members[i].memberID.length(), ' ') + "| "
            + members[i].fullName + string(17 - (int)members[i].fullName.length(), ' ') + "| "
            + members[i].expiryDate + string(14 - (int)members[i].expiryDate.length(), ' ') + "| "
            + status;
        printBoxLine(row, "left", 6);
    }
    printBoxBorder();

    string targetID;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter Member ID to Renew (or '0' to Cancel): ";
    cin >> targetID;

    if (targetID == "0") return;

    int mIdx = -1;
    for (int i = 0; i < memberCount; ++i) {
        if (members[i].memberID == targetID) { mIdx = i; break; }
    }

    if (mIdx == -1) {
        clearScreen();
        displayAppHeader("NOT FOUND");
        printBoxLine("[ERROR] Member ID does not exist.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    string newExpiry;
    cout << string(getLeftMargin() + 6, ' ') << "Enter New Expiry Date (YYYY-MM-DD): ";
    cin >> newExpiry;

    if (newExpiry.length() != 10 || newExpiry[4] != '-' || newExpiry[7] != '-') {
        clearScreen();
        displayAppHeader("INVALID FORMAT");
        printBoxLine("[ERROR] Invalid date format! Please use YYYY-MM-DD.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    if (newExpiry <= today) {
        clearScreen();
        displayAppHeader("INVALID DATE");
        printBoxLine("[ERROR] Expiry date MUST be a future date!", "center");
        printBoxLine("Today is: " + today + " | Input date: " + newExpiry, "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    members[mIdx].expiryDate = newExpiry;
    DataStore::saveMembers(members, memberCount);

    clearScreen();
    displayAppHeader("RENEWAL SUCCESSFUL");
    printBoxLine(" ", "center");
    printBoxLine("[SUCCESS] Membership renewed for " + members[mIdx].fullName + " (" + targetID + ")", "center");
    printBoxLine("Previous Expiry Date : " + today, "left", 20);
    printBoxLine("Updated Expiry Date  : " + newExpiry, "left", 20);
    printBoxLine("Account Status       : ACTIVE", "left", 20);
    printBoxLine(" ", "center");
    printBoxBorder();
    pauseScreen();
}


// ============================================================================
// SECTION: BMI & RECOMMENDATION SYSTEM (was src/RecommendationModule.cpp)
// ============================================================================

// 5.6.1 Calculate BMI
void calculateBMI() {
    clearScreen();
    displayAppHeader("CALCULATE BMI");

    double weight, height;
    cout << string(getLeftMargin() + 6, ' ') << "Enter weight (kg): ";
    cin >> weight;
    cout << string(getLeftMargin() + 6, ' ') << "Enter height (m) : ";
    cin >> height;

    if (height > 0) {
        double bmi = weight / (height * height);
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Your BMI is: " << fixed << setprecision(2) << bmi << "\n";

        if (bmi < 18.5) {
            cout << string(getLeftMargin() + 6, ' ') << "Category: Underweight\n";
        }
        else if (bmi >= 18.5 && bmi < 24.9) {
            cout << string(getLeftMargin() + 6, ' ') << "Category: Normal Weight\n";
        }
        else if (bmi >= 25.0 && bmi < 29.9) {
            cout << string(getLeftMargin() + 6, ' ') << "Category: Overweight\n";
        }
        else {
            cout << string(getLeftMargin() + 6, ' ') << "Category: Obese\n";
        }
    }
    else {
        cout << string(getLeftMargin() + 6, ' ') << "Invalid height.\n";
    }
    pauseScreen();
}

// 5.6.2 Recommend Diet Plan
void recommendDietPlan() {
    clearScreen();
    displayAppHeader("RECOMMEND DIET PLAN");
    printBoxLine("Select your fitness goal:", "left", 6);
    printBoxLine("[1] Weight Loss", "left", 6);
    printBoxLine("[2] Muscle Gain", "left", 6);
    printBoxLine("[3] Maintenance", "left", 6);
    printBoxBorder();

    int goal;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select goal: ";
    cin >> goal;

    if (goal == 1) {
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Diet Plan: High Protein, Low Carb (Caloric Deficit)\n";
        cout << string(getLeftMargin() + 6, ' ') << "Meal Recommendation: Salad, Grilled Chicken, Oatmeal\n";
    }
    else if (goal == 2) {
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Diet Plan: High Protein, High Carb (Caloric Surplus)\n";
        cout << string(getLeftMargin() + 6, ' ') << "Meal Recommendation: Eggs, Rice, Beef, Protein Shakes\n";
    }
    else if (goal == 3) {
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Diet Plan: Balanced Nutrition\n";
        cout << string(getLeftMargin() + 6, ' ') << "Meal Recommendation: Mixed Vegetables, Whole Grains, Fish\n";
    }
    else {
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Invalid selection.\n";
    }
    pauseScreen();
}

// 5.6.3 Generate Workout Plan
void generateWorkoutPlan() {
    clearScreen();
    displayAppHeader("GENERATE WORKOUT PLAN");
    printBoxLine("Select your fitness level:", "left", 6);
    printBoxLine("[1] Beginner", "left", 6);
    printBoxLine("[2] Intermediate", "left", 6);
    printBoxLine("[3] Advanced", "left", 6);
    printBoxBorder();

    int level;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select level: ";
    cin >> level;

    if (level == 1) {
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Plan: 3 Days/Week Full Body Workout & Light Cardio\n";
    }
    else if (level == 2) {
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Plan: 4 Days/Week Upper/Lower Split & HIIT Cardio\n";
    }
    else if (level == 3) {
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Plan: 5-6 Days/Week Push/Pull/Legs Split\n";
    }
    else {
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Invalid selection.\n";
    }
    pauseScreen();
}

// Menu Recommendation
void menuRecommendation() {
    int choice;
    do {
        clearScreen();
        displayAppHeader("BMI & RECOMMENDATION MENU");
        printBoxLine("[1] Calculate BMI", "left", 6);
        printBoxLine("[2] Recommend Diet Plan", "left", 6);
        printBoxLine("[3] Generate Workout Plan", "left", 6);
        printBoxLine("[0] Back", "left", 6);
        printBoxBorder();

        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select choice: ";
        cin >> choice;

        if (choice == 1) {
            calculateBMI();
        }
        else if (choice == 2) {
            recommendDietPlan();
        }
        else if (choice == 3) {
            generateWorkoutPlan();
        }
        else if (choice == 0) {
            return;
        }
        else {
            cout << string(getLeftMargin() + 6, ' ') << "Invalid choice.\n";
            pauseScreen();
        }
    } while (choice != 0);
}


// ============================================================================
// SECTION: REPORTING & STATISTICS (was src/ReportsModule.cpp)
// ============================================================================
namespace {

// Formats a number as "RM 12.50"
string rptFmtMoney(float v) {
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
    printBoxLine("Total Gross Sales        : " + rptFmtMoney(totalGross), "left", 4);
    printBoxLine("Total Discounts Applied  : " + rptFmtMoney(totalDiscount), "left", 4);
    printBoxLine("Net Gym Earnings         : " + rptFmtMoney(totalNet), "left", 4);
    printBoxLine("Total SST (6%) Collected : " + rptFmtMoney(totalSST), "left", 4);
    printBoxLine("Total Payments Received  : " + rptFmtMoney(totalCash), "left", 4);

    printBoxDivider();
    printBoxLine("[ 3. BUSINESS PERFORMANCE INDICATORS (KPIs) ]", "left", 4);
    printBoxDivider();
    printBoxLine("Total Receipts Issued    : " + to_string(totalReceipts), "left", 4);
    printBoxLine("Average Spend / Txn      : " + rptFmtMoney(averageSpend), "left", 4);
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


// ============================================================================
// SECTION: SERVICE, SCHEDULE & BOOKING MANAGEMENT (was src/ServiceBookingModule.cpp)
// ============================================================================

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


// ============================================================================
// SECTION: MAIN (was src/main.cpp)
// ============================================================================
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
