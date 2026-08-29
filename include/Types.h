#pragma once
#include <string>
#include "Config.h"

// ================= DATA STRUCTURE DEFINITIONS =================
struct LockerDevice {
    int lockerNo;
    std::string lockerType;
    bool isOccupied;
    std::string occupiedBy;
};

struct GymService {
    std::string serviceID;
    std::string name;
    std::string category; // "Group Activity" or "Personal Trainer"
    float price;          // Charge per booking (RM); billed as a pending payment on booking
    int capacity[DAYS][SLOTS];
    int booked[DAYS][SLOTS];
    bool isActive;
};

struct Member {
    std::string memberID;
    std::string fullName;
    std::string password;
    std::string expiryDate;
    std::string email;
    std::string phone;
    int points; // Loyalty points balance, spent via the Loyalty & Rewards module
};

struct Payment {
    int paymentID;
    std::string memberID;
    std::string bookingID;     // Links back to the Booking that generated this charge; "" for manual/admin charges
    std::string serviceType;
    float subtotal;
    float discountAmount;
    float sstAmount;
    float totalAmount;
    std::string status;        // "Pending", "Confirmed", or "Void" (booking canceled before payment)
    std::string paymentMethod; // "Cash" or "Card"
    float cashReceived;
    float changeGiven;
    int day, month, year;
    std::string confirmedBy;
};

struct Booking {
    std::string bookingID;
    std::string memberID;
    std::string category;
    std::string targetName;
    int dayIndex;
    int slotIndex;
    bool isActive;
    std::string status;     // "CONFIRMED", "DONE", "CANCELED"
};

// Data structure for a Master / Trainer profile
struct Master {
    std::string masterID;
    std::string name;
    std::string specialization;
    int experience;
    double rating;
};

// Data structure for a redeemable loyalty catalog item
struct Gift {
    std::string giftID;
    std::string name;
    int pointCost;
    std::string status; // "AVAILABLE" or "CANCELLED"
};
