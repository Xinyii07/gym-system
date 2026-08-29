1. after starting to add member , we sould allow user to quit to the previous menu.
2. When creating password for new member, we should not see the password.
3. When creating new member we need to validate email, should have @ and end with domain.
4. When create new member, we need to validate phone number to be number only and only 10 - 11 digits.
5. When log in, need to censor the password.
6. When we are at all member list, the new options should be 1 add memebr, 2 remove member, 3. ed0 back.
7. Add a user role label on top right after log in.
8. View member profile should be view my profile.

*Question to ask:
1. When editting profile, who is John Doe

---
New stuff to fix:
1. Add edit member in all member list
2. When removing a member, i want it to like seclect like claude code item, with a moving cursor.
3. Change all the master thing to coach instead.
4. Add coach or remove coach should be at master list, not inside details.
5. Add a way to let people change coach to view, using arrow key to switch between different coaches.
6. after starting to remove master , we sould allow user to quit to the previous menu.
7. When remove master, instead of asking for ID, let me choose like claude code selection.

New new edit for Coach List
1. When add coach, need to add validation for only number allow for experience, specification is only allow for words only, 
new  edit for member profile:
1. When editting the email, need to show the old email first, the old email cannot be the same with new email.

---
Report and statistis module  features:
1. sub function: generateSalesReport() (include the 2,3, 4 sub modules)
2. selectReportType() 
Display the sales report submenu, use a “switch” statement to handle the administrator's selection access for weekly or monthly report, and then direct the program execution flow to the corresponding report function.
3. weeklySalesReport() 
Filters transaction records from the Billing module for the current 7-day week. It calculates and formats the output into 3 sections:   
Service Sales Breakdown:
It reflects the number of units sold, standard unit cost, total sales and net revenue earned for each gym service category
Financial Summary: 
It gives a complete financial picture by calculating total gross sales, applied transactional discount, net gym earnings, 6% SST tax collected and total payments received.
Business Performance Indicators (KPIs): 
Total number of transaction receipts issued, average spend per transaction, and top-performing service.

4. monthlySalesReport() 
Prompt the administrator to select a specific month, then process the matching monthly transaction records. It calculates and formats the output into 3 sections:  
Service Sales Breakdown:
Quantities sold, unit prices, gross sales, and net income for each service like New Memberships, Renewals, Class Bookings.
Financial Summary: 
Include total gross sales, discounts applied on transactions, net gym earnings, 6% SST tax collected, and total payment collected.
Business Performance Indicators (KPIs): 
Total number of transaction receipts issued, average spend per transaction, and top-performing service.

5. calculateMemberStatistics()
It analyzes member data that records in the Customer Management module. It calculates total registered members, total accounts who are active or expired, number of new registration and member renewal.

6. analyzeBMIDistribution()
It analyzes members' BMI records from the Recommendation module and classifies members into four BMI categories which is underweight, normal weight, overweight, and obese. Additionally, it calculates the total count and percentage for each category, as well as the overall average BMI of the gym members.

Pseudocode for each:
1. menuReportsAndStatistics()
BEGIN menuReportsAndStatistics
    SET choice = 0
    
    WHILE choice != 4
        DISPLAY "=================================================="
        DISPLAY "      ADMIN REPORTING & STATISTICS MODULE         "
        DISPLAY "=================================================="
        DISPLAY "1. Generate Sales & Revenue Report"
        DISPLAY "2. View Membership Statistics"
        DISPLAY "3. Analyze Member BMI Distribution"
        DISPLAY "4. Return to Main Menu"
        DISPLAY "Enter your choice (1-4): "
        INPUT choice
        
        SWITCH choice
            CASE 1:
                CALL generateSalesReport()
            CASE 2:
                CALL calculateMemberStatistics()
            CASE 3:
                CALL analyzeBMIDistribution()
            CASE 4:
                DISPLAY "Returning to Main Menu..."
            DEFAULT:
                DISPLAY "Invalid selection! Please enter a number between 1 and 4."
        END SWITCH
    END WHILE
END menuReportsAndStatistics

2. generateSalesReport()

