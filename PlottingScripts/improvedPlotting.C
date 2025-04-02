// improvedPlotting.C
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <random>
#include <cmath>

// Include ROOT headers
#include "TFile.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TString.h"

using json = nlohmann::json;

// TODO: put structs in a header file (only after plotting is done in configuration.json)

// TODO: make slides with code screenshots and explanation

// TODO: add verbose/debug flags to configuration.json

// TODO: add strangeness to configurations (though won't work; no simulations available; maybe comment it out?)

// TODO: update the other configuration.json files with the "configuration.json" script

// TODO: add documentation describing all configurations in the json

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

struct YieldsAndErrors {
    std::vector<std::vector<std::vector<Double_t>>> vYields;
    std::vector<std::vector<std::vector<Double_t>>> vYieldsErrors;
    std::vector<std::vector<std::vector<Double_t>>> vYieldsRatioErrors;
};

struct LegendPair {
    std::string objectName; // the object you want to put (name e.g. = "B+" or "hDPhiLL"; depends on function call)
    std::string displayName; // what's shown in the legend
};

struct ColourOrLineStylePair {
    std::string drawObjectName; // what object are we giving a colour/linestyle?
    Int_t drawOptionName; // and what are we giving it?
};

struct canvasConfigs {
    std::string canvasName;
    std::string drawFunctionToUse; // name of functions defined in improvedPlotting()
    std::vector<std::string> vTUNES; // tune to be drawn on given canvas
    std::string FLAVOUR; // just one allowed, but could implement with a new function
    Int_t indexNominatorTUNE; // used for TUNE ratio plots, e.g. MONASH/JUNCTIONS to study enhancement explicitly
    Int_t indexDenominatorTUNE;

    // Save output
    bool write;
    std::string writePath;
    std::string writeName;

    // TODO: add other canvas settings (e.g. xMin, xMax, setLogy, etc.)
    Double_t xSizeCanvas; // new TCanvas(xSizeCanvas,ySizeCanvas)
    Double_t ySizeCanvas;
    Double_t yMinAxis; // ->SetRangeUser()
    Double_t yMaxAxis;
    Double_t hMinimum; // ->SetMinimum()
    Double_t hMaximum; 
    bool setLogy;
    Double_t xMinLegend; // new TLegend(xMinLegend,yMinLegend,xMaxLegend,yMaxLegend)
    Double_t xMaxLegend;
    Double_t yMinLegend;
    Double_t yMaxLegend;
    // Empty is best set with entries = -1, sizes need to always match!! (TODO: fix this? make this better? necessary?)
    std::vector<LegendPair> vLegendEntries;
    std::vector<ColourOrLineStylePair> vColoursTUNES; // give one per tune, needs to be same order as vTUNES above
    std::vector<ColourOrLineStylePair> vLineStylesDependencies; // give one per dependency, needs to be same order as dependencies given in configurations
    std::vector<ColourOrLineStylePair> vLineStylesBaryons; // give one per baryon (that is being checked for in the baryon/meson ratio function)
};

// To be taken from the configuration.json and send to main code
// output from readConfig()
struct CONFIGS {
    // General
    bool CALCULATE_ERRORS;
    int nSubSamples;
    std::string base_dir;

    // MONASH, JUNCTIONS, else...
    std::vector<std::string> vTUNES;

    // Which complete_root production? (beauty and charm)
    std::string bbBarDir;
    std::string ccBarDir;
    std::string bbBarDir_subSamples;
    std::string ccBarDir_subSamples;

    // (BplusBminus.root,BplusBplus.root), ...
    std::vector<TriggerAssociateOSandSS> vBeautyTriggerAssociateOSandSS;
    std::vector<TriggerAssociateOSandSS> vCharmTriggerAssociateOSandSS;

    // (hDPhiLL, hTrPtL), ...
    std::vector<HistogramAndTriggerPtHistogramNames> vHistogramAndTriggerPtHistogramNames;

    // Plotting settings
    std::vector<canvasConfigs> vCanvasConfigs;
    canvasConfigs canvasConfigs; // filled later, when looping over vCanvasConfigs, given as argument to plotting function
};

// Function to find the index of a tune name
int findTuneIndex(const std::vector<std::string>& vTUNES, const std::string& tuneName) {
    auto it = std::find(vTUNES.begin(), vTUNES.end(), tuneName);
    if (it != vTUNES.end()) {
        return std::distance(vTUNES.begin(), it);
    } else {
        return -1;
    }
}

