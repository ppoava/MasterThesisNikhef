// improvedPlotting.C
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

// Include ROOT headers
#include "TFile.h"
#include "TH1D.h"
#include "TCanvas.h"

using json = nlohmann::json;

// Define a structure to hold OS and SS correlation file names
struct TriggerAssociateOSandSS {
    std::string trigger;
    std::string associateOS;
    std::string associateSS;
    std::string OS;
    std::string SS;
};

struct HistogramAndTriggerPtHistogramNames {
    std::string hDPhi;
    std::string hTrPt;
};

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
        pair.trigger = configPair["trigger"].get<std::string>();
        pair.associateOS = configPair["associateOS"].get<std::string>();
        pair.associateSS = configPair["associateSS"].get<std::string>();
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
        pair.trigger = configPair["trigger"].get<std::string>();
        pair.associateOS = configPair["associateOS"].get<std::string>();
        pair.associateSS = configPair["associateSS"].get<std::string>();
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


// Simple function to calculate the yield given by two normalised OS and SS histograms
// Their angular spectra are subtracted (OS - SS) to reduce background
// and the full spectrum is integrated, though there is a posiblity to chose the integration range
// (if desired)
Double_t calculateOneYield(TH1D *hDPhiOS, TH1D *hTrPtOS, TH1D *hDPhiSS, TH1D *hTrPtSS, const char* FLAVOUR, Int_t i, Int_t j, Int_t k) {

    // Normalise by number of triggers
	hDPhiOS->Scale(1/hTrPtOS->Integral());
	hDPhiSS->Scale(1/hTrPtSS->Integral());
    // std::cout << "hDPhiOS->Integral() = " << hDPhiOS->Integral() << std::endl;
    // std::cout << "hTrPtOS->Integral() = " << hTrPtOS->Integral() << std::endl;

    TH1D *hCorr = (TH1D*)hDPhiOS->Clone();
    hCorr->Add(hDPhiSS, -1.);
    // std::cout << "hCorr->Integral() = " << hCorr->Integral() << std::endl;

    // TODO: add option in configuration.json to show angular correlation spectra
    // User can define OS, SS, for which associate, which dependency, whatever...
    /*
    TCanvas *chDPhiOS = new TCanvas(Form("chDPhiOS_%s_%i%i%i",FLAVOUR,i,j,k),Form("chDPhiOS_%s_%i%i%i",FLAVOUR,i,j,k),600,800);
    chDPhiOS->cd();
    hDPhiOS->Draw("hist");
    TCanvas *chDPhiSS = new TCanvas(Form("chDPhiSS_%s_%i%i%i",FLAVOUR,i,j,k),Form("chDPhiSS_%s_%i%i%i",FLAVOUR_i,j,k),600,800);
    chDPhiSS->cd();
    hDPhiSS->Draw("hist");
    TCanvas *cCorr = new TCanvas(Form("cCorr_%s_%i%i%i",FLAVOUR,i,j,k),Form("cCorr_%s_%i%i%i",FLAVOUR,i,j,k),600,800);
    cCorr->cd();
    hCorr->Draw("hist");
    */

    return hCorr->Integral();
} // calculateOneYield()


