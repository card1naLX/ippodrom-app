//Габралев Иван 13гр.
#include <iostream>
#include <string>
#include "db_manager.hpp"
#include "structures.hpp"

void printMenu(UserRole role) {
    std::cout << "\n=========================================\n";
    std::cout << "         HIPPODROME SYSTEM MENU          \n";
    std::cout << "=========================================\n";
    
    if (role == UserRole::Admin) {
        std::cout << "[1] Add new race with validation\n";
        std::cout << "[2] Distribute prize fund for a race\n";
        std::cout << "[3] Delete jockey (Integrity check)\n";
        std::cout << "[4] View top winning horse\n";
        std::cout << "[5] View jockey with most races\n";
    }
    
    if (role == UserRole::Admin || role == UserRole::Jockey) {
        std::cout << "[6] View jockey race history\n";
    }
    
    if (role == UserRole::Admin || role == UserRole::Owner) {
        std::cout << "[7] View owner's horses and results\n";
    }
    
    std::cout << "[8] Search races within a period\n";
    std::cout << "[0] Exit application\n";
    std::cout << "=========================================\n";
    std::cout << "Enter your choice: ";
}

int main() {
    DatabaseManager db;
    
    // Using absolute path to make sure the database file is found/created correctly
    std::string db_path = "C:\\Users\\gabra\\tpmp\\ippodrom-app\\horses.db";
    
    if (!db.connect(db_path)) {
        std::cerr << "Critical Error: Could not connect to the database!" << std::endl;
        return 1;
    }
    
    // Create tables and default accounts if they don't exist
    db.initializeTables();
    
    std::string username, password;
    UserRole current_role = UserRole::None;
    
    std::cout << "=========================================\n";
    std::cout << "   Welcome to Hippodrome System (KIS)    \n";
    std::cout << "=========================================\n";
    
    // Authentication loop
    while (current_role == UserRole::None) {
        std::cout << "\nPlease log in to continue.\n";
        std::cout << "Username: ";
        std::cin >> username;
        std::cout << "Password: ";
        std::cin >> password;
        
        current_role = db.checkAuth(username, password);
        
        if (current_role == UserRole::None) {
            std::cout << "Invalid username or password! Please try again.\n";
        }
    }
    
    std::cout << "\nLogin successful! Welcome, " << username << ".\n";
    
    int choice = -1;
    while (choice != 0) {
        printMenu(current_role);
        std::cin >> choice;
        
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid input. Please enter a number.\n";
            continue;
        }
        
        // Authorization & functionality checks
        if (choice == 1 && current_role == UserRole::Admin) {
            Race r;
            std::cout << "\n--- ADD NEW RACE ---\n";
            std::cout << "Enter date (YYYY-MM-DD): ";
            std::cin >> r.date;
            std::cout << "Enter race number: ";
            std::cin >> r.race_number;
            std::cout << "Enter horse alias: ";
            std::cin >> r.horse_alias;
            std::cout << "Enter jockey name: ";
            std::cin >> r.jockey_name;
            std::cout << "Enter final place: ";
            std::cin >> r.place;
            
            if (db.insertRaceWithValidation(r)) {
                std::cout << "Race successfully recorded!\n";
            } else {
                std::cout << "Failed to add race due to validation rules.\n";
            }
        }
        else if (choice == 2 && current_role == UserRole::Admin) {
            int race_num;
            double fund;
            std::cout << "\n--- CALCULATE PRIZE FUND ---\n";
            std::cout << "Enter race number: ";
            std::cin >> race_num;
            std::cout << "Enter total prize pool amount: ";
            std::cin >> fund;
            db.distributePrizeFund(race_num, fund);
        }
        else if (choice == 3 && current_role == UserRole::Admin) {
            int jockey_id;
            std::cout << "\n--- DELETE JOCKEY RECORD ---\n";
            std::cout << "Enter Jockey ID to delete: ";
            std::cin >> jockey_id;
            db.deleteJockey(jockey_id);
        }
        else if (choice == 4 && current_role == UserRole::Admin) {
            db.printHighestWinningHorse();
        }
        else if (choice == 5 && current_role == UserRole::Admin) {
            db.printJockeyWithMostRaces();
        }
        else if (choice == 6 && (current_role == UserRole::Admin || current_role == UserRole::Jockey)) {
            std::string j_name;
            std::cout << "\nEnter jockey name to view history: ";
            std::cin >> j_name;
            db.printJockeyHistory(j_name);
        }
        else if (choice == 7 && (current_role == UserRole::Admin || current_role == UserRole::Owner)) {
            std::string o_name;
            std::cout << "\nEnter owner name to view stable: ";
            std::cin >> o_name;
            db.printOwnerHorses(o_name);
        }
        else if (choice == 8) {
            std::string start, end;
            std::cout << "\n--- SEARCH BY PERIOD ---\n";
            std::cout << "Enter start date (YYYY-MM-DD): ";
            std::cin >> start;
            std::cout << "Enter end date (YYYY-MM-DD): ";
            std::cin >> end;
            db.printRacesInPeriod(start, end);
        }
        else if (choice == 0) {
            std::cout << "\nThank you for using Hippodrome KIS. Goodbye!\n";
        }
        else {
            std::cout << "\nAccess Denied or Invalid Option for your role.\n";
        }
    }
    
    db.disconnect();
    return 0;
}