CONFIGS readConfig(const char* configurations) {

    std::cout << std::endl;
    std::cout << "*** Reading configuration.json ***" << std::endl;
    std::cout << std::endl;

    // Open the JSON configuration file
    std::ifstream configFile(configurations);
    if (!configFile.is_open()) {
        std::cerr << "Error opening configuration file." << std::endl;
    }

    // Parse the JSON file
    json config;
    configFile >> config;

    // Extract values from the JSON

    // Generic options
    bool CALCULATE_ERRORS = config["calculate_errors"].get<bool>();
    std::string bbBarDir_sub_samples = config["bb_bar_complete_root_dir_sub_samples"];
    std::string ccBarDir_sub_samples = config["cc_bar_complete_root_dir_sub_samples"];
    int nSubSamples = config["nSubSamples"].get<int>();

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
    std::string path_to_complete_root_dir_sub_samples;
    for (const auto& TUNE : vTUNES) {
        path_to_complete_root_dir = base_dir + "/" + TUNE + "/" + bbBarDir;
        path_to_complete_root_dir_sub_samples = bbBarDir_sub_samples + "_" + TUNE;
        std::cout << "Full path beauty " << TUNE << ": " << path_to_complete_root_dir << std::endl;
        std::cout << "Full path beauty sub samples " << TUNE << ": " << path_to_complete_root_dir_sub_samples << std::endl;
    }
    for (const auto& TUNE : vTUNES) {
        path_to_complete_root_dir = base_dir + "/" + TUNE + "/" + ccBarDir;
        path_to_complete_root_dir_sub_samples = ccBarDir_sub_samples + "_" + TUNE;
        std::cout << "Full path charm " << TUNE << ": " << path_to_complete_root_dir << std::endl;
        std::cout << "Full path charm sub samples" << TUNE << ": " << path_to_complete_root_dir_sub_samples << std::endl;
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

    // How should everything be drawn?
    std::vector<canvasConfigs> vCanvasConfigs;
    for (const auto& configPair : config["canvases_to_be_drawn"]) {
        canvasConfigs pair;
        pair.canvasName = configPair["canvas_name"].get<std::string>();
        pair.drawFunctionToUse = configPair["draw_function_to_use"].get<std::string>();
        std::vector<std::string> vCanvasTUNES;
        for (const auto& TUNE : config["TUNES"]) {
            vCanvasTUNES.push_back(TUNE);
        }
        pair.FLAVOUR = configPair["FLAVOUR"].get<std::string>();
        std::string nominatorTuneName = configPair["nominator_TUNE"].get<std::string>();
        pair.indexNominatorTUNE = findTuneIndex(vTUNES, nominatorTuneName);
        if (pair.indexNominatorTUNE != -1) {
            std::cout << "Index of " << nominatorTuneName << " is: " << pair.indexNominatorTUNE << std::endl;
        } else {
            std::cout << nominatorTuneName << " ERROR: TUNE not found in vTUNES." << std::endl;
        }
        std::string denominatorTuneName = configPair["denominator_TUNE"].get<std::string>();
        pair.indexDenominatorTUNE = findTuneIndex(vTUNES, denominatorTuneName);
        if (pair.indexDenominatorTUNE != -1) {
            std::cout << "Index of " << denominatorTuneName << " is: " << pair.indexDenominatorTUNE << std::endl;
        } else {
            std::cout << nominatorTuneName << " ERROR: TUNE not found in vTUNES." << std::endl;
        }
        pair.write = configPair["write"].get<bool>();
        pair.writePath = configPair["write_path"].get<std::string>();
        pair.writeName = configPair["write_name"].get<std::string>();

        // Plotting settings
        pair.xSizeCanvas = configPair["x_size_canvas"].get<Double_t>();
        pair.ySizeCanvas = configPair["y_size_canvas"].get<Double_t>();
        pair.yMinAxis = configPair["y_min_axis"].get<Double_t>();
        pair.yMaxAxis = configPair["y_max_axis"].get<Double_t>();
        pair.hMinimum = configPair["histogram_minimum"].get<Double_t>();
        pair.hMaximum = configPair["histogram_maximum"].get<Double_t>();
        pair.setLogy = configPair["set_log_y"].get<bool>();
        pair.xMinLegend = configPair["x_min_legend"].get<Double_t>();
        pair.xMaxLegend = configPair["x_max_legend"].get<Double_t>();
        pair.yMinLegend = configPair["y_min_legend"].get<Double_t>();
        pair.yMinLegend = configPair["y_max_legend"].get<Double_t>();
        std::vector<LegendPair> vLegendEntries;
        for (const auto& configPair : config["legend_entries"]) {
            LegendPair pair;
            pair.objectName = configPair["object_name"].get<std::string>();
            pair.displayName = configPair["display_name"].get<std::string>();
            vLegendEntries.push_back(pair);
        }
        std::vector<ColourOrLineStylePair> vColoursTUNES;
        for (const auto& configPair : config["TUNE_colours"]) {
            ColourOrLineStylePair pair;
            pair.drawObjectName = configPair["TUNE_name"].get<std::string>();
            pair.drawOptionName = configPair["colour"].get<Int_t>();
            vColoursTUNES.push_back(pair);
        }
        std::vector<ColourOrLineStylePair> vLineStyleDependencies;
        for (const auto& configPair : config["dependency_line_styles"]) {
            ColourOrLineStylePair pair;
            pair.drawObjectName = configPair["dependency_name"].get<std::string>();
            pair.drawOptionName = configPair["draw_option"].get<Int_t>();
            vLineStyleDependencies.push_back(pair);
        }
        std::vector<ColourOrLineStylePair> vLineStylesBaryons;
        for (const auto& configPair : config["baryon_line_styles"]) {
            ColourOrLineStylePair pair;
            pair.drawObjectName = configPair["baryon_name"].get<std::string>();
            pair.drawOptionName = configPair["draw_option"].get<Int_t>();
            vLineStylesBaryons.push_back(pair);
        }
        // Sumarise configurations in struct, save per canvas
        vCanvasConfigs.push_back(pair);
    }
    for (const auto& pair : vCanvasConfigs) {
        std::cout << "canvasName: " << pair.canvasName << std::endl;
        std::cout << "drawFunctionToUse: " << pair.drawFunctionToUse << std::endl;
        // TODO: add print for vTUNES (after general print vector function)
    }
    std::cout << std::endl;


    // TODO: make a function that prints content of a vector
    CONFIGS configs_from_json;
    configs_from_json.CALCULATE_ERRORS = CALCULATE_ERRORS;
    configs_from_json.nSubSamples = nSubSamples;
    configs_from_json.base_dir = base_dir;
    configs_from_json.vTUNES = vTUNES;
    configs_from_json.bbBarDir = bbBarDir;
    configs_from_json.ccBarDir = ccBarDir;
    configs_from_json.bbBarDir_subSamples = bbBarDir_sub_samples;
    configs_from_json.ccBarDir_subSamples = ccBarDir_sub_samples;
    configs_from_json.vBeautyTriggerAssociateOSandSS = vBeautyTriggerAssociateOSandSS;
    configs_from_json.vCharmTriggerAssociateOSandSS = vCharmTriggerAssociateOSandSS;
    configs_from_json.vHistogramAndTriggerPtHistogramNames = vHistogramAndTriggerPtHistogramNames;
    configs_from_json.vCanvasConfigs = vCanvasConfigs;

    std::cout << "Configurations successfully read from JSON with parameters:" << std::endl;
    std::cout << "- CALCULATE_ERRORS = " << CALCULATE_ERRORS << std::endl;
    std::cout << "- nSubSamples = " << nSubSamples << std::endl;
    std::cout << "- base_dir = " << base_dir << std::endl;
    std::cout << "- vTUNES.size() = " << vTUNES.size() << std::endl;
    std::cout << "- bbBarDir = " << bbBarDir << std::endl;
    std::cout << "- ccBarDir = " << ccBarDir << std::endl;
    std::cout << "- bbBarDir_sub_samples = " << bbBarDir_sub_samples << std::endl;
    std::cout << "- ccBarDir_sub_samples = " << ccBarDir_sub_samples << std::endl;
    std::cout << "- vBeautyTriggerAssociateOSandSS.size() = " << vBeautyTriggerAssociateOSandSS.size() << std::endl;
    std::cout << "- vCharmTriggerAssociateOSandSS.size() = " << vCharmTriggerAssociateOSandSS.size() << std::endl;
    std::cout << "- vHistogramAndTriggerPtHistogramNames.size() = " << vHistogramAndTriggerPtHistogramNames.size() << std::endl;
    // vCanvasConfigs
    // TODO: give overview of canvases to be drawn and their settings etc.
    std::cout << std::endl;

    return configs_from_json;

} // readConfig()


// Simple function that returns the propagated error from a ratio A/B with errors for A and B
// Assumes A and B are uncorrelated
Double_t propagateRatioError(Double_t valueA, Double_t valueB, Double_t errorA, Double_t errorB) {
    Double_t relativeUncertainty = pow((errorA / valueA), 2) + pow((errorB / valueB), 2);
    return ((valueA / valueB) * sqrt(relativeUncertainty));
} // propagateRatioError()


// Simple function to calculate the yield given by two normalised OS and SS histograms
// Their angular spectra are subtracted (OS - SS) to reduce background
// and the full spectrum is integrated, though there is a posiblity to chose the integration range
// (if desired)
Double_t calculateOneYield(TH1D *hDPhiOS, TH1D *hTrPtOS, TH1D *hDPhiSS, TH1D *hTrPtSS, const char* FLAVOUR, 
                           Int_t i, Int_t j, Int_t k, Int_t l) {

    // Normalise by number of triggers
    std::cout << "hDPhiOS Integral: " << hDPhiOS->Integral() 
          << ", hDPhiSS Integral: " << hDPhiSS->Integral() << std::endl;
	hDPhiOS->Scale(1/hTrPtOS->Integral());
	hDPhiSS->Scale(1/hTrPtSS->Integral());
    std::cout << "hDPhiOS Integral: " << hDPhiOS->Integral() 
          << ", hDPhiSS Integral: " << hDPhiSS->Integral() << std::endl;

    TH1D *hCorr = (TH1D*)hDPhiOS->Clone();
    hCorr->Add(hDPhiSS, -1.);

    return hCorr->Integral();
} // calculateOneYield()


// Yields are calculated by looping over TUNES (e.g. MONASH),
// then ASSOCIATES (e.g. B-)
// and then DEPENDENCIES (e.g. DPhiLL)
// Function needs to be called for the FLAVOUR seperately (e.g. Beauty)
// The output is a 3D vector with the structure
// v[TUNE][ASSOCIATE][DEPENDENCY]
// TODO: change the .size() to variables nTUNES, etc. Like in the plotting function below
YieldsAndErrors calculateYieldsVector(CONFIGS configs_from_json, const char* FLAVOUR) {

    std::cout << "*** Calculating yields for " << FLAVOUR << " ***" << std::endl;

    // Retrieve settings from configuration.json
    bool CALCULATE_ERRORS = configs_from_json.CALCULATE_ERRORS;
    int nSubSamples = configs_from_json.nSubSamples;
    std::string base_dir = configs_from_json.base_dir;
    std::vector<std::string> vTUNES = configs_from_json.vTUNES;
    std::string complete_root_dir;
    if (strcmp(FLAVOUR, "BEAUTY") == 0) { complete_root_dir = configs_from_json.bbBarDir; }
    if (strcmp(FLAVOUR, "CHARM")  == 0) { complete_root_dir = configs_from_json.ccBarDir; }
    std::string complete_root_dir_sub_samples;
    if (strcmp(FLAVOUR, "BEAUTY") == 0) { complete_root_dir_sub_samples = configs_from_json.bbBarDir_subSamples; }
    if (strcmp(FLAVOUR, "CHARM")  == 0) { complete_root_dir_sub_samples = configs_from_json.ccBarDir_subSamples; }
    std::vector<TriggerAssociateOSandSS> vTriggerAssociateOSandSS;
    if (strcmp(FLAVOUR, "BEAUTY") == 0) { vTriggerAssociateOSandSS = configs_from_json.vBeautyTriggerAssociateOSandSS; }
    if (strcmp(FLAVOUR, "CHARM")  == 0) { vTriggerAssociateOSandSS = configs_from_json.vCharmTriggerAssociateOSandSS; }
    std::vector<HistogramAndTriggerPtHistogramNames> vHistogramAndTriggerPtHistogramNames = configs_from_json.vHistogramAndTriggerPtHistogramNames;

    // TODO: make these vectors into arrays, don't think vector is necessary
    // and the subYields are stored in an array anyways
    std::vector<std::vector<std::vector<Double_t>>> vYields;
    std::vector<std::vector<std::vector<Double_t>>> vYieldsErrors;
    std::vector<std::vector<std::vector<Double_t>>> vYieldsRatioErrors;
    Double_t vSubYields[vTUNES.size()][vTriggerAssociateOSandSS.size()][vHistogramAndTriggerPtHistogramNames.size()][nSubSamples];


    // TODO: make vTUNES.size into nTUNES, like in the plotting functions
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

                TH1D *hDPhiOS = (TH1D*)OStree->Get((hDPhiAndhTrPtNames.hDPhi).c_str());
	            TH1D *hDPhiSS = (TH1D*)SStree->Get((hDPhiAndhTrPtNames.hDPhi).c_str());
	            TH1D *hTrPtOS = (TH1D*)OStree->Get((hDPhiAndhTrPtNames.hTrPt).c_str());
	            TH1D *hTrPtSS = (TH1D*)SStree->Get((hDPhiAndhTrPtNames.hTrPt).c_str());

                // Prevent double-counting
	            if (strcmp((fileNamesOSandSS.trigger).c_str(), 
                           (fileNamesOSandSS.associateSS).c_str()) == 0) { 
                    hDPhiSS->Scale(0.5); } 


                // Calculate yield value and assign to appropriate place in vector
                Double_t yield = calculateOneYield(hDPhiOS, hTrPtOS, hDPhiSS, hTrPtSS, FLAVOUR, i, j, k, 0);
                if (i >= vYields.size()) { vYields.resize(i + 1); }
                if (j >= vYields[i].size()) { vYields[i].resize(j + 1); }
                if (k >= vYields[i][j].size()) { vYields[i][j].resize(k + 1); }
                vYields[i][j][k] = yield; 
                std::cout << "vYields[" << i << "][" << j << "][" << k << "] = " << vYields[i][j][k] << std::endl;
                std::cout << std::endl;


                // Calculate the error on the yield by subsampling with N samples
                // Not the most efficient way, but it is straightforward and clear
                // and anyways the files are quite small so it doesn't take too long
                if (CALCULATE_ERRORS) {
                    

                    // Error estimation is independent of binning, however one needs to make sure the 
                    // yield values are within the min and max bin ranges
                    TH1D *hSubYields = new TH1D(Form("hSubYields_%i%i%i", i, j, k), Form("hSubYields_%i%i%i", i, j, k), 50, vYields[i][j][k]/5, vYields[i][j][k]*5);
                    TH1D *hSubRatioYields = new TH1D(Form("hSubRatioYields_%i%i%i", i, j, k), Form("hSubRatioYields_%i%i%i", i, j, k), 50, (vYields[i][j][k]/vYields[i][0][k])/5, (vYields[i][j][k]/vYields[i][0][k])*5);


                    for (Int_t l = 1; l < nSubSamples+1; l++) {


                        TFile *OStree_subSamples = new TFile((complete_root_dir_sub_samples + "_" + TUNE + "/" + Form("combined_root_%i",l) + "/" + fileNamesOSandSS.OS).c_str());
                        TFile *SStree_subSamples = new TFile((complete_root_dir_sub_samples + "_" + TUNE + "/" + Form("combined_root_%i",l) + "/" + fileNamesOSandSS.SS).c_str());

                        TH1D *hDPhiOS_subSamples = (TH1D*)OStree_subSamples->Get((hDPhiAndhTrPtNames.hDPhi).c_str());
	                    TH1D *hDPhiSS_subSamples = (TH1D*)SStree_subSamples->Get((hDPhiAndhTrPtNames.hDPhi).c_str());
	                    TH1D *hTrPtOS_subSamples = (TH1D*)OStree_subSamples->Get((hDPhiAndhTrPtNames.hTrPt).c_str());
	                    TH1D *hTrPtSS_subSamples = (TH1D*)SStree_subSamples->Get((hDPhiAndhTrPtNames.hTrPt).c_str());

                        Double_t subYield = calculateOneYield(hDPhiOS_subSamples, hTrPtOS_subSamples, hDPhiSS_subSamples, hTrPtSS_subSamples,
                                                              FLAVOUR, i, j, k, l);
                        vSubYields[i][j][k][l] = subYield;
                        std::cout << "vSubYields[" << i << "][" << j << "][" << k << "][" << l << "] = " << subYield << std::endl;
                        std::cout << std::endl;

                        hSubYields->Fill(subYield);
                        hSubRatioYields->Fill((vSubYields[i][j][k][l])/(vSubYields[i][0][k][l]));

                        // TODO: debug checks not appearing anymore due to memory deletion?
                        TCanvas *cTestHDPhiOS;
                        if (i==0 && j==0 && k==0 && l==1) {
                            cTestHDPhiOS = new TCanvas("testHDPhiOS","testHDPhiOS",600,800);
                            cTestHDPhiOS->cd();
                            hDPhiOS_subSamples->Draw("hist");
                        }
                        if (i==0 && j==0 && k==0 && l==2) {
                            cTestHDPhiOS->cd();
                            hDPhiOS_subSamples->Draw("same hist");
                        }

                        // TODO: Close and delete more things? Memory issuse?
                        // Seems to automatically close histograms too?
                        // Free memory
                        OStree_subSamples->Close();
                        SStree_subSamples->Close();


                    } // Loop over SUBSAMPLES


                    TCanvas *cTestSubYields;
                    TCanvas *cTestSubRatioYields;
                    if (i==1 && j==4 && k==0) {
                            cTestSubYields = new TCanvas("testSubYields","testSubYields",600,800);
                            cTestSubYields->cd();
                            hSubYields->Draw("hist");
                        }
                    if (i==1 && j==4 && k==0) {
                        cTestSubRatioYields = new TCanvas("testSubRatioYields","testSubRatioYields",600,800);
                        cTestSubRatioYields->cd();
                        hSubRatioYields->Draw("hist");
                    }
                    Double_t yieldError = hSubYields->GetStdDev();
                    Double_t yieldRatioError = hSubRatioYields->GetStdDev();
                    if (i >= vYieldsErrors.size()) { vYieldsErrors.resize(i + 1); }
                    if (j >= vYieldsErrors[i].size()) { vYieldsErrors[i].resize(j + 1); }
                    if (k >= vYieldsErrors[i][j].size()) { vYieldsErrors[i][j].resize(k + 1); }
                    vYieldsErrors[i][j][k] = yieldError; 
                    std::cout << "vYieldsErrors[" << i << "][" << j << "][" << k << "] = " << vYieldsErrors[i][j][k] << std::endl;
                    std::cout << std::endl;
                    if (i >= vYieldsRatioErrors.size()) { vYieldsRatioErrors.resize(i + 1); }
                    if (j >= vYieldsRatioErrors[i].size()) { vYieldsRatioErrors[i].resize(j + 1); }
                    if (k >= vYieldsRatioErrors[i][j].size()) { vYieldsRatioErrors[i][j].resize(k + 1); }
                    vYieldsRatioErrors[i][j][k] = yieldRatioError; 
                    std::cout << "vYieldsRatioErrors[" << i << "][" << j << "][" << k << "] = " << vYieldsRatioErrors[i][j][k] << std::endl;
                    std::cout << std::endl;


                } // calculate errors


            } // Loop over DEPENDENCIES


            // TODO: cannot seem to draw 'test' plots for the yields anymore when this is enabled?
            // Free memory
            // OStree->Close();
            // SStree->Close();


        } // Loop over ASSOCIATES


    } // Loop over TUNES


    // TODO: make the names better
    YieldsAndErrors vYieldsAndErrors;
    vYieldsAndErrors.vYields = vYields;
    if (CALCULATE_ERRORS) { vYieldsAndErrors.vYieldsErrors = vYieldsErrors; }
    if (CALCULATE_ERRORS) { vYieldsAndErrors.vYieldsRatioErrors = vYieldsRatioErrors; }
    return vYieldsAndErrors;


} // calculateYieldsVector()


