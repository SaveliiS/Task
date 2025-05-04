#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <string>

using namespace std;

struct Time {
    int hours;
    int minutes;

    Time();
    Time(int h, int m);

    bool operator<(const Time& other) const;
    bool operator>(const Time& other) const;
    bool operator<=(const Time& other) const;
    bool operator>=(const Time& other) const;
    int toMinutes() const;
    static Time fromMinutes(int total);
};

Time parseTime(const string& s);

string timeToString(const Time& t);

struct Event {
    Time time;
    int id;
    vector<string> args;

    Event(const string& line);
    string toString() const;
};

struct Table {

    bool occupied;
    Time start;
    int revenue;
    int totalMinutes;

    Table();
};

class ComputerClub {
    int tableCount;
    Time openTime;
    Time closeTime;
    int hourCost;
    vector<Table> tables;
    queue<string> waitingQueue;
    unordered_map<string, int> clients;
    vector<Event> outputEvents;

    void addEvent(const Time& time, int id, const vector<string>& args);
    void handleError(const Event& e, const string& msg);
    void clientSit(const Time& time, const string& client, int tableId, bool toGenerate);

public:
    ComputerClub(int tables, Time open, Time close, int cost);
    void processEvent(const Event& e);
    void endDay();
    void printResults() const;
};

void validateLine(const string& line);
void validateEvent(const Event& e, const string& line);
int validateNum(const string& line);