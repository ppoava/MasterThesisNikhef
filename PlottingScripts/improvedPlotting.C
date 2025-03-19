// improvedPlotting.C
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

// Include ROOT headers
#include "TFile.h"
#include "TH1F.h"

using json = nlohmann::json;

// Define a structure to hold OS and SS correlation file names
struct TriggerAssociateOSandSS {
    std::string OS;
    std::string SS;
};

void readConfig() {

    // Open the JSON configuration file
    std::ifstream configFile("configuration.json");
    if (!configFile.is_open()) {
        std::cerr << "Error opening configuration file." << std::endl;
        return;
    }

    // Parse the JSON file
    json config;
    configFile >> config;

    // Extract values from the JSON
    // RootFiles path ("base directory")
    std::string base_dir = config["base_dir"];

    // Tunes
    std::vector<std::string> vTUNES;
    for (const auto& TUNE : config["PYTHIA_TUNES"]) {
        vTUNES.push_back(TUNE);
    }

    // Complete_root_dirs
    std::string bbBarDir = config["bb_bar_complete_root_dir"];
    std::string ccBarDir = config["cc_bar_complete_root_dir"];

    // Check if everything went well
    std::string path_to_complete_root_dir;
    for (const auto& TUNE : vTUNES) {
        path_to_complete_root_dir = base_dir + "/" + TUNE + "/" + bbBarDir;
        std::cout << "Full path beauty " << TUNE << ": " << path_to_complete_root_dir << std::endl;
    }
    for (const auto& TUNE : vTUNES) {
        path_to_complete_root_dir = base_dir + "/" + TUNE + "/" + ccBarDir;
        std::cout << "Full path charm " << TUNE << ": " << path_to_complete_root_dir << std::endl;
    }
    std::cout << std::endl;
    std::cout << "OKAY (or not)" << std::endl;
    std::cout << std::endl;

    // Which correlations need to be analysed?
    std::vector<TriggerAssociateOSandSS> vBeautyTriggerAssociateOSandSS;
    for (const auto& filePair : config["correlations_to_analyse"]) {
        TriggerAssociateOSandSS pair;
        pair.OS = filePair["OS"].get<std::string>();
        pair.SS = filePair["SS"].get<std::string>();
        vBeautyTriggerAssociateOSandSS.push_back(pair);
    }
    for (const auto& pair : vBeautyTriggerAssociateOSandSS) {
        std::cout << "OS File: " << pair.OS << ", SS File: " << pair.SS << std::endl;
    }
    std::cout << std::endl;

} // readConfig()

int improvedPlotting() {
    readConfig();
    return 0;
}