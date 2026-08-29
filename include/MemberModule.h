#pragma once
#include <string>
#include "Types.h"

// ================= MODULE 5.1: MEMBER & COACH MANAGEMENT =================
void registerMember(Member members[], int& memberCount);
std::string loginUser(const Member members[], int memberCount, std::string& loggedID);
void forgotPassword(Member members[], int memberCount);
void editMemberProfile(Member members[], int memberCount, const std::string& currentMemberID);
void viewMemberProfile(Member members[], int memberCount, const std::string& currentMemberID);
void viewAllMembers(Member members[], int& memberCount);
void viewMasterDetails(const Master masters[], int masterCount, int startIndex);
void viewMasterList(const std::string& role, Master masters[], int& masterCount);
void menuCustomerManagement(const std::string& role, Member members[], int& memberCount, const std::string& currentMemberID, Master masters[], int& masterCount);
void renewMembership(Member members[], int memberCount);