void drawBalancingPlots(CONFIGS configs_from_json, const char* FLAVOUR, YieldsAndErrors vYieldsAndErrors) {


    std::cout << "*** Drawing balancing plots for " << FLAVOUR << " ***" << std::endl;


    // Retrieve settings from configuration.json
    canvasConfigs canvasConfigs = configs_from_json.canvasConfigs;
    bool CALCULATE_ERRORS = configs_from_json.CALCULATE_ERRORS;
    std::string base_dir = configs_from_json.base_dir;
    std::vector<std::string> vTUNES = configs_from_json.vTUNES;
    std::vector<TriggerAssociateOSandSS> vTriggerAssociateOSandSS;
    if (strcmp(FLAVOUR, "BEAUTY") == 0) { vTriggerAssociateOSandSS = configs_from_json.vBeautyTriggerAssociateOSandSS; }
    if (strcmp(FLAVOUR, "CHARM") == 0)  { vTriggerAssociateOSandSS = configs_from_json.vCharmTriggerAssociateOSandSS; }
    std::vector<HistogramAndTriggerPtHistogramNames> vHistogramAndTriggerPtHistogramNames = configs_from_json.vHistogramAndTriggerPtHistogramNames;

    Int_t nTUNES = vTUNES.size();
    Int_t nAssociates = vTriggerAssociateOSandSS.size();
    Int_t nDependencies = vHistogramAndTriggerPtHistogramNames.size();

    // Values will be drawn from a 2D vector of TH1D with number of ASSOCIATES bins
    // This way the TUNE and DEPENDENCY can be looped over, while the data points will be the ASSOCIATES
    TH1D *vHists[nTUNES][nDependencies];
    // TODO: verbose
    // std::cout << "number of associates: " << nAssociates << std::endl;

    // Define a template for this plot to set titles, stats, etc.
    TH1D *hYieldsTemplate = new TH1D(Form("hYieldsTemplate_%s", FLAVOUR), Form("hYieldsTemplate_%s", FLAVOUR), nAssociates, 0, nAssociates);
    hYieldsTemplate->GetYaxis()->SetRangeUser(canvasConfigs.yMinAxis,canvasConfigs.yMaxAxis);

    TCanvas *cYields = new TCanvas(Form("cYields_%s", FLAVOUR), Form("cYields_%s", FLAVOUR), canvasConfigs.xSizeCanvas, canvasConfigs.ySizeCanvas);
    cYields->cd();
    gPad->SetLogy();
    hYieldsTemplate->SetStats(0);
    hYieldsTemplate->Draw("PE");


    // Loop over TUNES
    for (Int_t i=0; i<nTUNES; i++) {


        std::string TUNE = vTUNES[i];
        // TODO: verbose
        // std::cout << "starting loop over " << TUNE << std::endl;
        // std::cout << std::endl;


        // Loop over ASSOCIATES
        for (Int_t j=0; j<nAssociates; j++) {


            // TODO: fix this bug with associateName and formatting....
            TriggerAssociateOSandSS fileNamesOSandSS = vTriggerAssociateOSandSS[j];
            std::string associateName = fileNamesOSandSS.associateOS;
            // TODO: verbose
            // std::cout << "starting loop over associate: " << associateName << std::endl;
            // std::cout << "starting loop over OS file: " << fileNamesOSandSS.OS << " and SS file: " << fileNamesOSandSS.SS << std::endl;

            if (i==0) { // only set the template histogram once
                // Define associate label names for yield plots
                hYieldsTemplate->GetXaxis()->SetBinLabel(1+j, associateName.c_str());
            }
            // TODO: verbose
            // std::cout << std::endl;

            // Loop over DEPENDENCIES
            for (Int_t k=0; k<nDependencies; k++) {


                HistogramAndTriggerPtHistogramNames hDPhiAndhTrPtNames = vHistogramAndTriggerPtHistogramNames[k];
                // TODO: verbose
                // std::cout << "plotting histogram " << hDPhiAndhTrPtNames.hDPhi << " with trigger pT histogram " << hDPhiAndhTrPtNames.hTrPt << std::endl;

                vHists[i][k] = new TH1D(Form("hYields_%s_%i_%i_%i", FLAVOUR, i, j, k), Form("hYields_%s_%i_%i_%i", FLAVOUR, i, j, k), nAssociates, 0, nAssociates);
                vHists[i][k]->SetBinContent(1+j, vYieldsAndErrors.vYields[i][j][k]);
                if (CALCULATE_ERRORS) { 
                    vHists[i][k]->SetBinError(1+j, vYieldsAndErrors.vYieldsErrors[i][j][k]);
                }
                else {
                    vHists[i][k]->SetBinError(1+j, 1e-10);
                }
                cYields->cd();
                // TODO: do this with configuration.json and not hard-coded
                if (i==0) { vHists[0][k]->SetLineColor(kBlue); }
                if (i==1) { vHists[1][k]->SetLineColor(kRed);  }
                vHists[i][k]->Draw("same PE");

                // TODO: verbose
                // std::cout << std::endl;


            } // Loop over DEPENDENCIES


        } // Loop over ASSOCIATES


    } // Loop over TUNES


    return;


} // drawBalancingPlots()


