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
    return 0;
  }

  std::string inputfile = argv[1];
  std::string outfile = argv[2];
  std::string verboseString = argv[3];
  bool verbose = verboseString != "0";

    //Load the TTree from the output RAT file containing the run-level information
    TChain runTree("runT");
    runTree.Add( inputfile.c_str() );
    //Tell the RunStore where to look for run-level information
    RAT::DS::RunStore::SetReadTree(&runTree);

    //Load the run
    std::cout << "Loading Run." << std::endl;
    RAT::DS::Run* run = RAT::DS::RunStore::Get()->GetRun(1);
    if (run == 0) std::cout << "Run not found!" << std::endl;

    //Load in the PMT info
    std::cout << "Run Loaded." << std::endl;
    RAT::DS::PMTInfo* pmtinfo = run->GetPMTInfo();
    std::cout << "\nThis geometry contains " << pmtinfo->GetPMTCount() << " PMTs." << std::endl;

  RAT::DSReader* ds = new RAT::DSReader( inputfile.c_str() ); 

  TFile* out = new TFile(outfile.c_str(), "RECREATE" );

  //Hit positions
  TH2D* r2zHitPosHist = new TH2D("r2zHitPosHist", "r^{2} vs. z (cm)", 40, -1., 100., 50, -125., 125.);
  TH2D* xyHitPosHist = new TH2D("xyHitPosHist", "x vs y (cm)", 40, -10., 10., 40, -10., 10.);
  TH1D* zHitPosHist= new TH1D("zHitPosHist", "z Hit Positions", 141, -124., 124.);

  //Uncorrected hit times
  TH1D* totalHitTimeHist = new TH1D("totalHitTimeHist", "Hit Times", 1200, -100., 500.);
  TH1D* scintHitTimeHist = new TH1D("scintHitTimeHist", "Scintillation Hit Times", 1200, -100, 500);

  TH1D* wavelenHist = new TH1D("wavelenHist", "Wavelengths", 200, 0, 1000);
  
  TH1D* energyDepHist = new TH1D("energyDepHist", "", 100, 20, 60);
  TH1D* energyDepNoQuenchHist = new TH1D("energyDepNoQuenchHist", "", 100, 20, 60);
  TH1D* numScintPhotonHist = new TH1D("numScintPhotons", "", 200, 0, 20000);
  TH1D* numReemitPhotonHist = new TH1D("numReemitPhotons", "", 200, 0, 20000);
  TH1D* photonsPerEnergyDep = new TH1D("photonsPerEnergyDep", "Light Yield", 200, 0, 2000);

  double numReemit = 0, numScint = 0;
  //Keep track of the number of scintillation and re-emission photons generated

  // variables we want
  int numPE;
  int numPMT;
  double positionV[3]; // true vertex
  double radius;

  int ievent = 0;
  int nevents = ds->GetTotal();

  std::cout << "Number of events: " << nevents << std::endl;

  while (ievent < nevents) {
    int numBadTiming = 0;
    RAT::DS::Root* root = ds->NextEvent();
    
    // --------------------------------
    // Clear Variables
    numPE = 0;
    numPMT = 0;
    positionV[0] = positionV[1] = positionV[2] = 0.0;
    radius = 0.0;
    // --------------------------------

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

    // true vertex of primary particle
    positionV[0] = mc->GetMCParticle(0)->GetPosition().X()/10.0; //change to cm
    positionV[1] = mc->GetMCParticle(0)->GetPosition().Y()/10.0; //change to cm
    positionV[2] = mc->GetMCParticle(0)->GetPosition().Z()/10.0; //change to cm
    radius = sqrt(positionV[0]*positionV[0] + positionV[1]*positionV[1]);

    numPE = mc->GetNumPE();
    numPMT = mc->GetMCPMTCount();

    // count stuff
    for ( int iPMT = 0; iPMT < numPMT; iPMT++ ) {
      RAT::DS::MCPMT* PMT = mc->GetMCPMT( iPMT );
      int numHits = PMT->GetMCPhotonCount();
      int PMTID = PMT->GetID();
      TVector3 pmtPos = pmtinfo->GetPosition(PMTID);

      for (int iHit = 0; iHit < numHits; iHit++) {
	RAT::DS::MCPhoton* hit = PMT->GetMCPhoton( iHit );
	double wavelen = hit->GetLambda() * 1.0e6; //Convert mm to nm
    if ( hit->IsDarkHit() ) {
        std::cout << "We've got a dark hit!" << std::endl;
        continue;
    }
	
	double tHit = hit->GetHitTime();


	xyHitPosHist->Fill(pmtPos.X()/10.0, pmtPos.Y()/10.0);
	r2zHitPosHist->Fill(pmtPos.X()*pmtPos.X()/100.0 + pmtPos.Y()*pmtPos.Y()/100.0, pmtPos.Z()/10.0);
	zHitPosHist->Fill(-pmtPos.Z()/10.0);
    totalHitTimeHist->Fill(tHit);
    wavelenHist->Fill(wavelen);
    if (hit->GetOriginFlag() == 1) scintHitTimeHist->Fill(tHit);
      }
    }


    double energyDep = mc->GetMCSummary()->GetTotalScintEdep();
    double energyDepNoQuench = mc->GetMCSummary()->GetTotalScintEdepQuenched();
    double numScintPhotons = mc->GetMCSummary()->GetNumScintPhoton();
    double numReemitPhotons = mc->GetMCSummary()->GetNumReemitPhoton();
    numReemit += numReemitPhotons; numScint += numScintPhotons; 
    energyDepHist->Fill(energyDep);
    energyDepNoQuenchHist->Fill(energyDepNoQuench);
    numScintPhotonHist->Fill(numScintPhotons);
    numReemitPhotonHist->Fill(numReemitPhotons);
    photonsPerEnergyDep->Fill(numScintPhotons/energyDep);  
 
    if (verbose) { 
        std::cout << "------------------------------------------" << std::endl;
        std::cout << "EVENT " << ievent << std::endl;
        std::cout << " PEs: " << numPE << " PMTs: " << numPMT << std::endl;
        std::cout << "Position: " << positionV[0] << ", " << positionV[1] << ", " << positionV[2] << std::endl;
        std::cout << "Total Energy Deposited: " << energyDep << " MeV" << std::endl;
        std::cout << "Energy Deposited with Quenched Energy Removed: " << energyDepNoQuench << " MeV" << std::endl;
        std::cout << "Number of Scint Photons Generated: " << numScintPhotons << std::endl;
        std::cout << "Number of Reemit Photons Generated: " << numReemitPhotons << std::endl;
    }
    ievent++; 
    
  } //end of while loop

  std::cout << "Writing." << std::endl;
  std::cout << "Total number of re-emitted photons generated: " << numReemit << std::endl;
  std::cout << "Total number of scintillation photons generated: " << numScint << std::endl;

  out->cd();

  TCanvas* canTotal = new TCanvas("totalTimeHist", "", 800, 600);
  totalHitTimeHist->SetStats(0);
  totalHitTimeHist->SetLineColor(1);
  totalHitTimeHist->GetXaxis()->SetTitle("Time (ns)");
  totalHitTimeHist->GetYaxis()->SetTitleOffset(1.5);
  totalHitTimeHist->Draw(); 
  canTotal->SetLogy();
  canTotal->Write();
  
  TCanvas* canScint = new TCanvas("scintTimeHist", "", 800, 600);
  scintHitTimeHist->SetStats(0);
  scintHitTimeHist->SetLineColor(1);
  scintHitTimeHist->GetXaxis()->SetTitle("Time (ns)");
  scintHitTimeHist->GetYaxis()->SetTitleOffset(1.5);
  scintHitTimeHist->Draw(); 
  canScint->SetLogy();
  canScint->Write();
  
  TCanvas* canWavelen = new TCanvas("wavelenHist", "", 800, 600);
  wavelenHist->SetStats(0);
  wavelenHist->SetLineColor(1);
  wavelenHist->GetXaxis()->SetTitle("Wavelength (nm)");
  wavelenHist->GetYaxis()->SetTitleOffset(1.5);
  wavelenHist->Draw(); 
  canWavelen->SetLogy();
  canWavelen->Write();

  TCanvas* xyHitMapCan = new TCanvas("xyHitMapCan", "", 800, 600);
  xyHitPosHist->SetStats(0);
  xyHitPosHist->GetXaxis()->SetTitle("x position (cm)");
  xyHitPosHist->GetYaxis()->SetTitle("y position (cm)");
  xyHitPosHist->Draw("COLZ");
  xyHitMapCan->SetLogz();
  xyHitMapCan->Write();
  
  TCanvas* r2zHitMapCan = new TCanvas("r2zHitMapCan", "", 800, 600);
  r2zHitPosHist->SetStats(0);
  r2zHitPosHist->GetXaxis()->SetTitle("r^{2} position (cm)");
  r2zHitPosHist->GetYaxis()->SetTitle("z position (cm)");
  r2zHitPosHist->Draw("COLZ");
  r2zHitMapCan->SetLogz();
  r2zHitMapCan->Write();
  
  TCanvas* zHitMapCan = new TCanvas("zHitMapCan", "", 800, 600);
  zHitPosHist->SetStats(0);
  zHitPosHist->GetXaxis()->SetTitle("z position (cm)");
  zHitPosHist->Draw();
  zHitMapCan->Write();
  
  TCanvas* photonsPerEnergyCan = new TCanvas("photonsPerEnergyCan", "", 800, 600);
  photonsPerEnergyDep->SetStats(0);
  photonsPerEnergyDep->GetXaxis()->SetTitle("Light Yield (photons/MeV)");
  photonsPerEnergyDep->Draw();
  photonsPerEnergyCan->Write();

  energyDepHist->Write();
  energyDepNoQuenchHist->Write();
  numScintPhotonHist->Write();
  numReemitPhotonHist->Write();
  xyHitPosHist->Write();
  r2zHitPosHist->Write();
  totalHitTimeHist->Write();

  std::cout << "Finished." << std::endl;

  return 0;
}
