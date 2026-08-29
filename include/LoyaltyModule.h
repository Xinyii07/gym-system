#pragma once
#include <string>
#include "Types.h"

// ================= MODULE 5.7: LOYALTY & GIFT MANAGEMENT =================
void menuLoyalty(const std::string& role, Member members[], int memberCount,
                  const std::string& currentMemberID, Gift giftList[], int& giftCount);