START generateSalesReport
    SET reportChoice = 0
    WHILE reportChoice is not equal to 3
        DISPLAY "=========================================="
        DISPLAY "          SALES REPORT GENERATOR          "
        DISPLAY "=========================================="
        DISPLAY "1. Weekly Sales Report"
        DISPLAY "2. Monthly Sales Report"
        DISPLAY "3. Back"
        DISPLAY "Select report type (1-3): "
        INPUT reportChoice

        IF reportChoice is invalid OR reportChoice < 1 OR reportChoice > 3 THEN
            DISPLAY "Error: Invalid choice! Please enter 1, 2, or 3."
        ELSE IF reportChoice is equal to 1 THEN
            CALL generateWeeklySalesReport
        ELSE IF reportChoice is equal to 2 THEN
            CALL generateMonthlySalesReport
        ELSE IF reportChoice is equal to 3 THEN
            DISPLAY "Returning to Reporting Menu..."
        END IF
    END WHILE
END generateSalesReport



3. weeklySalesReport() 

START weeklySalesReport
    IF billingList is empty THEN
        DISPLAY error message for empty billing database
    ELSE
        SET totalGross = 0.0
        SET totalDiscount = 0.0
        SET totalSST = 0.0
        SET totalNet = 0.0
        SET totalUnitsSold = 0
        SET totalReceipts = 0
        SET topService = ""
        SET maxUnits = 0

        DISPLAY weekly sales report header

        FOR EACH service IN gymServiceList
            SET serviceQty = 0
            SET serviceGross = 0.0
            SET serviceDiscount = 0.0

            FOR EACH transaction IN billingList
                IF transaction.serviceID is equal to service.serviceID AND transaction.date is within current week THEN
                    SET serviceQty = serviceQty + transaction.quantity
                    SET serviceGross = serviceGross + (transaction.unitPrice * transaction.quantity)
                    SET serviceDiscount = serviceDiscount + transaction.discount
                END IF
            END FOR

            SET serviceNet = serviceGross - serviceDiscount

            IF serviceQty > 0 THEN
                DISPLAY service details showing name, unit price, quantity sold, gross sales, discount, and net income
                
                SET totalGross = totalGross + serviceGross
                SET totalDiscount = totalDiscount + serviceDiscount
                SET totalUnitsSold = totalUnitsSold + serviceQty

                IF serviceQty > maxUnits THEN
                    SET maxUnits = serviceQty
                    SET topService = service.name
                END IF
            END IF
        END FOR

        FOR EACH transaction IN billingList
            IF transaction.date is within current week THEN
                SET totalSST = totalSST + transaction.sstAmount
                SET totalReceipts = totalReceipts + 1
            END IF
        END FOR

        IF totalReceipts is equal to 0 THEN
            DISPLAY notice message for no sales recorded in current week
        ELSE
            SET totalNet = totalGross - totalDiscount
            SET totalCash = totalNet + totalSST
            SET averageSpend = totalCash / totalReceipts

            DISPLAY service breakdown summary showing total units sold
            DISPLAY financial summary showing total gross sales, total discounts, net revenue, total SST, and total cash collected
            DISPLAY business performance indicators showing total receipts issued, average spend per transaction, and top performing service
        END IF
    END IF
END weeklySalesReport

