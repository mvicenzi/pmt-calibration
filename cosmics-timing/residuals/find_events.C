#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <tuple>
#include <string>
#include <map>

#include "TFile.h"
#include "TTree.h"

using namespace std;

// Struct to hold match results
struct EventMatch {
    string file;
    string original;
    int run;
    int event;
    int cryo;
    size_t index;
};

std::string extract_name(const std::string& current_filename)
{
    string original_file = "UNKNOWN";

    // Strip directory path
    string fname = current_filename.substr(current_filename.find_last_of('/') + 1);
    
    // Find where the real original name starts
    auto pos = fname.find("compressed_data_");
    if (pos != string::npos) {
    
        // Keep only from "compressed_data_"
        string core = fname.substr(pos);
    
        // Split on '_'
        vector<string> tokens;
        size_t start = 0, end;
        while ((end = core.find('_', start)) != string::npos) {
            tokens.push_back(core.substr(start, end - start));
            start = end + 1;
        }
        tokens.push_back(core.substr(start));
    
        // Rebuild until first timestamp token
        string rebuilt;
        for (const auto& tok : tokens) {
            if (!rebuilt.empty())
                rebuilt += "_";
            rebuilt += tok;
    
            // YYYYMMDDThhmmss
            if (tok.size() >= 15 && tok[8] == 'T') {
                original_file = rebuilt + ".root";
                break;
            }
        }
    }
    return original_file;       
}


void finder(
    const vector<tuple<int, int, int>>& event_pairs,
    const string& file_list_path =
        "/exp/icarus/app/users/mvicenzi/timework/jobs/prods/files_calibnutples_std_xrootd.list"
) {
    // ------------------------------------------------------------------
    // Read file list
    // ------------------------------------------------------------------
    vector<string> files;
    ifstream file_stream(file_list_path);
    string line;

    if (!file_stream.is_open()) {
        cout << "Error: Could not open file list: " << file_list_path << endl;
        return;
    }

    while (getline(file_stream, line)) {
        if (!line.empty()) {
            files.push_back(line);
        }
    }
    file_stream.close();

    cout << "Loaded " << files.size() << " files from file list" << endl;

    // ------------------------------------------------------------------
    // Build lookup: (run,event) -> cryo
    // ------------------------------------------------------------------
    map<pair<int, int>, int> target_map;
    set<pair<int, int>> remaining;

    for (const auto& [run, event, cryo] : event_pairs) {
        target_map[{run, event}] = cryo;
        remaining.insert({run, event});
    }

    vector<EventMatch> matches;

    // ------------------------------------------------------------------
    // Loop over files
    // ------------------------------------------------------------------
    int i = 0;
    for (const auto& file_path : files) {

        if( i%10 ==0 ) {
            cout << "Processing file " << i << " / " << files.size() << endl;
        }       

        // Stop early if everything was found
        if (remaining.empty()) {
            cout << "All target events found, stopping early." << endl;
            break;
        }

        TFile* root_file = TFile::Open(file_path.c_str(), "READ");
        if (!root_file || root_file->IsZombie()) {
            cout << "Error: Could not open " << file_path << endl;
            delete root_file;
            continue;
        }

        TTree* evtTree =
            dynamic_cast<TTree*>(root_file->Get("simpleLightAna/eventstree"));
        if (!evtTree) {
            cout << "Error: Could not find eventstree in " << file_path << endl;
            root_file->Close();
            delete root_file;
            continue;
        }

        int run = 0, event = 0;
        evtTree->SetBranchAddress("run", &run);
        evtTree->SetBranchAddress("event", &event);

        const Long64_t nentries = evtTree->GetEntries();
        vector<int> all_events;
        all_events.reserve(evtTree->GetEntries());

        for (Long64_t i = 0; i < nentries; ++i) {
            evtTree->GetEntry(i);
            all_events.push_back(event);

            pair<int, int> key{run, event};
            auto it = remaining.find(key);

            if (it != remaining.end()) {
                int cryo = target_map[key];
                string current_filename = root_file->GetName();

                sort(all_events.begin(), all_events.end());
                auto jt = lower_bound(all_events.begin(), all_events.end(), key.second);
                size_t index = distance(all_events.begin(), jt);

                cout << "Match found in " << current_filename
                     << " for (run, event, cryo)=("
                     << run << ", " << event << ", " << cryo << ")"
                     << "at index " << index
                     << endl;

                string original_file = extract_name(current_filename);

                matches.push_back({current_filename, original_file, run, event, cryo, index});
                remaining.erase(it);

                // Stop looping entries if done
                if (remaining.empty())
                    break;
            }
        }

        root_file->Close();
        delete root_file;
        i++;
    }

    // ------------------------------------------------------------------
    // Summary
    // ------------------------------------------------------------------

    string out_filelist = "matched_files.csv";
    ofstream out(out_filelist);
    out << "original_file,event,index,cryo" << std::endl;

    if (!out.is_open()) {
        cout << "Error: could not open output file list " << out_filelist << endl;
    }

    cout << "\n=== Summary ===" << endl;
    cout << "Found " << matches.size() << " matching events" << endl;

    for (const auto& match : matches) {
        cout << "  File: " << match.file << std::endl
             << "  Raw name: " << match.original << std::endl
             << " | run: " << match.run
             << " | event: " << match.event
             << " | cryo: " << match.cryo
             << " | index: " << match.index
             << std::endl;

        out << match.original << "," << match.event << "," << match.index << "," << match.cryo << std::endl;
    }

    out.close();

    if (!remaining.empty()) {
        cout << "\nMissing events:" << endl;
        for (const auto& [run, event] : remaining) {
            cout << "  run=" << run << " event=" << event << endl;
        }
    }
}

// ------------------------------------------------------------------
// Driver
// ------------------------------------------------------------------
void find_events()
{
    vector<tuple<int, int, int>> event_pairs = {
        {9435, 1117, 1},
        {9435, 1189, 1}
    };

    string fileList =
        "/exp/icarus/app/users/mvicenzi/timework/jobs/prods/files_calibnutples_std_xrootd.list";

    finder(event_pairs, fileList);
}
