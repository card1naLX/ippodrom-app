//Артем Гордийчук 13гр.

#pragma once
#include <string>
#include "sqlite3.h"
#include "structures.hpp"

class DatabaseManager {
private:
    sqlite3* db;

public:
    DatabaseManager();
    ~DatabaseManager();

    bool connect(const std::string& db_name);
    void disconnect();
    

    void initializeTables();
    UserRole checkAuth(const std::string& username, const std::string& password);
    bool insertRaceWithValidation(const Race& race);
    void distributePrizeFund(int race_number, double total_fund);
    
    void printHighestWinningHorse();
    void printJockeyWithMostRaces();
    void printJockeyHistory(const std::string& jockey_name);
    void printOwnerHorses(const std::string& owner_name);
    void printRacesInPeriod(const std::string& start_date, const std::string& end_date);
    bool deleteJockey(int jockey_id);
};