// Yields are calculated by looping over TUNES (e.g. MONASH),
// then ASSOCIATES (e.g. B-)
// and then DEPENDENCIES (e.g. DPhiLL)
// Function needs to be called for the FLAVOUR seperately (e.g. Beauty)
// The output is a 3D vector with the structure
// v[TUNE][ASSOCIATE][DEPENDENCY]
std::vector<std::vector<std::vector<Double_t>>> calculateYieldsVector(CONFIGS configs_from_json, const char* FLAVOUR) {

    std::cout << "*** Calculating yields for " << FLAVOUR << " ***" << std::endl;

    std::vector<std::vector<std::vector<Double_t>>> vYields;

    // Retrieve settings from configuration.json
    std::string base_dir = configs_from_json.base_dir;
    std::vector<std::string> vTUNES = configs_from_json.vTUNES;
    std::string complete_root_dir;
    if (strcmp(FLAVOUR, "BEAUTY") == 0) { complete_root_dir = configs_from_json.bbBarDir; }
    if (strcmp(FLAVOUR, "CHARM") == 0)  { complete_root_dir = configs_from_json.ccBarDir; }
    std::vector<TriggerAssociateOSandSS> vTriggerAssociateOSandSS;
    if (strcmp(FLAVOUR, "BEAUTY") == 0) { vTriggerAssociateOSandSS = configs_from_json.vBeautyTriggerAssociateOSandSS; }
    if (strcmp(FLAVOUR, "CHARM") == 0)  { vTriggerAssociateOSandSS = configs_from_json.vCharmTriggerAssociateOSandSS; }
    std::vector<HistogramAndTriggerPtHistogramNames> vHistogramAndTriggerPtHistogramNames = configs_from_json.vHistogramAndTriggerPtHistogramNames;


    // Loop over TUNES
    for (Int_t i=0; i<vTUNES.size(); i++) {


        std::string TUNE = vTUNES[i];
        std::cout << "starting loop over " << TUNE << std::endl;
        std::cout << std::endl;


        // Loop over ASSOCIATES
        for (Int_t j=0; j<vTriggerAssociateOSandSS.size(); j++) {


            TriggerAssociateOSandSS fileNamesOSandSS = vTriggerAssociateOSandSS[j];
            std::cout << "starting loop over OS file: " << fileNamesOSandSS.OS << " and SS file: " << fileNamesOSandSS.SS << std::endl;
            TFile *OStree = new TFile((base_dir + "/" + TUNE + "/" + complete_root_dir + "_" + TUNE + "/" + fileNamesOSandSS.OS).c_str());
            TFile *SStree = new TFile((base_dir + "/" + TUNE + "/" + complete_root_dir + "_" + TUNE + "/" + fileNamesOSandSS.SS).c_str());
            std::cout << std::endl;


            // Loop over DEPENDENCIES
            for (Int_t k=0; k<vHistogramAndTriggerPtHistogramNames.size(); k++) {


                HistogramAndTriggerPtHistogramNames hDPhiAndhTrPtNames = vHistogramAndTriggerPtHistogramNames[k];
                std::cout << "analysing histogram " << hDPhiAndhTrPtNames.hDPhi << " with trigger pT histogram " << hDPhiAndhTrPtNames.hTrPt << std::endl;

                TH1D* hDPhiOS = (TH1D*)OStree->Get((hDPhiAndhTrPtNames.hDPhi).c_str());
	            TH1D* hDPhiSS = (TH1D*)SStree->Get((hDPhiAndhTrPtNames.hDPhi).c_str());
	            TH1D* hTrPtOS = (TH1D*)OStree->Get((hDPhiAndhTrPtNames.hTrPt).c_str());
	            TH1D* hTrPtSS = (TH1D*)SStree->Get((hDPhiAndhTrPtNames.hTrPt).c_str());

	            if (strcmp((fileNamesOSandSS.trigger).c_str(), 
                           (fileNamesOSandSS.associateSS).c_str()) == 0) { 
                    hDPhiSS->Scale(0.5); } // Prevent double-counting

                // Calculate yield value and assign to appropriate place in vector
                Double_t yield = calculateOneYield(hDPhiOS, hTrPtOS, hDPhiSS, hTrPtSS, FLAVOUR, i, j, k);
                if (i >= vYields.size()) { vYields.resize(i + 1); }
                if (j >= vYields[i].size()) { vYields[i].resize(j + 1); }
                if (k >= vYields[i][j].size()) { vYields[i][j].resize(k + 1); }
                vYields[i][j][k] = yield; 
                std::cout << "vYields[" << i << "][" << j << "][" << k << "] = " << vYields[i][j][k] << std::endl;
                std::cout << std::endl;


            } // Loop over DEPENDENCIES


        } // Loop over ASSOCIATES


    } // Loop over TUNES


    return vYields;


} // calculateYieldsVector()