4. START monthlySalesReport
    IF billingList is empty THEN
        DISPLAY "Error: No transaction records found."
    ELSE
        SET targetMonth = 0
        DISPLAY "Enter target month (1-12): "
        INPUT targetMonth

        WHILE targetMonth is invalid OR targetMonth < 1 OR targetMonth > 12
            DISPLAY "Error: Invalid month! Please enter a valid month (1-12): "
            INPUT targetMonth
        END WHILE

        SET totalGross = 0.0
        SET totalDiscount = 0.0
        SET totalSST = 0.0
        SET totalNet = 0.0
        SET totalUnitsSold = 0
        SET totalReceipts = 0
        SET topService = ""
        SET maxUnits = 0

        DISPLAY "=== MONTHLY SALES REPORT ==="
        DISPLAY "[ 1. SERVICE SALES BREAKDOWN ]"

        FOR EACH service IN gymServiceList
            SET serviceQty = 0
            SET serviceGross = 0.0
            SET serviceDiscount = 0.0

            FOR EACH transaction IN billingList
                IF transaction.serviceID is equal to service.serviceID AND transaction.month is equal to targetMonth THEN
                    SET serviceQty = serviceQty + transaction.quantity
                    SET serviceGross = serviceGross + (transaction.unitPrice * transaction.quantity)
                    SET serviceDiscount = serviceDiscount + transaction.discount
                END IF
            END FOR

            SET serviceNet = serviceGross - serviceDiscount

            IF serviceQty > 0 THEN
                DISPLAY service.name, service.unitPrice, serviceQty, serviceGross, serviceDiscount, serviceNet
                
                SET totalGross = totalGross + serviceGross
                SET totalDiscount = totalDiscount + serviceDiscount
                SET totalUnitsSold = totalUnitsSold + serviceQty

                IF serviceQty > maxUnits THEN
                    SET maxUnits = serviceQty
                    SET topService = service.name
                END IF
            END IF
        END FOR

        FOR EACH transaction IN billingList
            IF transaction.month is equal to targetMonth THEN
                SET totalSST = totalSST + transaction.sstAmount
                SET totalReceipts = totalReceipts + 1
            END IF
        END FOR

        IF totalReceipts is equal to 0 THEN
            DISPLAY "Notice: No sales transactions recorded for the selected month."
        ELSE
            SET totalNet = totalGross - totalDiscount
            SET totalCash = totalNet + totalSST
            SET averageSpend = totalCash / totalReceipts

            DISPLAY "Total Units Sold: ", totalUnitsSold

            DISPLAY "[ 2. FINANCIAL SUMMARY & TAX ACCOUNTING ]"
            DISPLAY totalGross, totalDiscount, totalNet,  totalSST and totalCash

            DISPLAY "[ 3. BUSINESS PERFORMANCE INDICATORS (KPIs) ]"
            DISPLAY totalReceipts,  averageSpend, topService
        END IF
    END IF
END monthlySalesReport

5. calculateMemberStatistics()
START calculateMemberStatistics
    IF memberList is empty THEN
        DISPLAY error message for empty member database
    ELSE
        SET totalMembers = 0
        SET activeCount = 0
        SET expiredCount = 0
        SET newMonthlyCount = 0

        FOR EACH member IN memberList
            SET totalMembers = totalMembers + 1
            
            IF member.status is equal to "ACTIVE" THEN
                SET activeCount = activeCount + 1
            ELSE
                SET expiredCount = expiredCount + 1
            END IF
            
            IF member.joinMonth is equal to current month THEN
                SET newMonthlyCount = newMonthlyCount + 1
            END IF
        END FOR

        IF totalMembers is equal to 0 THEN
            DISPLAY notice message for no member records available
        ELSE
            SET retentionRate = (activeCount / totalMembers) * 100

            DISPLAY membership dashboard showing total members, active members, expired members, new registrations, and active retention rate
        END IF
    END IF
END calculateMemberStatistics

6. analyzeBMIDistribution()

START analyzeBMIDistribution
    IF memberList is empty THEN
        DISPLAY error message for empty member database
    ELSE
        SET totalEvaluated = 0
        SET underweightCount = 0
        SET normalCount = 0
        SET overweightCount = 0
        SET obeseCount = 0
        SET sumBMI = 0.0

        FOR EACH member IN memberList
            IF member.height > 0 AND member.weight > 0 THEN
                SET bmi = member.weight / (member.height * member.height)
                SET sumBMI = sumBMI + bmi
                SET totalEvaluated = totalEvaluated + 1

                IF bmi < 18.5 THEN
                    SET underweightCount = underweightCount + 1
                ELSE IF bmi >= 18.5 AND bmi < 25.0 THEN
                    SET normalCount = normalCount + 1
                ELSE IF bmi >= 25.0 AND bmi < 30.0 THEN
                    SET overweightCount = overweightCount + 1
                ELSE
                    SET obeseCount = obeseCount + 1
                END IF
            END IF
        END FOR

        IF totalEvaluated is equal to 0 THEN
            DISPLAY error message for no valid member health records found
        ELSE
            SET averageBMI = sumBMI / totalEvaluated

            DISPLAY BMI distribution summary showing total evaluated members, count per category, and average gym BMI

            IF (overweightCount + obeseCount) > normalCount THEN
                DISPLAY recommendation message for weight-loss and cardio programs
            ELSE
                DISPLAY recommendation message for general conditioning programs
            END IF
        END IF
    END IF