void drawBalancingPlotsTUNERatios(CONFIGS configs_from_json, const char* FLAVOUR, YieldsAndErrors vYieldsAndErrors,
                                  Int_t indexNominatorTUNE, Int_t indexDenominatorTUNE) {


    std::cout << "*** Drawing balancing plots with TUNE ratios for " << FLAVOUR;


    // Retrieve settings from configuration.json
    canvasConfigs canvasConfigs = configs_from_json.canvasConfigs;
    bool CALCULATE_ERRORS = configs_from_json.CALCULATE_ERRORS;
    std::string base_dir = configs_from_json.base_dir;
    std::vector<std::string> vTUNES = configs_from_json.vTUNES; // TODO: put the name of the tune in output for clarity?
    std::vector<TriggerAssociateOSandSS> vTriggerAssociateOSandSS;
    if (strcmp(FLAVOUR, "BEAUTY") == 0) { vTriggerAssociateOSandSS = configs_from_json.vBeautyTriggerAssociateOSandSS; }
    if (strcmp(FLAVOUR, "CHARM") == 0)  { vTriggerAssociateOSandSS = configs_from_json.vCharmTriggerAssociateOSandSS; }
    std::vector<HistogramAndTriggerPtHistogramNames> vHistogramAndTriggerPtHistogramNames = configs_from_json.vHistogramAndTriggerPtHistogramNames;

    std::cout << " and TUNE " << vTUNES[indexNominatorTUNE] << "/" << vTUNES[indexDenominatorTUNE] << " ***" << std::endl;

    Int_t nAssociates = vTriggerAssociateOSandSS.size();
    Int_t nDependencies = vHistogramAndTriggerPtHistogramNames.size();

    // Values will be drawn from a 2D vector of TH1D with number of ASSOCIATES bins
    // This way the TUNE and DEPENDENCY can be looped over, while the data points will be the ASSOCIATES
    TH1D *vHists[nDependencies];
    // TODO: verbose
    // std::cout << "number of associates: " << nAssociates << std::endl;

    // Define a template for this plot to set titles, stats, etc.
    TH1D *hYieldsTemplate = new TH1D(Form("hYieldsTUNERatiosTemplate_%s", FLAVOUR), Form("hYieldsTUNERatiosTemplate_%s", FLAVOUR), nAssociates, 0, nAssociates);
    hYieldsTemplate->GetYaxis()->SetRangeUser(1e-1,1e3);

    TCanvas *cYields = new TCanvas(Form("cYieldsTUNERatios_%s", FLAVOUR), Form("cYieldsTUNERatios_%s", FLAVOUR), canvasConfigs.xSizeCanvas, canvasConfigs.ySizeCanvas);
    cYields->cd();
    gPad->SetLogy();
    hYieldsTemplate->SetStats(0);
    hYieldsTemplate->Draw("PE");

    // TODO: verbose
    // std::cout << "dividing " << vTUNES[indexNominatorTUNE] << "/" << vTUNES[indexDenominatorTUNE] << std::endl;
    // std::cout << std::endl;


    // Loop over ASSOCIATES
    for (Int_t j=0; j<nAssociates; j++) {


        // TODO: fix this bug with associateName and formatting....
        TriggerAssociateOSandSS fileNamesOSandSS = vTriggerAssociateOSandSS[j];
        std::string associateName = fileNamesOSandSS.associateOS;
        // TODO: verbose
        // std::cout << "starting loop over associate: " << associateName << std::endl;
        // std::cout << "starting loop over OS file: " << fileNamesOSandSS.OS << " and SS file: " << fileNamesOSandSS.SS << std::endl;
        // Define associate label names for yield plots
        hYieldsTemplate->GetXaxis()->SetBinLabel(1+j, associateName.c_str());
        // std::cout << std::endl;


        // Loop over DEPENDENCIES
        for (Int_t k=0; k<nDependencies; k++) {


            HistogramAndTriggerPtHistogramNames hDPhiAndhTrPtNames = vHistogramAndTriggerPtHistogramNames[k];
            // TODO: verbose
            // std::cout << "plotting histogram " << hDPhiAndhTrPtNames.hDPhi << " with trigger pT histogram " << hDPhiAndhTrPtNames.hTrPt << std::endl;

            vHists[k] = new TH1D(Form("hYields_%s_%i_%i", FLAVOUR, j, k), Form("hYields_%s_%i_%i", FLAVOUR, j, k), nAssociates, 0, nAssociates);
            vHists[k]->SetBinContent(1+j, vYieldsAndErrors.vYields[indexNominatorTUNE][j][k]/vYieldsAndErrors.vYieldsErrors[indexDenominatorTUNE][j][k]);
            if (CALCULATE_ERRORS) { 
                // TODO: implement ratio error in yield code
                // vHists[k]->SetBinError(1+j, vYieldsAndErrors.vYieldsErrors[i][j][k]);
                vHists[k]->SetBinError(1+j, propagateRatioError(vYieldsAndErrors.vYields[indexNominatorTUNE][j][k], 
                                                                vYieldsAndErrors.vYields[indexDenominatorTUNE][j][k],
                                                                vYieldsAndErrors.vYieldsErrors[indexNominatorTUNE][j][k],
                                                                vYieldsAndErrors.vYieldsErrors[indexDenominatorTUNE][j][k]));
            }
            else {
                vHists[k]->SetBinError(1+j, 1e-10);
            }
            cYields->cd();
            vHists[k]->SetLineColor(kBlack);
            vHists[k]->Draw("same PE");

            // TODO: verbose
            // std::cout << std::endl;


        } // Loop over DEPENDENCIES


    } // Loop over ASSOCIATES


    return;


} // drawBalancingPlotsTUNERatios()


