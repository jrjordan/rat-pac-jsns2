#include <iostream>
#include <sstream>
#include <fstream>

#include "TFile.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TLegend.h"
#include "TEllipse.h"
#include "TGraph.h"
#include "TRandom3.h"
#include "TF1.h"

#include "RAT/DSReader.hh"
#include "RAT/DS/MC.hh"
#include "RAT/DS/Root.hh"
#include "RAT/DS/PMT.hh"
#include "RAT/DS/EV.hh"
#include "RAT/DS/Run.hh"
#include "RAT/DS/RunStore.hh"

int main( int nargs, char** argv ) {

    TRandom3 rand(0);

    if ( nargs<3 ) { //4
        std::cout << "usage: analyze_data <input RAT root file> <output rootfile> <verbose flag (0 or 1)>" << std::endl;
        return 1;
    }

    std::string inputfile = argv[1];
    std::string outfile = argv[2];
    std::string verboseString = argv[3];
    bool verbose = verboseString != "0";

    RAT::DSReader* ds = new RAT::DSReader( inputfile.c_str() ); 

    TFile* out = new TFile(outfile.c_str(), "RECREATE" );

    //Neutron capture times
    TH1D* captureTimeHist = new TH1D("captureTimeHist", "Capture Times", 100, 0.0, 500.0);
    
    //Total energy of the capture gammas
    TH1D* gammaEnergyHist = new TH1D("gammaEnergyHist", "Gamma Energy", 100, 0.0, 10.0);

    int ievent = 0;
    int nevents = ds->GetTotal();

    std::cout << "Number of events: " << nevents << std::endl;

    int numCaptures = 0;

    while (ievent < nevents) {
        RAT::DS::Root* root = ds->NextEvent();
    
        RAT::DS::MC* mc = root->GetMC();
        if ( mc==NULL ) {
            std::cout << "mc==NULL" << std::endl;
            break;
        }

        if ( mc->GetMCParticleCount()==0 ) {
            std::cout << "ParticleCount==0" << std::endl;
            ievent++;
            continue;
        }

        //Look for neutron captures
        int numTracks = mc->GetMCTrackCount();
        bool didCapture = false;
        double captureTime = 0.0;
        double gammaEnergy = 0.0;

        for (int i = 0; i < numTracks; ++i) {
            RAT::DS::MCTrack* track = mc->GetMCTrack(i);
            std::string partName = track->GetParticleName();
            
            //If the particle is a neutron, check if it captures
            if (partName == "neutron") {
                int numTrackSteps = track->GetMCTrackStepCount();
                for (int j = 0; j < numTrackSteps; ++j) {
                    RAT::DS::MCTrackStep* trackStep = track->GetMCTrackStep(j);
                    //Check what process killed this step
                    std::string processName = trackStep->GetProcess();
                    if (processName == "nCapture") {
                        numCaptures++;
                        captureTime = trackStep->GetGlobalTime();
                        didCapture = true;
                    }
        
                }
            }

            //Look for daughter gammas from the neutron capture
            else if (partName == "gamma" && track->GetParentID() == 1) {
                gammaEnergy += track->GetMCTrackStep(0)->GetKE();
            } 
        }
        captureTimeHist->Fill(captureTime/1000.0);  //convert to microseconds
        gammaEnergyHist->Fill(gammaEnergy);

        if (verbose && didCapture) { 
            std::cout << "------------------------------------------" << std::endl;
            std::cout << "EVENT " << ievent << std::endl;
            std::cout << " Neutron capture found!" << std::endl;
            std::cout << " Total gamma energy: " << gammaEnergy << " MeV." << std::endl;
        }
        ievent++; 
    
    } //end of while loop

    std::cout << numCaptures << " neutron captures found." << std::endl;
    std::cout << "Writing." << std::endl;
  
    captureTimeHist->Write();
    gammaEnergyHist->Write();

    out->cd();

    std::cout << "Finished." << std::endl;

    return 0;
}
