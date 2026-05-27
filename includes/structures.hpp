//Артем Гордийчук 13гр.

#pragma once
#include <string>

enum class UserRole {
    Admin,
    Jockey,
    Owner,
    None
};

struct User {
    int id;
    std::string login;
    std::string role;
};

struct Horse {
    int id;
    std::string alias;
    int age;
    int experience;
    std::string owner_name;
    double purchase_price;
};

struct Jockey {
    int id;
    std::string name;
    int experience;
    int birth_year;
    std::string address;
};

struct Race {
    int id;
    std::string date;
    int race_number;
    std::string horse_alias;
    std::string jockey_name;
    int place;
};