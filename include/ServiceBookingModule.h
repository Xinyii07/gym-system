#pragma once
#include <string>
#include "Types.h"

// ================= SERVICE, SCHEDULE & BOOKING MANAGEMENT =================
void displayWeeklyClassTable(const GymService services[], int serviceCount);
void displaySingleSchedule(const GymService& srv, bool isAdminView = false);

void createBooking(const std::string& memberID, GymService services[], int serviceCount, Booking bookingList[], int& bookingCount, Payment paymentList[], int& paymentCount);
void viewGymService(GymService services[], int serviceCount, const std::string& memberID, Booking bookingList[], int& bookingCount, Payment paymentList[], int& paymentCount);
void viewMemberBooking(const std::string& memberID, const Booking bookingList[], int bookingCount);
void editOrCancelBooking(const std::string& memberID, GymService services[], int serviceCount, Booking bookingList[], int bookingCount, Payment paymentList[], int paymentCount);

void menuServiceBooking(const std::string& memberID, GymService services[], int serviceCount, LockerDevice lockers[], int totalLockers, Booking bookingList[], int& bookingCount, Payment paymentList[], int& paymentCount);

void addGymService(GymService services[], int& serviceCount);
void updateServiceDetails(GymService services[], int serviceCount);
void deleteGymService(GymService services[], int serviceCount, Booking bookingList[], int bookingCount, Payment paymentList[], int paymentCount);
void viewMasterSchedule(const GymService services[], int serviceCount);
void viewAllMemberBookings(const Booking bookingList[], int bookingCount);
void markAppointmentAsDone(Booking bookingList[], int bookingCount);

void menuScheduleManagement(GymService services[], int& serviceCount, Member members[], int memberCount, Booking bookingList[], int& bookingCount, LockerDevice lockers[], int totalLockers, Payment paymentList[], int paymentCount);
