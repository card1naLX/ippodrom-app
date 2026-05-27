//Артем Гордийчук 13гр.


#include "db_manager.hpp"
#include <iostream>
#include <iomanip>

// Constructor
DatabaseManager::DatabaseManager() : db(nullptr) {}

// Destructor
DatabaseManager::~DatabaseManager() {
    disconnect();
}

// Connect to the database
bool DatabaseManager::connect(const std::string& db_name) {
    if (sqlite3_open(db_name.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Failed to open DB: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    // Enable Foreign Key support
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
    return true;
}

// Disconnect from the database
void DatabaseManager::disconnect() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

// Initialize tables and insert default users
void DatabaseManager::initializeTables() {
    const char* create_jockeys = 
        "CREATE TABLE IF NOT EXISTS JOCKEYS ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "age INTEGER);";

    const char* create_horses = 
        "CREATE TABLE IF NOT EXISTS HORSES ("
        "alias TEXT PRIMARY KEY,"
        "age INTEGER,"
        "owner_name TEXT NOT NULL);";

    const char* create_races = 
        "CREATE TABLE IF NOT EXISTS RACES ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "date TEXT NOT NULL,"
        "race_number INTEGER NOT NULL,"
        "horse_alias TEXT,"
        "jockey_name TEXT NOT NULL,"
        "place INTEGER NOT NULL,"
        "FOREIGN KEY(horse_alias) REFERENCES HORSES(alias) ON DELETE RESTRICT);";

    const char* create_users = 
        "CREATE TABLE IF NOT EXISTS USERS ("
        "username TEXT PRIMARY KEY,"
        "password TEXT NOT NULL,"
        "role TEXT NOT NULL);";

    sqlite3_exec(db, create_jockeys, nullptr, nullptr, nullptr);
    sqlite3_exec(db, create_horses, nullptr, nullptr, nullptr);
    sqlite3_exec(db, create_races, nullptr, nullptr, nullptr);
    sqlite3_exec(db, create_users, nullptr, nullptr, nullptr);

    // Insert default users if they don't exist
    sqlite3_exec(db, "INSERT OR IGNORE INTO USERS VALUES('admin', 'admin123', 'admin');", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "INSERT OR IGNORE INTO USERS VALUES('ivanov', 'jockeypass', 'jockey');", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "INSERT OR IGNORE INTO USERS VALUES('petrov_owner', 'owner777', 'owner');", nullptr, nullptr, nullptr);
    
    // Insert some initial test data
    sqlite3_exec(db, "INSERT OR IGNORE INTO HORSES VALUES('Lightning', 4, 'petrov_owner');", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "INSERT OR IGNORE INTO HORSES VALUES('Granite', 5, 'petrov_owner');", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "INSERT OR IGNORE INTO JOCKEYS VALUES(1, 'ivanov', 25);", nullptr, nullptr, nullptr);
}

// Check user authorization
UserRole DatabaseManager::checkAuth(const std::string& username, const std::string& password) {
    std::string query = "SELECT role FROM USERS WHERE username = ? AND password = ?;";
    sqlite3_stmt* stmt;
    UserRole role = UserRole::None;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string role_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            if (role_str == "admin") role = UserRole::Admin;
            else if (role_str == "jockey") role = UserRole::Jockey;
            else if (role_str == "owner") role = UserRole::Owner;
        }
    }
    sqlite3_finalize(stmt);
    return role;
}