void drawBalancingBaryonMesonRatioPlots(CONFIGS configs_from_json, const char* FLAVOUR, YieldsAndErrors vYieldsAndErrors) {


    std::cout << "*** Drawing balancing baryon/meson ratio plots for " << FLAVOUR << " ***" << std::endl;


    // Retrieve settings from configuration.json
    canvasConfigs canvasConfigs = configs_from_json.canvasConfigs;
    bool CALCULATE_ERRORS = configs_from_json.CALCULATE_ERRORS;
    std::string base_dir = configs_from_json.base_dir;
    std::vector<std::string> vTUNES = configs_from_json.vTUNES;
    std::vector<TriggerAssociateOSandSS> vTriggerAssociateOSandSS;
    if (strcmp(FLAVOUR, "BEAUTY") == 0) { vTriggerAssociateOSandSS = configs_from_json.vBeautyTriggerAssociateOSandSS; }
    if (strcmp(FLAVOUR, "CHARM") == 0)  { vTriggerAssociateOSandSS = configs_from_json.vCharmTriggerAssociateOSandSS; }
    std::vector<HistogramAndTriggerPtHistogramNames> vHistogramAndTriggerPtHistogramNames = configs_from_json.vHistogramAndTriggerPtHistogramNames;

    Int_t nTUNES = vTUNES.size();
    Int_t nAssociates = vTriggerAssociateOSandSS.size();
    Int_t nDependencies = vHistogramAndTriggerPtHistogramNames.size();

    // Values will be drawn from a 2D vector of TH1D with number of DEPENDENCIES bins
    // This way the TUNE and ASSOCIATE can be looped over, while the data points will be the DEPENDENCIES
    TH1D *vHists[nTUNES][nAssociates];
    // TODO: verbose
    // std::cout << "number of dependencies: " << nDependencies << std::endl;

    // Define a template for this plot to set titles, stats, etc.
    TH1D *hYieldsTemplate = new TH1D(Form("hYieldsBaryonMesonRatioTemplate_%s", FLAVOUR), Form("hYieldsBaryonMesonRatioTemplate_%s", FLAVOUR), nDependencies, 0, nDependencies);
    hYieldsTemplate->GetYaxis()->SetRangeUser(1e-4,0.2);

    TCanvas *cYields = new TCanvas(Form("cYieldsBaryonMesonRatio_%s", FLAVOUR), Form("cYieldsBaryonMesonRatio_%s", FLAVOUR), canvasConfigs.xSizeCanvas, canvasConfigs.ySizeCanvas);
    cYields->cd();
    hYieldsTemplate->SetStats(0);
    hYieldsTemplate->Draw("PE");


    // Loop over TUNES
    for (Int_t i=0; i<nTUNES; i++) {


        std::string TUNE = vTUNES[i];
        // TODO: verbose
        // std::cout << "starting loop over " << TUNE << std::endl;
        // std::cout << std::endl;


        // Loop over ASSOCIATES
        for (Int_t j=0; j<nAssociates; j++) {


            // TODO: fix this bug with associateName and formatting....
            TriggerAssociateOSandSS fileNamesOSandSS = vTriggerAssociateOSandSS[j];
            std::string associateName = fileNamesOSandSS.associateOS;
            // TODO: one can also define this in the configuration.json (if only interested in some, or more, or e.g. strange baryons)
            if (associateName != "Lambda_b" && associateName != "Sigma_b0" &&
                associateName != "Lambda_c(+)-bar" && associateName != "Sigma_c(+)-bar") { continue ;} // only for baryons
            // TODO: verbose
            // std::cout << "starting loop over associate: " << associateName << std::endl;
            // std::cout << "starting loop over OS file: " << fileNamesOSandSS.OS << " and SS file: " << fileNamesOSandSS.SS << std::endl;
            // std::cout << std::endl;

            // Loop over DEPENDENCIES
            for (Int_t k=0; k<nDependencies; k++) {


                HistogramAndTriggerPtHistogramNames hDPhiAndhTrPtNames = vHistogramAndTriggerPtHistogramNames[k];
                // TODO: verbose
                // std::cout << "plotting histogram " << hDPhiAndhTrPtNames.hDPhi << " with trigger pT histogram " << hDPhiAndhTrPtNames.hTrPt << std::endl;

                vHists[i][j] = new TH1D(Form("hYieldsBaryonMesonRatio_%s_%i_%i_%i", FLAVOUR, i, j, k), Form("hYieldsBaryonMesonRatio_%s_%i_%i_%i", FLAVOUR, i, j, k), nDependencies, 0, nDependencies);
                vHists[i][j]->SetBinContent(1+k, vYieldsAndErrors.vYields[i][j][k] / vYieldsAndErrors.vYields[i][0][k]);
                if (CALCULATE_ERRORS) { 
                    // Several options for error calculation/propagation
                    // Ratio calculated seperately:
                    vHists[i][j]->SetBinError(1+k, vYieldsAndErrors.vYieldsRatioErrors[i][j][k]);
                    // Naive error propagation (assuming no correlation):
                    /*
                    vHists[i][j]->SetBinError(1+k, propagateRatioError(vYieldsAndErrors.vYields[i][j][k], 
                                                                       vYieldsAndErrors.vYields[i][0][k],
                                                                       vYieldsAndErrors.vYieldsErrors[i][j][k],
                                                                       vYieldsAndErrors.vYieldsErrors[i][0][k]));
                    */
                    // Placeholder; same error as single yield:
                    // vHists[i][j]->SetBinError(1+k, vYieldsAndErrors.vYieldsErrors[i][j][k]);
                }
                else {
                    vHists[i][j]->SetBinError(1+k, 1e-10);
                }
                cYields->cd();
                // TODO: do this with configuration.json and not hard-coded
                if (i==0) { vHists[0][j]->SetLineColor(kBlue); }
                if (i==1) { vHists[1][j]->SetLineColor(kRed);  }
                vHists[i][j]->Draw("same PE");

                hYieldsTemplate->GetXaxis()->SetBinLabel(1+k, (hDPhiAndhTrPtNames.hDPhi).c_str());

                // TODO: verbose
                // std::cout << std::endl;


            } // Loop over DEPENDENCIES


        } // Loop over ASSOCIATES


    } // Loop over TUNES


    return;


} // drawBalancingBaryonMesonRatioPlots()


