#pragma once
#include <string>
#include "Types.h"

// ================= MODULE 5.2: BILLING & PAYMENT MANAGEMENT =================
void getCurrentDate(int& day, int& month, int& year);
float calculateTotalCharge(float baseCharge, float discountPercent, float& discountAmt, float& sstAmt);
int processPayment(const std::string& memberID, const std::string& bookingID, const std::string& serviceType, float baseCharge, float discountPercent, Payment paymentList[], int& paymentCount);
void voidPaymentsForBooking(Payment paymentList[], int paymentCount, const std::string& bookingID);
void generateInvoice(const Payment paymentList[], int paymentCount, int paymentID);
int viewPendingPayment(const Payment paymentList[], int paymentCount, const std::string& currentUserID, const std::string& userRole);
void viewPaymentHistory(const Payment paymentList[], int paymentCount, const std::string& memberID, const std::string& userRole);
void confirmPayment(Payment paymentList[], int& paymentCount, const std::string& adminName, Member members[], int memberCount);
void menuBillingAndPayment(const std::string& role, const std::string& userID, Payment paymentList[], int& paymentCount, const std::string& adminName, Member members[], int memberCount);
