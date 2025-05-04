#include "func.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <stdexcept>
#include <cctype>
using namespace std;

Time::Time() : hours(0), minutes(0) {}
Time::Time(int h, int m) : hours(h), minutes(m) {}

bool Time::operator<(const Time& other) const {
	return hours < other.hours || (hours == other.hours && minutes < other.minutes);
}

bool Time::operator>(const Time& other) const {
    return hours > other.hours || (hours == other.hours && minutes > other.minutes);
}

bool Time::operator<=(const Time& other) const {
	return *this < other || (hours == other.hours && minutes == other.minutes);
}

bool Time::operator>=(const Time& other) const {
    return *this > other || (hours == other.hours && minutes == other.minutes);
}

int Time::toMinutes() const {
	return hours * 60 + minutes;
}

Time Time::fromMinutes(int total) {
	return {total / 60, total % 60};
}

Time parseTime(const string& s) {
	if (s.size() != 5 || s[2] != ':') return {-1, -1};
    for (int i {0}; i < 5; ) {
        if (i == 2) { i++; continue; }
        if (!isdigit(s[i])) return {-1, -1};
        i++;
    }
    return {stoi(s.substr(0, 2)), stoi(s.substr(3, 2))};
}

string timeToString(const Time& t) {
	stringstream ss;
    ss << setw(2) << setfill('0') << t.hours << ":" 
       << setw(2) << setfill('0') << t.minutes;
    return ss.str();
}

Event::Event(const string& line) {
	stringstream ss(line);
    string t;
    ss >> t;
    time = parseTime(t);
    ss >> id;
    string arg;
    while (ss >> arg) args.push_back(arg);
}

string Event::toString() const {
	stringstream ss;
    ss << timeToString(time) << " " << id;
    for (const auto& a : args) ss << " " << a;
    return ss.str();
}

Table::Table() : occupied(false), revenue(0), totalMinutes(0) {}

ComputerClub::ComputerClub(int tables, Time open, Time close, int cost):
	tableCount(tables), openTime(open), closeTime(close), hourCost(cost)
	{ this->tables.resize(tables); }

void ComputerClub::addEvent(const Time& time, int id, const vector<string>& args) {
	outputEvents.push_back(timeToString(time) + " " + to_string(id) + " " + args[0] + (args.size() > 1 ? " " + args[1] : ""));
}

void ComputerClub::handleError(const Event& e, const string& msg) {
	outputEvents.push_back(timeToString(e.time) + " 13 " + msg);
}

void ComputerClub::clientSit(const Time& time, const string& client, int tableId, bool toGenerate) {
    tables[tableId].occupied = true;
    tables[tableId].start = time;
    clients[client] = tableId;
    if (toGenerate) addEvent(time, 12, {client, to_string(tableId + 1)});
}

void ComputerClub::processEvent(const Event& e) {
	outputEvents.push_back(e);
        
        switch (e.id) {
        case 1: {
            if (e.time < openTime || closeTime <= e.time) {
                handleError(e, "NotOpenYet");
                return;
            }
            if (clients.count(e.args[0])) {
                handleError(e, "YouShallNotPass");
            } else {
                clients[e.args[0]] = -1;
            }
            break;
        }
        case 2: {
            string client = e.args[0];
            int tableId = stoi(e.args[1]) - 1;
                
            if (!clients.count(client)) {
                handleError(e, "ClientUnknown");
                return;
            }
            if (tableId < 0 || tableId >= tableCount) return;
            if (tables[tableId].occupied) {
                handleError(e, "PlaceIsBusy");
                return;
            }
                
            if (clients[client] != -1) {
                tables[clients[client]].occupied = false;
            }
            clientSit(e.time, client, tableId, false);
            break;
        }
        case 3: {
            if (any_of(tables.begin(), tables.end(), [](const Table& t) { return !t.occupied; })) {
                handleError(e, "ICanWaitNoLonger!");
                return;
            }
            if (waitingQueue.size() >= tableCount) {
                addEvent(e.time, 11, {e.args[0]});
                clients.erase(e.args[0]);
            } else {
                waitingQueue.push(e.args[0]);
            }
            break;
        }
        case 4: {
            string client = e.args[0];
            if (!clients.count(client)) {
                handleError(e, "ClientUnknown");
                return;
            }
                
            int tableId = clients[client];
            if (tableId != -1) {
                Time start = tables[tableId].start;
                int minutes = e.time.toMinutes() - start.toMinutes();
                tables[tableId].totalMinutes += minutes;
                tables[tableId].revenue += ((minutes + 59) / 60) * hourCost;
                tables[tableId].occupied = false;

                if (!waitingQueue.empty()) {
                    string nextClient = waitingQueue.front();
                    waitingQueue.pop();
                    clientSit(e.time, nextClient, tableId, true);
                }
            }
            clients.erase(client);
            break;
        }
    }
}

void ComputerClub::endDay() {
	vector<string> remaining;
    for (const auto& [client, _] : clients) remaining.push_back(client);
    sort(remaining.begin(), remaining.end());
        
    for (const auto& client : remaining) {
        addEvent(closeTime, 11, {client});
        if (clients[client] != -1) {
            int tableId = clients[client];
            Time start = tables[tableId].start;
            int minutes = closeTime.toMinutes() - start.toMinutes();
            tables[tableId].totalMinutes += minutes;
            tables[tableId].revenue += ((minutes + 59) / 60) * hourCost;
        }
    }
}

void ComputerClub::printResults() const {
	cout << timeToString(openTime) << endl;

    for (const auto& e : outputEvents) {
        cout << e.toString() << endl;
    }

    cout << timeToString(closeTime) << endl;
        
    for (int i = 0; i < tableCount; i++) {
        Time total = Time::fromMinutes(tables[i].totalMinutes);
        cout << i+1 << " " << tables[i].revenue << " " 
             << timeToString(total) << endl;
    }
}

void validateLine(const string& line) {
	if (line.empty()) throw runtime_error("");
}

void validateEvent(const Event& e, const string& line) {
	if (e.time.hours == -1) throw runtime_error(line);
    if ((e.id < 1 || e.id > 4) && (e.id < 11 || e.id > 13)) throw runtime_error(line);
    if ((e.id != 2 && e.id != 12) && (e.args).size() != 1) throw runtime_error(line);
    else if (e.id != 2 && e.id != 12) {
        for (char c : (e.args)[0])
            if (!(isalnum(c) || c == '_' || c == '-')) throw runtime_error(line);
    }
    if ((e.id == 2 || e.id == 12) && (e.args).size() != 2) throw runtime_error(line);
    else if (e.id == 2 || e.id == 12) {
        for (char c : (e.args)[0])
            if (!(isalnum(c) || c == '_' || c == '-')) throw runtime_error(line);
        for (char c : (e.args)[1]) if (!isdigit(c)) throw runtime_error(line);
    }
}

int validateNum(const string& line) {
	int number = stoi(line);
    for (char c : line) if (!isdigit(c)) throw runtime_error(line);
    return number;
}