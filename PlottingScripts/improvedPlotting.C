// improvedPlotting.C
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <random>

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

    // Generic options
    bool CALCULATE_ERRORS = config["calculate_errors"].get<bool>();
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
    configs_from_json.CALCULATE_ERRORS = CALCULATE_ERRORS;
    configs_from_json.nSubSamples = nSubSamples;
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
    // std::cout << "hCorr->Integral() = " << hCorr->Integral() << std::endl;

    // TODO: add option in configuration.json to show angular correlation spectra
    // User can define OS, SS, for which associate, which dependency, whatever...
    /*
    TCanvas *chDPhiOS = new TCanvas(Form("chDPhiOS_%s_%i%i%i_%i",FLAVOUR,i,j,k,l),Form("chDPhiOS_%s_%i%i%i_%i",FLAVOUR,i,j,k,l),600,800);
    chDPhiOS->cd();
    hDPhiOS->Draw("hist");
    TCanvas *chDPhiSS = new TCanvas(Form("chDPhiSS_%s_%i%i%i_%i",FLAVOUR,i,j,k,l),Form("chDPhiSS_%s_%i%i%i_%i",FLAVOUR_i,j,k,l),600,800);
    chDPhiSS->cd();
    hDPhiSS->Draw("hist");
    TCanvas *cCorr = new TCanvas(Form("cCorr_%s_%i%i%i_%i",FLAVOUR,i,j,k,l),Form("cCorr_%s_%i%i%i_%i",FLAVOUR,i,j,k,l),600,800);
    cCorr->cd();
    hCorr->Draw("hist");
    */

    return hCorr->Integral();
} // calculateOneYield()


// Function to create a subsample histogram with the same binning as the original
/*
TH1D* createSubSampleHistogram(TH1D *originalHist, Int_t subSampleIndex, Int_t nSubSamples,
                               std::string FLAVOUR, std::string TYPE, Int_t i, Int_t j, Int_t k) {
                                
    // Create a new histogram for the subsample with the same binning
    TH1D *subHist = new TH1D(
        Form("subHist_%s_%s_%s_%i%i%i_%i", FLAVOUR.c_str(), originalHist->GetName(), TYPE.c_str(), i, j, k, subSampleIndex),
        Form("SubSample %i of %s", subSampleIndex, originalHist->GetTitle()),
        originalHist->GetNbinsX(), originalHist->GetXaxis()->GetXmin(), originalHist->GetXaxis()->GetXmax()
    );

    // Get the total number of entries in the original histogram
    int totalEntries = originalHist->GetEntries();
    int subSampleSize = totalEntries / nSubSamples;

    // Randomly select entries for the subsample
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> binDis(1, originalHist->GetNbinsX());  // Distribute over bins

    // Fill the subsample histogram with random entries
    for (Int_t sample = 0; sample < subSampleSize; ++sample) {
        int randomBin = binDis(gen);  // Select a random bin
        Double_t binContent = originalHist->GetBinContent(randomBin);  // Get bin content
        
        // Fill the subsample with random values within that bin
        if (binContent > 0) {
            std::uniform_real_distribution<> valueDis(originalHist->GetBinLowEdge(randomBin),
                                                      originalHist->GetBinLowEdge(randomBin + 1));
            Double_t value = valueDis(gen);
            subHist->Fill(value);  // Add the value to the subsample histogram
        }
    }

    std::cout << "Subsample " << subSampleIndex << " Mean: " << subHist->GetMean() 
              << ", Variance: " << subHist->GetStdDev() << std::endl;
    

    // std::cout << "Mean: " << subHist->GetMean() << ", Variance: " << subHist->GetStdDev() << std::endl;
    
    return subHist;
} // createSubSampleHistogram()
*/

