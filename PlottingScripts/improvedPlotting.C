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

struct HistogramAndTriggerPtHistogramNames {
    std::string hDPhi;
    std::string hTrPt;
};

void normalise(TH1D* hist, TH1D* trig, Double_t xmin, Double_t xmax) {
	Int_t bin_start = trig->FindBin(xmin);
	Int_t bin_finish = trig->FindBin(xmax);
	Double_t integral = trig->Integral(bin_start, bin_finish);
	std::cout << "function integral = " << integral << std::endl;
	hist->Scale(1./integral);
}

// To be taken from the configuration.json and send to main code
// output from readConfig()
struct CONFIGS {
    // General
    std::string base_dir;

    // MONASH, JUNCTIONS, else...
    std::vector<std::string> vTUNES;

    // Which complete_root production? (beauty and charm)
    std::string bbBarDir;
    std::string ccBarDir;

    // (BplusBminus.root,BplusBplus.root), ...
    std::vector<TriggerAssociateOSandSS> vBeautyTriggerAssociateOSandSS;
    std::vector<TriggerAssociateOSandSS> vCharmTriggerAssociateOSandSS;

    // (hDPhiLL, hTrPtL), ...
    std::vector<HistogramAndTriggerPtHistogramNames> vHistogramAndTriggerPtHistogramNames;
};

CONFIGS readConfig() {

    std::cout << "Reading configuration.json" << std::endl;
    std::cout << std::endl;

    // Open the JSON configuration file
    std::ifstream configFile("configuration.json");
    if (!configFile.is_open()) {
        std::cerr << "Error opening configuration file." << std::endl;
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

    // Which correlations need to be analysed?
    std::vector<TriggerAssociateOSandSS> vBeautyTriggerAssociateOSandSS;
    for (const auto& configPair : config["beauty_correlations_to_analyse"]) {
        TriggerAssociateOSandSS pair;
        pair.OS = configPair["OS"].get<std::string>();
        pair.SS = configPair["SS"].get<std::string>();
        vBeautyTriggerAssociateOSandSS.push_back(pair);
    }
    for (const auto& pair : vBeautyTriggerAssociateOSandSS) {
        std::cout << "OS File: " << pair.OS << ", SS File: " << pair.SS << std::endl;
    }
    std::vector<TriggerAssociateOSandSS> vCharmTriggerAssociateOSandSS;
    for (const auto& configPair : config["charm_correlations_to_analyse"]) {
        TriggerAssociateOSandSS pair;
        pair.OS = configPair["OS"].get<std::string>();
        pair.SS = configPair["SS"].get<std::string>();
        vCharmTriggerAssociateOSandSS.push_back(pair);
    }
    for (const auto& pair : vBeautyTriggerAssociateOSandSS) {
        std::cout << "OS File: " << pair.OS << ", SS File: " << pair.SS << std::endl;
    }
    std::cout << std::endl;

    // Which histograms need to be analysed?
    // i.e. which dependencies (pT, mult, etc.)
    std::vector<HistogramAndTriggerPtHistogramNames> vHistogramAndTriggerPtHistogramNames;
    for (const auto& configPair : config["histograms_to_analyse"]) {
        HistogramAndTriggerPtHistogramNames pair;
        pair.hDPhi = configPair["hDPhi"].get<std::string>();
        pair.hTrPt = configPair["hTrPt"].get<std::string>();
        vHistogramAndTriggerPtHistogramNames.push_back(pair);
    }
    for (const auto& pair : vHistogramAndTriggerPtHistogramNames) {
        std::cout << "hDPhi: " << pair.hDPhi << ", hTrPt: " << pair.hTrPt << std::endl;
    }
    std::cout << std::endl;

    std::cout << "OKAY (or not)" << std::endl;
    std::cout << std::endl;

    // TODO: give short summary of settings given and what the output yield vector will look like

    CONFIGS configs_from_json;
    configs_from_json.base_dir = base_dir;
    configs_from_json.vTUNES = vTUNES;
    configs_from_json.bbBarDir = bbBarDir;
    configs_from_json.ccBarDir = ccBarDir;
    configs_from_json.vBeautyTriggerAssociateOSandSS = vBeautyTriggerAssociateOSandSS;
    configs_from_json.vCharmTriggerAssociateOSandSS = vCharmTriggerAssociateOSandSS;
    configs_from_json.vHistogramAndTriggerPtHistogramNames = vHistogramAndTriggerPtHistogramNames;

    return configs_from_json;

} // readConfig()

int improvedPlotting() {
    CONFIGS configs_from_json = readConfig();
    return 0;
}