void drawBalancingBaryonMesonRatioPlotsTUNERatios(CONFIGS configs_from_json, const char* FLAVOUR, YieldsAndErrors vYieldsAndErrors,
                                                  Int_t indexNominatorTUNE, Int_t indexDenominatorTUNE) {


    std::cout << "*** Drawing balancing baryon/meson ratio plots with TUNE ratios for " << FLAVOUR << " ***" << std::endl;


    // Retrieve settings from configuration.json
    canvasConfigs canvasConfigs = configs_from_json.canvasConfigs;
    bool CALCULATE_ERRORS = configs_from_json.CALCULATE_ERRORS;
    std::string base_dir = configs_from_json.base_dir;
    std::vector<std::string> vTUNES = configs_from_json.vTUNES;
    std::vector<TriggerAssociateOSandSS> vTriggerAssociateOSandSS;
    if (strcmp(FLAVOUR, "BEAUTY") == 0) { vTriggerAssociateOSandSS = configs_from_json.vBeautyTriggerAssociateOSandSS; }
    if (strcmp(FLAVOUR, "CHARM") == 0)  { vTriggerAssociateOSandSS = configs_from_json.vCharmTriggerAssociateOSandSS; }
    std::vector<HistogramAndTriggerPtHistogramNames> vHistogramAndTriggerPtHistogramNames = configs_from_json.vHistogramAndTriggerPtHistogramNames;

    std::cout << " and TUNE " << vTUNES[indexNominatorTUNE] << "/" << vTUNES[indexDenominatorTUNE] << " ***" << std::endl;

    Int_t nAssociates = vTriggerAssociateOSandSS.size();
    Int_t nDependencies = vHistogramAndTriggerPtHistogramNames.size();

    // Values will be drawn from a 2D vector of TH1D with number of DEPENDENCIES bins
    // This way the TUNE and ASSOCIATE can be looped over, while the data points will be the DEPENDENCIES
    TH1D *vHists[nAssociates];
    // TODO: verbose
    // std::cout << "number of dependencies: " << nDependencies << std::endl;

    // Define a template for this plot to set titles, stats, etc.
    TH1D *hYieldsTemplate = new TH1D(Form("hYieldsBaryonMesonRatioTUNERatioTemplate_%s", FLAVOUR), Form("hYieldsBaryonMesonRatioTUNERatioTemplate_%s", FLAVOUR), nDependencies, 0, nDependencies);
    hYieldsTemplate->GetYaxis()->SetRangeUser(1e-1,1e1);

    TCanvas *cYields = new TCanvas(Form("cYieldsBaryonMesonRatioTUNERatio_%s", FLAVOUR), Form("cYieldsBaryonMesonRatioTUNERatio_%s", FLAVOUR), canvasConfigs.xSizeCanvas, canvasConfigs.ySizeCanvas);
    cYields->cd();
    hYieldsTemplate->SetStats(0);
    hYieldsTemplate->Draw("PE");

    // TODO: verbose
    // std::cout << "starting loop over " << TUNE << std::endl;
    // std::cout << std::endl;


    // Loop over ASSOCIATES
    for (Int_t j=0; j<nAssociates; j++) {


        // TODO: fix this bug with associateName and formatting....
        TriggerAssociateOSandSS fileNamesOSandSS = vTriggerAssociateOSandSS[j];
        std::string associateName = fileNamesOSandSS.associateOS;
        // TODO: one can also define this in the configuration.json (if only interested in some, or more, or e.g. strange baryons)
        if (associateName != "Lambda_b" && associateName != "Sigma_b0" &&
            associateName != "Lambda_c(+)-bar" && associateName != "Sigma_c(+)-bar") { continue ;} // only for baryons
        // TODO: verbose
        // std::cout << "starting loop over associate: " << associateName << std::endl;
        // std::cout << "starting loop over OS file: " << fileNamesOSandSS.OS << " and SS file: " << fileNamesOSandSS.SS << std::endl;
        // std::cout << std::endl;

        // Loop over DEPENDENCIES
        for (Int_t k=0; k<nDependencies; k++) {


            HistogramAndTriggerPtHistogramNames hDPhiAndhTrPtNames = vHistogramAndTriggerPtHistogramNames[k];
            // TODO: verbose
            // std::cout << "plotting histogram " << hDPhiAndhTrPtNames.hDPhi << " with trigger pT histogram " << hDPhiAndhTrPtNames.hTrPt << std::endl;

            vHists[j] = new TH1D(Form("hYieldsBaryonMesonRatio_%s_%i_%i", FLAVOUR, j, k), Form("hYieldsBaryonMesonRatio_%s_%i_%i", FLAVOUR, j, k), nDependencies, 0, nDependencies);
            vHists[j]->SetBinContent(1+k, (vYieldsAndErrors.vYields[indexNominatorTUNE][j][k] / vYieldsAndErrors.vYields[indexNominatorTUNE][0][k])
                                           / (vYieldsAndErrors.vYields[indexDenominatorTUNE][j][k] / vYieldsAndErrors.vYields[indexDenominatorTUNE][0][k]));
            if (CALCULATE_ERRORS) { 
                // Several options for error calculation/propagation
                // Ratio calculated seperately:
                // TODO: add this in yield calculation
                // TODO: also check if this is without bugs (features?) now..
                // vHists[j]->SetBinError(1+k, vYieldsAndErrors.vYieldsRatioErrors[i][j][k]);
                vHists[j]->SetBinError(1+k, propagateRatioError(vYieldsAndErrors.vYields[indexNominatorTUNE][j][k]/vYieldsAndErrors.vYields[indexDenominatorTUNE][j][k], 
                                                                vYieldsAndErrors.vYields[indexNominatorTUNE][0][k]/vYieldsAndErrors.vYields[indexDenominatorTUNE][0][k],
                                                                vYieldsAndErrors.vYieldsRatioErrors[indexNominatorTUNE][j][k],
                                                                vYieldsAndErrors.vYieldsRatioErrors[indexDenominatorTUNE][0][k]));
                // Naive error propagation (assuming no correlation):
                /*
                vHists[j]->SetBinError(1+k, propagateRatioError(vYieldsAndErrors.vYields[i][j][k], 
                                                                    vYieldsAndErrors.vYields[i][0][k],
                                                                    vYieldsAndErrors.vYieldsErrors[i][j][k],
                                                                    vYieldsAndErrors.vYieldsErrors[i][0][k]));
                */
                // Placeholder; same error as single yield:
                // vHists[j]->SetBinError(1+k, vYieldsAndErrors.vYieldsErrors[i][j][k]);
            }
            else {
                vHists[j]->SetBinError(1+k, 1e-10);
            }
            cYields->cd();
            vHists[j]->SetLineColor(kBlack);
            vHists[j]->Draw("same PE");

            hYieldsTemplate->GetXaxis()->SetBinLabel(1+k, (hDPhiAndhTrPtNames.hDPhi).c_str());

            // TODO: verbose
            // std::cout << std::endl;


        } // Loop over DEPENDENCIES


    } // Loop over ASSOCIATES


    return;


} // drawBalancingBaryonMesonRatioPlotsTUNERatios()