END analyzeBMIDistribution

---
feature to edit:
1. can you let the code become more easier, easy to understand for university level espeacially for the 
2. i think just cancel the analyze member BMI distribution, a little too complex for my current level.
3. change the return to menu into 0 in admin reporting and statistisc module.
4. change the 3 back in sales report generator into 0.


--- 
1. The top performing service in business performance indicator should be the services that sell the most mean the higher booking number of service.
2. Service sales breakdown in weekly sales report mean the class sell, like 'yoga' how much unit price (per class charges), QTy (how many member book appointment for that class), and discout (mean when checkout in comfirm payment how many discount value give to it, the total amount of dicount amount), net mean fross - discount.
3. Change all the word coach back to master.
---
Module for Manage gift:
1. Main Function: menuLoyalty()
Serves as the main controller for the module, providing options to view points and tiers, browse the reward catalog, process redemptions, and manage gifts.

2. Sub-Function: 
awardPaymentPoint():
Allows viewing a member's total accumulated loyalty points and their current tier status (Silver, Gold, or Platinum).
rewardCatalog()
Displays the list of gifts available for redemption and the required points, as well as the tier-based discount rates applicable to members.
redeemGift()
An administrator-only function that allows staff to verify, approve, and process gift redemptions by deducting points from member accounts.
manageGift()
An administrator-only function for updating the gift list, including adding new reward items or removing items that are no longer available.

START menuLoyalty
    SET choice = 0
    WHILE choice is not equal to 5
        DISPLAY "=== MEMBER LOYALTY & REWARDS MODULE ==="
        DISPLAY "1. View Member Points & Tier Status"
        DISPLAY "2. Browse Reward Catalog & Discounts"
        DISPLAY "3. Process Gift Redemption (Admin)"
        DISPLAY "4. Manage Gift Catalog (Admin)"
        DISPLAY "5. Return to Main Menu"
        DISPLAY "Enter your choice (1-5): "
        DISPLAY prompt to enter choice (1-5)
        INPUT choice

        IF choice is invalid OR choice < 1 OR choice > 5 THEN
            DISPLAY error message for invalid menu choice
        ELSE IF choice is equal to 0 THEN
            DISPLAY message returning to main menu        
        ELSE IF choice is equal to 1 THEN
            CALL awardPaymentPoint
        ELSE IF choice is equal to 2 THEN
            CALL rewardCatalog
        ELSE IF choice is equal to 3 THEN
            CALL redeemGift
        ELSE IF choice is equal to 4 THEN
            CALL manageGift

        END IF
    END WHILE
END menuLoyalty


5.7.2 awardPaymentPoint()

START awardPaymentPoint
    Get member ID and member point from system
    IF memberList is empty THEN
        DISPLAY error message for empty member database
    ELSE
        SET memberID = ""
        DISPLAY prompt to enter member ID
        INPUT memberID
        
        FIND member IN memberList WHERE member.id is equal to memberID

        IF member is not found THEN
            DISPLAY error message for invalid member ID
        ELSE
            SET userPoints = member.points

            IF userPoints >= 500 THEN
                SET tierName = "Platinum"
                SET tierDiscount = 10
            ELSE IF userPoints >= 250 THEN
                SET tierName = "Gold"
                SET tierDiscount = 8
            ELSE IF userPoints >= 150 THEN
                SET tierName = "Silver"
                SET tierDiscount = 5
            ELSE
                SET tierName = "Basic"
                SET tierDiscount = 0
            END IF

            DISPLAY member point summary showing total points, current tier status, and active billing discount percentage
        END IF
    END IF
END awardPaymentPoint



5.7.3 rewardCatalog()

START rewardCatalog
    IF giftList is empty THEN
        DISPLAY error message for empty gift catalog
    ELSE
        DISPLAY tier discount breakdown showing Silver (150 pts = 5%), Gold (250 pts = 8%), and Platinum (500 pts = 10%)

        FOR EACH gift IN giftList
            IF gift.status is equal to "AVAILABLE" THEN
                DISPLAY gift details showing gift ID, gift name, and required point cost
            END IF
        END FOR
    END IF
END rewardCatalog


5.7.4 redeemGift()

