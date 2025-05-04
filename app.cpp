#include "func.h"
#include <fstream>
#include <sstream>
using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }

    ifstream file(argv[1]);
    if (!file) {
        cerr << "Cannot open file" << endl;
        return 1;
    }

    try {
        string line;

        getline(file, line);
        int tables = validateNum(line);

        getline(file, line);
        Time open, close;
        stringstream ss(line);
        string openStr, closeStr;
        ss >> openStr >> closeStr;
        open = parseTime(openStr);
        close = parseTime(closeStr);
        if (close <= open || close.hours == -1 || open.hours == -1) 
            throw runtime_error(line);

        getline(file, line);
        int hourCost = validateNum(line);

        ComputerClub club(tables, open, close, hourCost);

            getline(file, line);
            validateLine(line);
            Event e(line);
            validateEvent(e, line);
            open = e.time;
            club.processEvent(e);

        while (getline(file, line)) {
            validateLine(line);
            Event e(line);
            validateEvent(e, line);
            if (e.time >= open) open = e.time;
            else throw runtime_error(line);
            club.processEvent(e);
        }

        club.endDay();
        club.printResults();

    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    return 0;
}