void drawBalancingPlots(CONFIGS configs_from_json, const char* FLAVOUR, std::vector<std::vector<std::vector<Double_t>>> vYields) {


    std::cout << "*** Drawing balancing plots for " << FLAVOUR << " ***" << std::endl;


    // Retrieve settings from configuration.json
    std::string base_dir = configs_from_json.base_dir;
    std::vector<std::string> vTUNES = configs_from_json.vTUNES;
    std::vector<TriggerAssociateOSandSS> vTriggerAssociateOSandSS;
    if (strcmp(FLAVOUR, "BEAUTY") == 0) { vTriggerAssociateOSandSS = configs_from_json.vBeautyTriggerAssociateOSandSS; }
    if (strcmp(FLAVOUR, "CHARM") == 0)  { vTriggerAssociateOSandSS = configs_from_json.vCharmTriggerAssociateOSandSS; }
    std::vector<HistogramAndTriggerPtHistogramNames> vHistogramAndTriggerPtHistogramNames = configs_from_json.vHistogramAndTriggerPtHistogramNames;


    // Values will be drawn from a 2D vector of TH1D with number of ASSOCIATES bins
    // This way the TUNE and DEPENDENCY can be looped over, while the data points will be the ASSOCIATES
    std::vector<std::vector<TH1D*>> histograms2D;
    Int_t nAssociates = vTriggerAssociateOSandSS.size();
    std::cout << "number of associates: " << nAssociates << std::endl;
    histograms2D.resize(nAssociates);

    // Define a template for this plot to set titles, stats, etc.
    TH1D *hYieldsTemplate = new TH1D(Form("hYieldsTemplate_%s", FLAVOUR), "hYieldsTemplate", nAssociates, 0, nAssociates);

    TCanvas *cYields = new TCanvas(Form("cYields_%s", FLAVOUR), Form("cYields_%s", FLAVOUR), 800, 600);
    cYields->cd();
    hYieldsTemplate->Draw("hist");


    // Loop over TUNES
    for (Int_t i=0; i<vTUNES.size(); i++) {


        std::string TUNE = vTUNES[i];
        std::cout << "starting loop over " << TUNE << std::endl;
        std::cout << std::endl;


        // Loop over ASSOCIATES
        for (Int_t j=0; j<nAssociates; j++) {


            TriggerAssociateOSandSS fileNamesOSandSS = vTriggerAssociateOSandSS[j];
            std::cout << "starting loop over associate: " << fileNamesOSandSS.associateOS << std::endl;
            std::cout << "starting loop over OS file: " << fileNamesOSandSS.OS << " and SS file: " << fileNamesOSandSS.SS << std::endl;

            if (i==0) { // only set the template histogram once

            }


            std::cout << std::endl;

            // Loop over DEPENDENCIES
            for (Int_t k=0; k<vHistogramAndTriggerPtHistogramNames.size(); k++) {


                HistogramAndTriggerPtHistogramNames hDPhiAndhTrPtNames = vHistogramAndTriggerPtHistogramNames[k];
                std::cout << "plotting histogram " << hDPhiAndhTrPtNames.hDPhi << " with trigger pT histogram " << hDPhiAndhTrPtNames.hTrPt << std::endl;
                std::cout << std::endl;


            } // Loop over DEPENDENCIES


        } // Loop over ASSOCIATES


    } // Loop over TUNES



    // Define associate label names for yield plots
    // hYieldsTemplate->GetXaxis()->SetBinLabel(1+i/Nhist, histEntry.AssociateNameOS);

    return;
} // drawBalancingPlots()


int improvedPlotting() {

    // Read configurations defined by user in configuration.json
    CONFIGS configs_from_json = readConfig();

    // Calculate the 3D yield vector
    std::vector<std::vector<std::vector<Double_t>>> vYieldsBeauty;
    std::vector<std::vector<std::vector<Double_t>>> vYieldsCharm;
    vYieldsBeauty = calculateYieldsVector(configs_from_json,"BEAUTY");
    vYieldsCharm =  calculateYieldsVector(configs_from_json,"CHARM");

    // Draw the balancing plots using the 3D yield vector
    drawBalancingPlots(configs_from_json,"BEAUTY",vYieldsBeauty);

    return 0;
}