START redeemGift
    IF memberList is empty OR giftList is empty THEN
        DISPLAY error message for missing member or gift records
    ELSE
        SET memberID = ""
        DISPLAY prompt for admin to enter member ID
        INPUT memberID

        FIND member IN memberList WHERE member.id is equal to memberID

        IF member is not found THEN
            DISPLAY error message for invalid member ID
        ELSE
            SET giftID = ""
            DISPLAY prompt for admin to enter gift ID to redeem
            INPUT giftID

            FIND gift IN giftList WHERE gift.id is equal to giftID

            IF gift is not found THEN
                DISPLAY error message for invalid gift ID
            ELSE IF gift.status is not equal to "AVAILABLE" THEN
                DISPLAY error message for gift out of stock or unavailable
            ELSE IF member.points < gift.pointCost THEN
                DISPLAY error message for insufficient loyalty points
            ELSE
                SET member.points = member.points - gift.pointCost

                DISPLAY confirmation message for successful over-the-counter gift redemption
                DISPLAY updated member remaining point balance
            END IF
        END IF
    END IF
END redeemGift


5.7.5 manageGift()

START manageGift
    SET adminChoice = 0
    WHILE adminChoice is not equal to 3
        DISPLAY gift management sub-menu options for adding or canceling gifts
        DISPLAY prompt to enter choice (1-3)
        INPUT adminChoice

        IF adminChoice is invalid OR adminChoice < 1 OR adminChoice > 3 THEN
            DISPLAY error message for invalid selection
        ELSE IF adminChoice is equal to 1 THEN
            SET newGiftName = ""
            SET newPointCost = 0

            DISPLAY prompt to enter new gift name
            INPUT newGiftName
            DISPLAY prompt to enter required point cost
            INPUT newPointCost

            WHILE newGiftName is empty OR newPointCost <= 0
                DISPLAY error message for invalid gift details
                INPUT newGiftName
                INPUT newPointCost
            END WHILE

            CREATE new gift with entered name, point cost, and status "AVAILABLE"
            ADD new gift to giftList

            DISPLAY confirmation message for successfully adding new gift to catalog

        ELSE IF adminChoice is equal to 2 THEN
            IF giftList is empty THEN
                DISPLAY error message for empty gift catalog
            ELSE
                SET giftID = ""
                DISPLAY prompt to enter gift ID to cancel or remove
                INPUT giftID

                FIND gift IN giftList WHERE gift.id is equal to giftID

                IF gift is not found THEN
                    DISPLAY error message for invalid gift ID
                ELSE
                    SET gift.status = "CANCELLED"
                    DISPLAY confirmation message for successfully canceling gift item
                END IF
            END IF
        ELSE IF adminChoice is equal to 3 THEN
            DISPLAY message returning to loyalty menu
        END IF
    END WHILE
END manageGift

--- 
1. When view member point and tier status, give me like a claude code select bar to choose which member to view.
2. When inside the view member point , always ley user can quit to previous menu.
3. Put the manage gift catalog option in the Browse Reward catalog, mean [2] in member Loyalty & reward module have to change to Browse & Manage Gidt Catalog & Discount.
4. Add one more function in Manage Gift catalog, which allow admin to edit the discount amount
5. There will be endless loop whe i key in a character in the point cost when i am in add gift catalog.
6. When in the add gift catalog, give the option 0 to back to previous menu.
7. For the process gift redemption, show the full list of gift with gift ID , and full list of member list with their points so that admin can key in the member ID and Gift ID by referring the table.
8. When admin in the manage gift catalog, always able the admin to back to previous menu.
---
newedit
1. In the cancel/ remove gift, give 2 option one is for totally remove from the gift catalog, another is to set it as tempolary unavailable.
2. Add the function to let member see the pending payment and history payment for own only in customer main menu
3. change the payment history and invoice in the user billing & payment menu/
4. the loyalty point should be link to the completed payment , once the payment have been mark as done by the admin and the amount spend will be occur in tne payment history, so link the payment amount to the point. RM1 = 1 point.
5. Add one more statement in the reward catalog & discount where every spend of RM1  = 1 point.

[DONE - see src/LoyaltyModule.cpp, src/BillingModule.cpp]

1. For the admin confirm pending payment, display a full list of member that have pending payment.
2. In confirm pending payment, any 


