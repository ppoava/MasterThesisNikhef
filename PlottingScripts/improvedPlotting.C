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
    std::string bbBarDir = config["path_to_bb_bar_complete_root_dir"];
    std::cout << "path_to_bb_bar_complete_root_dir: " << bbBarDir << std::endl;

    // Which correlations need to be analysed?
    std::vector<TriggerAssociateOSandSS> vBeautyTriggerAssociateOSandSS;
    for (const auto& filePair : config["correlations_to_analyse"]) {
        TriggerAssociateOSandSS pair;
        pair.OS = bbBarDir + "/" + filePair["OS"].get<std::string>();
        pair.SS = bbBarDir + "/" + filePair["SS"].get<std::string>();
        vBeautyTriggerAssociateOSandSS.push_back(pair);
    }
    for (const auto& pair : vBeautyTriggerAssociateOSandSS) {
        std::cout << "OS File: " << pair.OS << ", SS File: " << pair.SS << std::endl << std::endl;
    }

} // readConfig()

int improvedPlotting() {
    readConfig();
    return 0;
}