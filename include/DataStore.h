#pragma once
#include "Types.h"

// ================= FILE-BASED DATA PERSISTENCE =================
// Loads/saves each data type to a plain delimited text file under data/.
// load* returns the number of records read (0 if the file doesn't exist yet).
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
