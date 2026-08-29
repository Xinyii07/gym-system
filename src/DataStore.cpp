#include "DataStore.h"
#include "Config.h"
#include <fstream>
#include <vector>
#include <filesystem>

using namespace std;

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