// Yields are calculated by looping over TUNES (e.g. MONASH),
// then ASSOCIATES (e.g. B-)
// and then DEPENDENCIES (e.g. DPhiLL)
// Function needs to be called for the FLAVOUR seperately (e.g. Beauty)
// The output is a 3D vector with the structure
// v[TUNE][ASSOCIATE][DEPENDENCY]
// TODO: change the .size() to variables nTUNES, etc. Like in the plotting function below
YieldsAndErrors calculateYieldsVector(CONFIGS configs_from_json, const char* FLAVOUR) {

    std::cout << "*** Calculating yields for " << FLAVOUR << " ***" << std::endl;

    std::vector<std::vector<std::vector<Double_t>>> vYields;
    std::vector<std::vector<std::vector<Double_t>>> vYieldsErrors;

    // Retrieve settings from configuration.json
    bool CALCULATE_ERRORS = configs_from_json.CALCULATE_ERRORS;
    int nSubSamples = configs_from_json.nSubSamples;
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

                TH1D *hDPhiOS = (TH1D*)OStree->Get((hDPhiAndhTrPtNames.hDPhi).c_str());
	            TH1D *hDPhiSS = (TH1D*)SStree->Get((hDPhiAndhTrPtNames.hDPhi).c_str());
	            TH1D *hTrPtOS = (TH1D*)OStree->Get((hDPhiAndhTrPtNames.hTrPt).c_str());
	            TH1D *hTrPtSS = (TH1D*)SStree->Get((hDPhiAndhTrPtNames.hTrPt).c_str());

                // Prevent double-counting
	            if (strcmp((fileNamesOSandSS.trigger).c_str(), 
                           (fileNamesOSandSS.associateSS).c_str()) == 0) { 
                    hDPhiSS->Scale(0.5); } 

                // Calculate the error on the yield by subsampling with N samples
                // Not the most efficient way, but it is straightforward and clear
                // and anyways the files are quite small so it doesn't take too long
                if (CALCULATE_ERRORS) {
                    /*
                    TH1D *hSubYields = new TH1D(Form("hSubYields_%i%i%i", i, j, k), Form("hSubYields_%i%i%i", i, j, k), 50, 3.32, 3.39);
                    for (Int_t l = 0; l < nSubSamples; l++) {
                        // Most of the indices are there to create unique histograms
                        // They don't actually provide interesting information besides debugging
                        // Don't stare too long at them
                        TH1D *subHDPhiOS = createSubSampleHistogram(hDPhiOS, l, nSubSamples, FLAVOUR, "OS", i, j, k);
                        TH1D *subHDPhiSS = createSubSampleHistogram(hDPhiSS, l, nSubSamples, FLAVOUR, "SS", i, j, k);
                        TH1D *subHTrPtOS = createSubSampleHistogram(hTrPtOS, l, nSubSamples, FLAVOUR, "OS", i, j, k);
                        TH1D *subHTrPtSS = createSubSampleHistogram(hTrPtSS, l, nSubSamples, FLAVOUR, "SS", i, j, k);
                        Double_t subYield = calculateOneYield(subHDPhiOS, subHTrPtOS, subHDPhiSS, subHTrPtSS, FLAVOUR, i, j, k, l);
                        std::cout << "vYields[" << i << "][" << j << "][" << k << "][" << l << "] = " << subYield << std::endl;
                        std::cout << std::endl;

                        delete subHDPhiOS;
                        delete subHTrPtOS;
                        delete subHDPhiSS;
                        delete subHTrPtSS;

                        // Calculate standard deviation by assuming Gaussian
                        hSubYields->Fill(subYield);

                        TCanvas *cTestHDPhiOS;
                        TCanvas *cTestSubYields;
                        if (i==0 && j==0 && k==0 && l==0) {
                            cTestHDPhiOS = new TCanvas("testHDPhiOS","testHDPhiOS",600,800);
                            cTestHDPhiOS->cd();
                            subHDPhiOS->Draw("hist");
                        }
                        if (i==0 && j==0 && k==0 && l==1) {
                            cTestHDPhiOS->cd();
                            subHDPhiOS->Draw("same hist");
                        }
                        if (i==0 && j==0 && k==0 && l==0) {
                            cTestSubYields = new TCanvas("testSubYields","testSubYields",600,800);
                            cTestSubYields->cd();
                            hSubYields->Draw("hist");
                        }
                    }
                    Double_t yieldError = hSubYields->GetStdDev();
                    if (i >= vYieldsErrors.size()) { vYieldsErrors.resize(i + 1); }
                    if (j >= vYieldsErrors[i].size()) { vYieldsErrors[i].resize(j + 1); }
                    if (k >= vYieldsErrors[i][j].size()) { vYieldsErrors[i][j].resize(k + 1); }
                    vYieldsErrors[i][j][k] = yieldError; 
                    std::cout << "vYieldsErrors[" << i << "][" << j << "][" << k << "] = " << vYieldsErrors[i][j][k] << std::endl;
                    std::cout << std::endl;
                */
                }

                // Calculate yield value and assign to appropriate place in vector
                Double_t yield = calculateOneYield(hDPhiOS, hTrPtOS, hDPhiSS, hTrPtSS, FLAVOUR, i, j, k, 0);
                if (i >= vYields.size()) { vYields.resize(i + 1); }
                if (j >= vYields[i].size()) { vYields[i].resize(j + 1); }
                if (k >= vYields[i][j].size()) { vYields[i][j].resize(k + 1); }
                vYields[i][j][k] = yield; 
                std::cout << "vYields[" << i << "][" << j << "][" << k << "] = " << vYields[i][j][k] << std::endl;
                std::cout << std::endl;


            } // Loop over DEPENDENCIES


        } // Loop over ASSOCIATES


    } // Loop over TUNES


    YieldsAndErrors vYieldsAndErrors;
    vYieldsAndErrors.vYields = vYields;
    if (CALCULATE_ERRORS) { vYieldsAndErrors.vYieldsErrors = vYieldsErrors; }
    return vYieldsAndErrors;


} // calculateYieldsVector()