// Run macro with 
// >> root 'improvedPlotting.C("configuration_multiplicity.json")'
int improvedPlotting(const char* configuration) {

    // Read configurations defined by user in configuration.json
    CONFIGS configs_from_json = readConfig(configuration);

    // Calculate the 3D yield vector
    YieldsAndErrors vYieldsBeauty;
    YieldsAndErrors vYieldsCharm;
    vYieldsBeauty = calculateYieldsVector(configs_from_json,"BEAUTY");
    vYieldsCharm =  calculateYieldsVector(configs_from_json,"CHARM");

    // Draw the balancing plots using the 3D yield vector and configurations given
    std::vector<canvasConfigs> vCanvasConfigs = configs_from_json.vCanvasConfigs;
    std::vector<TCanvas*> vCanvases;
    YieldsAndErrors vYields; // TODO: define variables in loop or above?
    for (const auto& canvasConfigs : vCanvasConfigs) {
        configs_from_json.canvasConfigs = canvasConfigs;
        std::string canvasName = canvasConfigs.canvasName;
        std::string drawFunctionToUse = canvasConfigs.drawFunctionToUse;
        std::vector<std::string> vTUNES = canvasConfigs.vTUNES;
        std::string FLAVOUR = canvasConfigs.FLAVOUR;
        Int_t indexNominatorTUNE = canvasConfigs.indexNominatorTUNE;
        Int_t indexDenominatorTUNE = canvasConfigs.indexDenominatorTUNE;
        // TODO: alternatively, just don't define vYieldsBeauty above, 
        // it's a bit redundant now..
        if (strcmp(FLAVOUR.c_str(), "BEAUTY") == 0)  { vYields = vYieldsBeauty; }
        if (strcmp(FLAVOUR.c_str(), "CHARM" ) == 0)  { vYields = vYieldsCharm;  }

        if (strcmp(drawFunctionToUse.c_str(), "drawBalancingPlots") == 0)  { 
            drawBalancingPlots(configs_from_json,FLAVOUR.c_str(),vYields); 
        }
        if (strcmp(drawFunctionToUse.c_str(), "drawBalancingPlotsTUNERatios") == 0)  { 
            drawBalancingPlotsTUNERatios(configs_from_json,FLAVOUR.c_str(),vYields,indexNominatorTUNE,indexDenominatorTUNE); 
        }
        if (strcmp(drawFunctionToUse.c_str(), "drawBalancingBaryonMesonRatioPlots") == 0)  { 
            drawBalancingBaryonMesonRatioPlots(configs_from_json,FLAVOUR.c_str(),vYields); 
        }
        if (strcmp(drawFunctionToUse.c_str(), "drawBalancingBaryonMesonRatioPlotsTUNERatios") == 0)  { 
            drawBalancingBaryonMesonRatioPlotsTUNERatios(configs_from_json,FLAVOUR.c_str(),vYields,indexNominatorTUNE,indexDenominatorTUNE); 
        }
        // TODO: add other configurations for drawing (e.g. xMax, xMin, etc.)
    }

    // TODO: add the updated simulations to the RootFiles (including subSamples)

    // TODO: in existing functions, add the TUNE[i]/TUNE[j] subratio plots
    // (including error propagation)

    // TODO: add the part for miniPads

    // TODO: only for simple tests, remove
    /*
    drawBalancingPlots(configs_from_json,"BEAUTY",vYieldsBeauty);
    drawBalancingPlots(configs_from_json,"CHARM", vYieldsCharm);

    drawBalancingBaryonMesonRatioPlots(configs_from_json,"BEAUTY",vYieldsBeauty);
    drawBalancingBaryonMesonRatioPlots(configs_from_json,"CHARM", vYieldsCharm);
    */

    return 0;
}
