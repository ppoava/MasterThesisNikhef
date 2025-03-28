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

// TODO: put structs in a header file

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
};

CONFIGS readConfig() {

    std::cout << "Reading configuration.json" << std::endl;
    std::cout << std::endl;

    // Open the JSON configuration file
    // TODO: make this more general
    std::ifstream configFile("configuration_multiplicity.json");
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

    std::cout << "OKAY (or not)" << std::endl;
    std::cout << std::endl;

    // TODO: give short summary of settings given and what the output yield vector will look like

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

    return configs_from_json;

} // readConfig()


// Simple function that returns the propagated error from a ratio A/B with errors for A and B
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
    std::cout << "number of associates: " << nAssociates << std::endl;

    // Define a template for this plot to set titles, stats, etc.
    TH1D *hYieldsTemplate = new TH1D(Form("hYieldsTemplate_%s", FLAVOUR), "hYieldsTemplate", nAssociates, 0, nAssociates);
    hYieldsTemplate->GetYaxis()->SetRangeUser(1e-4,0.8);

    TCanvas *cYields = new TCanvas(Form("cYields_%s", FLAVOUR), Form("cYields_%s", FLAVOUR), 800, 600);
    cYields->cd();
    gPad->SetLogy();
    hYieldsTemplate->SetStats(0);
    hYieldsTemplate->Draw("PE");


    // Loop over TUNES
    for (Int_t i=0; i<nTUNES; i++) {


        std::string TUNE = vTUNES[i];
        std::cout << "starting loop over " << TUNE << std::endl;
        std::cout << std::endl;


        // Loop over ASSOCIATES
        for (Int_t j=0; j<nAssociates; j++) {


            // TODO: fix this bug with associateName and formatting....
            TriggerAssociateOSandSS fileNamesOSandSS = vTriggerAssociateOSandSS[j];
            std::string associateName = fileNamesOSandSS.associateOS;
            std::cout << "starting loop over associate: " << associateName << std::endl;
            std::cout << "starting loop over OS file: " << fileNamesOSandSS.OS << " and SS file: " << fileNamesOSandSS.SS << std::endl;

            if (i==0) { // only set the template histogram once
                // Define associate label names for yield plots
                hYieldsTemplate->GetXaxis()->SetBinLabel(1+j, associateName.c_str());
            }


            std::cout << std::endl;

            // Loop over DEPENDENCIES
            for (Int_t k=0; k<nDependencies; k++) {


                HistogramAndTriggerPtHistogramNames hDPhiAndhTrPtNames = vHistogramAndTriggerPtHistogramNames[k];
                std::cout << "plotting histogram " << hDPhiAndhTrPtNames.hDPhi << " with trigger pT histogram " << hDPhiAndhTrPtNames.hTrPt << std::endl;

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

                std::cout << std::endl;


            } // Loop over DEPENDENCIES


        } // Loop over ASSOCIATES


    } // Loop over TUNES


    return;


} // drawBalancingPlots()


void drawBalancingBaryonMesonRatioPlots(CONFIGS configs_from_json, const char* FLAVOUR, YieldsAndErrors vYieldsAndErrors) {


    std::cout << "*** Drawing balancing baryon/meson ratio plots for " << FLAVOUR << " ***" << std::endl;


    // Retrieve settings from configuration.json
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
    std::cout << "number of dependencies: " << nDependencies << std::endl;

    // Define a template for this plot to set titles, stats, etc.
    TH1D *hYieldsTemplate = new TH1D(Form("hYieldsBaryonMesonRatioTemplate_%s", FLAVOUR), "hYieldsBaryonMesonRatioTemplate", nDependencies, 0, nDependencies);
    hYieldsTemplate->GetYaxis()->SetRangeUser(1e-4,0.2);

    TCanvas *cYields = new TCanvas(Form("cYieldsBaryonMesonRatio_%s", FLAVOUR), Form("cYieldsBaryonMesonRatio_%s", FLAVOUR), 800, 600);
    cYields->cd();
    hYieldsTemplate->SetStats(0);
    hYieldsTemplate->Draw("PE");


    // Loop over TUNES
    for (Int_t i=0; i<nTUNES; i++) {


        std::string TUNE = vTUNES[i];
        std::cout << "starting loop over " << TUNE << std::endl;
        std::cout << std::endl;


        // Loop over ASSOCIATES
        for (Int_t j=0; j<nAssociates; j++) {


            // TODO: fix this bug with associateName and formatting....
            TriggerAssociateOSandSS fileNamesOSandSS = vTriggerAssociateOSandSS[j];
            std::string associateName = fileNamesOSandSS.associateOS;
            // TODO: one can also define this in the configuration.json (if only interested in some, or more, or e.g. strange baryons)
            if (associateName != "Lambda_b" && associateName != "Sigma_b0" &&
                associateName != "Lambda_c(+)-bar" && associateName != "Sigma_c(+)-bar") { continue ;} // only for baryons
            std::cout << "starting loop over associate: " << associateName << std::endl;
            std::cout << "starting loop over OS file: " << fileNamesOSandSS.OS << " and SS file: " << fileNamesOSandSS.SS << std::endl;
            std::cout << std::endl;

            // Loop over DEPENDENCIES
            for (Int_t k=0; k<nDependencies; k++) {


                HistogramAndTriggerPtHistogramNames hDPhiAndhTrPtNames = vHistogramAndTriggerPtHistogramNames[k];
                std::cout << "plotting histogram " << hDPhiAndhTrPtNames.hDPhi << " with trigger pT histogram " << hDPhiAndhTrPtNames.hTrPt << std::endl;

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

                std::cout << std::endl;


            } // Loop over DEPENDENCIES


        } // Loop over ASSOCIATES


    } // Loop over TUNES


    return;


} // drawBalancingBaryonMesonRatioPlots()


int improvedPlotting() {

    // Read configurations defined by user in configuration.json
    CONFIGS configs_from_json = readConfig();

    // Calculate the 3D yield vector
    YieldsAndErrors vYieldsBeauty;
    YieldsAndErrors vYieldsCharm;
    // vYieldsBeauty = calculateYieldsVector(configs_from_json,"BEAUTY");
    vYieldsCharm =  calculateYieldsVector(configs_from_json,"CHARM");

    // Draw the balancing plots using the 3D yield vector
    // drawBalancingPlots(configs_from_json,"BEAUTY",vYieldsBeauty);
    drawBalancingPlots(configs_from_json,"CHARM", vYieldsCharm);

    // Draw the balancing baryon/meson ratio plots
    // drawBalancingBaryonMesonRatioPlots(configs_from_json,"BEAUTY",vYieldsBeauty);
    drawBalancingBaryonMesonRatioPlots(configs_from_json,"CHARM", vYieldsCharm);

    return 0;
}