// Insert race with validation
bool DatabaseManager::insertRaceWithValidation(const Race& race) {
    // 1. Check if the horse exists
    std::string check_query = "SELECT COUNT(*) FROM HORSES WHERE alias = ?;";
    sqlite3_stmt* check_stmt;
    bool horse_exists = false;

    if (sqlite3_prepare_v2(db, check_query.c_str(), -1, &check_stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(check_stmt, 1, race.horse_alias.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(check_stmt) == SQLITE_ROW) {
            horse_exists = (sqlite3_column_int(check_stmt, 0) > 0);
        }
    }
    sqlite3_finalize(check_stmt);

    if (!horse_exists) {
        std::cerr << "Validation Error: Horse with alias '" << race.horse_alias << "' is not registered!" << std::endl;
        return false;
    }

    // 2. Insert the race record
    std::string insert_query = "INSERT INTO RACES (date, race_number, horse_alias, jockey_name, place) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* ins_stmt;
    bool success = false;

    if (sqlite3_prepare_v2(db, insert_query.c_str(), -1, &ins_stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(ins_stmt, 1, race.date.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(ins_stmt, 2, race.race_number);
        sqlite3_bind_text(ins_stmt, 3, race.horse_alias.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(ins_stmt, 4, race.jockey_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(ins_stmt, 5, race.place);

        if (sqlite3_step(ins_stmt) == SQLITE_DONE) {
            success = true;
        }
    }
    sqlite3_finalize(ins_stmt);
    return success;
}

// Calculate and distribute prize fund
void DatabaseManager::distributePrizeFund(int race_number, double total_fund) {
    std::string query = "SELECT horse_alias, place FROM RACES WHERE race_number = ? ORDER BY place ASC;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, race_number);
        
        std::cout << "\n--- PRIZE FUND DISTRIBUTION FOR RACE #" << race_number << " ---" << std::endl;
        int count = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string alias = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            int place = sqlite3_column_int(stmt, 1);
            double prize = 0;

            if (place == 1) prize = total_fund * 0.50;      // 50% for 1st place
            else if (place == 2) prize = total_fund * 0.30; // 30% for 2nd place
            else if (place == 3) prize = total_fund * 0.20; // 20% for 3rd place

            if (prize > 0) {
                std::cout << "Place: " << place << " | Horse: " << std::setw(10) << alias << " | Payout: " << prize << " USD" << std::endl;
                count++;
            }
        }
        if (count == 0) std::cout << "The race is empty or places are not assigned yet." << std::endl;
    }
    sqlite3_finalize(stmt);
}

// Print highest winning horse
void DatabaseManager::printHighestWinningHorse() {
    std::string query = "SELECT horse_alias, COUNT(*) as wins FROM RACES WHERE place = 1 GROUP BY horse_alias ORDER BY wins DESC LIMIT 1;";
    sqlite3_stmt* stmt;
    std::cout << "\n--- TOP WINNING HORSE ---" << std::endl;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string alias = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            int wins = sqlite3_column_int(stmt, 1);
            std::cout << "Alias: " << alias << " | Wins: " << wins << std::endl;
        } else { std::cout << "No wins recorded in the system." << std::endl; }
    }
    sqlite3_finalize(stmt);
}

// Print jockey with most races
void DatabaseManager::printJockeyWithMostRaces() {
    std::string query = "SELECT jockey_name, COUNT(*) as rc FROM RACES GROUP BY jockey_name ORDER BY rc DESC LIMIT 1;";
    sqlite3_stmt* stmt;
    std::cout << "\n--- JOCKEY WITH MOST RACES ---" << std::endl;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            int count = sqlite3_column_int(stmt, 1);
            std::cout << "Jockey: " << name << " | Total Races: " << count << std::endl;
        } else { std::cout << "No races recorded." << std::endl; }
    }
    sqlite3_finalize(stmt);
}

// Print jockey history
void DatabaseManager::printJockeyHistory(const std::string& jockey_name) {
    std::string query = "SELECT date, race_number, horse_alias, place FROM RACES WHERE jockey_name = ? ORDER BY date DESC;";
    sqlite3_stmt* stmt;
    std::cout << "\n--- RACE HISTORY FOR JOCKEY: " << jockey_name << " ---" << std::endl;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, jockey_name.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            int num = sqlite3_column_int(stmt, 1);
            std::string horse = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            int place = sqlite3_column_int(stmt, 3);
            std::cout << date << " | Race #" << num << " | Horse: " << horse << " | Place: " << place << std::endl;
        }
    }
    sqlite3_finalize(stmt);
}

// Print owner's horses
void DatabaseManager::printOwnerHorses(const std::string& owner_name) {
    std::string query = "SELECT h.alias, r.date, r.place FROM HORSES h LEFT JOIN RACES r ON h.alias = r.horse_alias WHERE h.owner_name = ?;";
    sqlite3_stmt* stmt;
    std::cout << "\n--- HORSES OWNED BY: " << owner_name << " ---" << std::endl;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, owner_name.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string alias = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            const char* d = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            int pl = sqlite3_column_int(stmt, 2);
            std::cout << "Alias: " << std::setw(10) << alias << " | Date: " << (d ? d : "None") << " | Place: " << pl << std::endl;
        }
    }
    sqlite3_finalize(stmt);
}

// Print races within a specific period
void DatabaseManager::printRacesInPeriod(const std::string& start_date, const std::string& end_date) {
    std::string query = "SELECT date, race_number, horse_alias, place FROM RACES WHERE date BETWEEN ? AND ?;";
    sqlite3_stmt* stmt;
    std::cout << "\n--- RACES IN PERIOD ---" << std::endl;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, start_date.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, end_date.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            int num = sqlite3_column_int(stmt, 1);
            std::string horse = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            int place = sqlite3_column_int(stmt, 3);
            std::cout << date << " | Race #" << num << " | Horse: " << horse << " | Place: " << place << std::endl;
        }
    }
    sqlite3_finalize(stmt);
}

// Delete jockey with integrity check
bool DatabaseManager::deleteJockey(int jockey_id) {
    std::string query = "DELETE FROM JOCKEYS WHERE id = ?;";
    sqlite3_stmt* stmt;
    bool success = false;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, jockey_id);
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) {
            if (sqlite3_changes(db) > 0) {
                std::cout << "Jockey successfully deleted." << std::endl;
                success = true;
            } else { std::cout << "Jockey not found." << std::endl; }
        } else if (rc == SQLITE_CONSTRAINT) {
            std::cerr << "RESTRICT Error: Cannot delete jockey due to foreign key constraint!" << std::endl;
        }
    }
    sqlite3_finalize(stmt);
    return success;
}