void drawBalancingPlots(CONFIGS configs_from_json, const char* FLAVOUR, YieldsAndErrors vYields) {


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
    hYieldsTemplate->GetYaxis()->SetRangeUser(1e-4,0.4);

    TCanvas *cYields = new TCanvas(Form("cYields_%s", FLAVOUR), Form("cYields_%s", FLAVOUR), 800, 600);
    cYields->cd();
    gPad->SetLogy();
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
                vHists[i][k]->SetBinContent(1+j, vYields.vYields[i][j][k]);
                if (CALCULATE_ERRORS) { 
                    vHists[i][k]->SetBinError(1+j, vYields.vYieldsErrors[i][j][k]);
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


void drawBalancingBaryonMesonRatioPlots(CONFIGS configs_from_json, const char* FLAVOUR, YieldsAndErrors vYields) {


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
    hYieldsTemplate->GetYaxis()->SetRangeUser(1e-4,0.4);

    TCanvas *cYields = new TCanvas(Form("cYieldsBaryonMesonRatio_%s", FLAVOUR), Form("cYieldsBaryonMesonRatio_%s", FLAVOUR), 800, 600);
    cYields->cd();
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
            if (associateName != "Lambda_b" && associateName != "Sigma_b0") { continue ;} // only for baryons
            std::cout << "starting loop over associate: " << associateName << std::endl;
            std::cout << "starting loop over OS file: " << fileNamesOSandSS.OS << " and SS file: " << fileNamesOSandSS.SS << std::endl;
            std::cout << std::endl;

            // Loop over DEPENDENCIES
            for (Int_t k=0; k<nDependencies; k++) {


                HistogramAndTriggerPtHistogramNames hDPhiAndhTrPtNames = vHistogramAndTriggerPtHistogramNames[k];
                std::cout << "plotting histogram " << hDPhiAndhTrPtNames.hDPhi << " with trigger pT histogram " << hDPhiAndhTrPtNames.hTrPt << std::endl;

                vHists[i][j] = new TH1D(Form("hYieldsBaryonMesonRatio_%s_%i_%i_%i", FLAVOUR, i, j, k), Form("hYieldsBaryonMesonRatio_%s_%i_%i_%i", FLAVOUR, i, j, k), nDependencies, 0, nDependencies);
                vHists[i][j]->SetBinContent(1+k, vYields.vYields[i][j][k] / vYields.vYields[i][0][k]);
                if (CALCULATE_ERRORS) { 
                    // TODO: ratio errors as seperate entry in vYieldsBaryonMesonRatioErrors...
                    vHists[i][j]->SetBinError(1+k, vYields.vYieldsErrors[i][j][k]);
                }
                else {
                    vHists[i][j]->SetBinError(1+k, 1e-10);
                }
                cYields->cd();
                // TODO: do this with configuration.json and not hard-coded
                if (i==0) { vHists[0][j]->SetLineColor(kBlue); }
                if (i==1) { vHists[1][j]->SetLineColor(kRed);  }
                vHists[i][j]->Draw("same PE");

                if (i==0 && j==0) { // only set the template histogram once
                // Define associate label names for yield plots
                // TODO: ask this as input vector in configuration.json
                hYieldsTemplate->GetXaxis()->SetBinLabel(1+k, (hDPhiAndhTrPtNames.hDPhi).c_str());
            }

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
    vYieldsBeauty = calculateYieldsVector(configs_from_json,"BEAUTY");
    // vYieldsCharm =  calculateYieldsVector(configs_from_json,"CHARM");

    // Draw the balancing plots using the 3D yield vector
    drawBalancingPlots(configs_from_json,"BEAUTY",vYieldsBeauty);
    // drawBalancingPlots(configs_from_json,"CHARM", vYieldsCharm);

    // Draw the balancing baryon/meson ratio plots
    drawBalancingBaryonMesonRatioPlots(configs_from_json,"BEAUTY",vYieldsBeauty);
    // drawBalancingBaryonMesonRatioPlots(configs_from_json,"CHARM", vYieldsCharm);

    return 0;
}