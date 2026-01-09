#include "analyzer.h" // others already included in analayzer.h.
#include <fstream> // for opening and reading csv file.
#include <sstream> // read a string file. Used for parsing (spliting a csv file lines into pieces for (TripID,PickupZone,PickupTime). -- Note from 28/12/2025 have a look at sstream tomorrow
#include <algorithm>
#include <cctype>

using namespace std;

void trim(string &s) {
    size_t start = 0;
    while (start < s.size() && isspace(static_cast<unsigned char>(s[start]))) {
        start++;
    }

    size_t end = s.size();
    while (end > start && isspace(static_cast<unsigned char>(s[end - 1]))) {
        end--;
    }

     if (start == 0 && end == s.size()) {
        return;
    }

    s = s.substr(start, end - start);
}

void TripAnalyzer::ingestFile(const std::string& csvPath) {

// TODO:
// - open file
// - skip header
// - skip malformed rows
// - extract PickupZoneID and pickup hour
// - aggregate counts

    zoneCounts.clear();
    slotCounts.clear();

    zoneCounts.reserve(50000); // we reserve to prevent rehashing(changing memory).
    slotCounts.reserve(50000);

    ifstream fileOpener(csvPath);
    if(!fileOpener.is_open()) {return; } // if the file is not opening return.
    string line;

    // fileOpener : is a object in ifstream that contains current read position.
    if(!getline(fileOpener, line)) {return; } // if its empty return.

    while(getline(fileOpener, line)){ // getline:read a line from file. We use while to do "getline" for each line in file.

        if (line.empty()) continue;
        
        string tripID;
        string pickupZone;
        string dropoffZone;
        string pickupDateTime; 
        string distanceTaken;
        string rideFee; // to give a place to stay for divided lines.
        
        size_t startPos = 0; // Start index position for reading operation.
        size_t encountedCommaPos; // First encounter of ',' index after reading starting from startPos.

        //TripID
        encountedCommaPos = line.find(',', startPos); // start from startPos and find the first ',' that encounters and asaign the index of first comma to encountedCommaPos.
        if (encountedCommaPos == string::npos) {continue;} // if it cant find ',' its dirty data.
        tripID = line.substr(startPos, encountedCommaPos - startPos); // get the line between start position and found ',' (not including ',') and save it.
        startPos = encountedCommaPos + 1; // we do this because we have more columns in a line so we need to rememberwhere its left of to start the new operation.

        //pickupZone
        encountedCommaPos = line.find(',', startPos); // because pickupZone comes after TripID we use startPos as where we left of (after the lsat encountedCommaPos).
        if (encountedCommaPos == string::npos) {continue;} 
        pickupZone = line.substr(startPos, encountedCommaPos - startPos); 
        startPos = encountedCommaPos + 1; 

        size_t comma3 = line.find(',', startPos); // create a new size_t to store the postion for solving the confilct with github and hackerrank ( 3 coloumns and 6 columns confilict).

        if (comma3 == string::npos) {
            pickupDateTime = line.substr(startPos); // if there is no comma after then its 3 coloumn format. so its pickupDateTime.
        } else {
            size_t comma4 = line.find(',', comma3 + 1); // we check for another.

            if (comma4 == string::npos) {
                pickupDateTime = line.substr(startPos);// if there is no comma after then its pickupDateTime.
            } else { // after this point ,we are sure its 6 coloumn format so we move on with the same operation for parsing.

                //dropoffZone
                dropoffZone = line.substr(startPos, comma3 - startPos);

                // pickupDateTime
                pickupDateTime = line.substr(comma3 + 1, comma4 - (comma3 + 1));

                // distanceTaken 
                size_t comma5 = line.find(',', comma4 + 1);
                if (comma5 == string::npos) continue;
                distanceTaken = line.substr(comma4 + 1, comma5 - (comma4 + 1));

                // rideFee 
                rideFee = line.substr(comma5 + 1);
            }
        }

        trim(tripID);
        trim(pickupZone);
        trim(pickupDateTime); // get rid of spaces.

        if (tripID.empty())          continue;
        if (pickupZone.empty())      continue;
        if (pickupDateTime.empty())  continue; // even if we do the !getline to prevent empty parts, getline still can take " " in a variable. We check the variables if its empty or not.


        size_t colonIndex = pickupDateTime.find(':'); // we find the ':' to seperate hours from pickupDateTime
        if (colonIndex == string::npos || colonIndex < 2) continue; // to find dirty data and skip it.

        //the reseon we use unisgned char is safety, its to prevent isdigit to fail when we check that.
        //with 'static_cast<unsigned char>' we force it to be unsigned char.(If the data has a value other than 0-255 we force its value to be in 0-255 with static_char<unisgend char>)
        unsigned char hour1 = static_cast<unsigned char>(pickupDateTime[colonIndex - 2]);
        unsigned char hour2 = static_cast<unsigned char>(pickupDateTime[colonIndex - 1]);

        if (!isdigit(hour1) || !isdigit(hour2)) {continue;} // we check if its a number.

        string seperatedHourString; // we create a string to convert hours to integar.
        seperatedHourString.push_back(hour1); // we push the chars from back to add it to string.
        seperatedHourString.push_back(hour2);

        int seperatedHourInt = stoi(seperatedHourString);
        if (seperatedHourInt < 0 || seperatedHourInt > 23) continue;

    // AGGREGAETE COUNTS
    zoneCounts[pickupZone]++; // go to the next line and save the information of that line.

    string combinedZoneAndHour = pickupZone + "@" + to_string(seperatedHourInt); // we are changing integer again to string because when we changed the string hour to integer, we validated it.

    slotCounts[combinedZoneAndHour]++; //  go to the next line and save the information of that line.
    }


}

    vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    // TODO:
    // - sort by count desc, zone asc
    // - return first k

    vector<ZoneCount> result;
    for (const auto& pair : zoneCounts) {
    ZoneCount zc;
    zc.zone = pair.first; 
    zc.count = pair.second; 
    result.push_back(zc);
        }
    sort(result.begin(), result.end(), [](const ZoneCount& a, const ZoneCount& b) {

    if (a.count != b.count) {
    return a.count > b.count; }
    return a.zone < b.zone;
        });
    if (k > 0 && k < (int)result.size()) {
    result.resize(k);
        }
    return result;
}

vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {

// TODO:
// - sort by count desc, zone asc, hour asc
// - return first k

    vector<SlotCount> results;

    for (const auto& entry : slotCounts) {


    string combinedKey = entry.first;
    long long count = entry.second;

    size_t atSymbolPos = combinedKey.find('@');

    if (atSymbolPos != string::npos) {

    string zoneName = combinedKey.substr(0, atSymbolPos);
    string hourStr = combinedKey.substr(atSymbolPos + 1);

    int hourInt = std::stoi(hourStr);

    SlotCount sc;
    sc.zone = zoneName;
    sc.hour = hourInt;
    sc.count = count;
    results.push_back(sc);
    }
}


sort(results.begin(), results.end(), [](const SlotCount& a, const SlotCount& b) {

    if (a.count != b.count) {
    return a.count > b.count; 
    }

    if (a.zone != b.zone) {
    return a.zone < b.zone; 
    }

    return a.hour < b.hour; 
    });

    if (k > 0 && k < (int)results.size()) {
    results.resize(k);
    }

    return results;
}

