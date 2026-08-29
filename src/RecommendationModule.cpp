#include "RecommendationModule.h"
#include "UIUtils.h"
#include <iostream>
#include <iomanip>

using namespace std;

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
