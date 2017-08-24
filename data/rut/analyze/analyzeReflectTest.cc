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

    //Uncorrected hit times
    TH1D* totalHitTimeHist = new TH1D("totalHitTimeHist", "Hit Times", 500, 0.0, 5.0);

    //Hit map on the surface of the PMT
    TH2D* PMTHitMap = new TH2D("PMTHitMap", "", 100, -10, 10, 100, -10, 10);
    
    //Uncorrected hit times
    TH1D* wavelengthHist = new TH1D("wavelengthHist", "Wavelength", 400, 400, 800);

    //Variables we want
    int numPE;
    int numPMT;

    int ievent = 0;
    int nevents = ds->GetTotal();

    //Keep track of the number of photons total detected over all events
    int numDetected = 0;

    std::cout << "Number of events: " << nevents << std::endl;

    while (ievent < nevents) {
        int numBadTiming = 0;
        RAT::DS::Root* root = ds->NextEvent();
    
        numPE = 0;
        numPMT = 0;

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

        numPE = mc->GetNumPE();
        numPMT = mc->GetMCPMTCount();

        //Count incident photons
        for ( int iPMT = 0; iPMT < numPMT; iPMT++ ) {
            RAT::DS::MCPMT* PMT = mc->GetMCPMT( iPMT );
            int numHits = PMT->GetMCPhotonCount();

            for (int iHit = 0; iHit < numHits; iHit++) {
	            RAT::DS::MCPhoton* hit = PMT->GetMCPhoton( iHit );
                numDetected += 1;
                
                //Should not be any dark hits. Check just in case
                if ( hit->IsDarkHit() ) {
                    std::cout << "We've got a dark hit!" << std::endl;
                    continue;
                }
	
	            double tHit = hit->GetHitTime();
                double wavelength = hit->GetLambda()*1e6; //convert to nm
                //Hit position in PMT coordinates
                TVector3 hitPos = hit->GetPosition();
                PMTHitMap->Fill(hitPos.X(), hitPos.Y());
        
                totalHitTimeHist->Fill(tHit);
                wavelengthHist->Fill(wavelength);
            } 
        }

        if (verbose) { 
            std::cout << "------------------------------------------" << std::endl;
            std::cout << "EVENT " << ievent << std::endl;
            std::cout << " PEs: " << numPE << " PMTs: " << numPMT << std::endl;
        }
        ievent++; 
    
    } //end of while loop

    std::cout << numDetected << " photons detected." << std::endl;
    std::cout << "Writing." << std::endl;
  
    out->cd();

    TCanvas* canTotal = new TCanvas("totalTimeHist", "", 800, 600);
    totalHitTimeHist->SetStats(0);
    totalHitTimeHist->SetLineColor(1);
    totalHitTimeHist->GetXaxis()->SetTitle("Time (ns)");
    totalHitTimeHist->GetYaxis()->SetTitleOffset(1.5);
    totalHitTimeHist->Draw(); 
    canTotal->SetLogy();
    canTotal->Write();
  
    totalHitTimeHist->Write();
    PMTHitMap->Write();
    wavelengthHist->Write();

    std::cout << "Finished." << std::endl;

    return 0;
}
