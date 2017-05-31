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

    //Simulation is too slow with the full light yield.
    //Speed up the simulation by using a lower light yield
    //and correcting the results after the fact
    int lightYieldCorrection = 5; 

    if ( nargs < 4 ) {
        std::cout << "Usage: simpleAnalysis <input RAT root file> <output rootfile> <verbose flag (0 or 1)>" << std::endl;
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
    if (run == 0) {
        std::cout << "Run not found! Exiting." << std::endl;
        return 0;
    }

    //Load in the PMT info
    std::cout << "Run Loaded." << std::endl;
    RAT::DS::PMTInfo* pmtinfo = run->GetPMTInfo();
    std::cout << "\nThis geometry contains " << pmtinfo->GetPMTCount() << " PMTs." << std::endl;

    //DSReader class lets us loop through events and read the MC
    //information from the output data structure
    RAT::DSReader* ds = new RAT::DSReader( inputfile.c_str() ); 

    //File to write output plots to
    TFile* out = new TFile(outfile.c_str(), "RECREATE" );

    //Uncorrected hit times
    TH1D* totalHitTimeHist = new TH1D("totalHitTimeHist", "", 1200, -100., 500.);
    TH1D* cherenkovHitTimeHist = new TH1D("cherekovHitTimeHist", "", 1200, -100., 500.);
    TH1D* scintHitTimeHist = new TH1D("scintHitTimeHist", "", 1200, -100, 500.);
    TH1D* reScatHitTimeHist = new TH1D("reScatHitTimeHist", "", 1200, -100., 500.);
  
    //Corrected hit times
    TH1D* totalCorrectedTimeHist = new TH1D("totalCorrectedTimeHist", "", 1200, -100., 500.);
    TH1D* cherenkovCorrectedTimeHist = new TH1D("cherekovCorrectedTimeHist", "", 1200, -100., 500.);
    TH1D* scintCorrectedTimeHist = new TH1D("scintCorrectedTimeHist", "", 1200, -100., 500.);
    TH1D* reScatCorrectedTimeHist = new TH1D("reScatCorrectedTimeHist", "", 1200, -100., 500.);

    //Wavelength of incident photons for different creator processes
    TH1D* cherenkovWavelen = new TH1D("cherenkovWavelen", "", 200, 0, 1000);
    TH1D* scintWavelen = new TH1D("scintWavelen", "", 200, 0, 1000);
    TH1D* reemitWavelen = new TH1D("reemitWavelen", "", 200, 0, 1000);
    TH2D* wavelengthVsTime = new TH2D("wavelengthVsTime", "", 200, -140, 260, 150, 400, 700);
   
    //Map of where the PMTs is being hit on its surface
    //All PMTs are shown on the same plot
    TH2D* PMTHitMap = new TH2D("PMTHitMap", "", 130, -130, 130, 130, -130, 130);
 
    //Event-level information
    TH1D* energyDepHist = new TH1D("energyDepHist", "", 100, 0, 20);
    TH1D* numScintPhotonHist = new TH1D("numScintPhotons", "", 150, 0, 30000);
    TH1D* numReemitPhotonHist = new TH1D("numReemitPhotons", "", 200, 0, 40000);
    
    //Number of photoelectrons created in a given event
    TH1D* numPEHist = new TH1D("numPEHist", "", 250, 0, 25000);
    TH1D* numPEPerMeV = new TH1D("numPEPerMeV", "", 200, 0, 2000);

    //Keep track of which event we're looking at
    int ievent = 0;
    int nevents = ds->GetTotal();

    std::cout << "This file contains " << nevents << " events." << std::endl;
    std::cout << "Processesing events." << std::endl;

    while (ievent < nevents) {
        //The DS::Root object contains the event-level information
        RAT::DS::Root* root = ds->NextEvent();

        //Some variables we're interested in
        int numPE = 0;
        int numPMT = 0;
        double positionV[3];
        positionV[0] = positionV[1] = positionV[2] = 0.0;
        double radius = 0.0;

        //Get the Monte Carlo truth information for this event
        RAT::DS::MC* mc = root->GetMC();
        
        //Check to make sure we have MC information and particles
        if ( mc == NULL ) {
            std::cout << "mc == NULL" << std::endl;
            break;
        }
        if ( mc->GetMCParticleCount() == 0 ) {
            std::cout << "ParticleCount == 0" << std::endl;
            ievent++;
            continue;
        }

        // True vertex of primary particle
        positionV[0] = mc->GetMCParticle(0)->GetPosition().X();
        positionV[1] = mc->GetMCParticle(0)->GetPosition().Y();
        positionV[2] = mc->GetMCParticle(0)->GetPosition().Z();
        radius = sqrt(positionV[0]*positionV[0] + positionV[1]*positionV[1]);

        //Number of PMTs that got hit in this event
        numPMT = mc->GetMCPMTCount();

        //Loop over all the hit PMTs in the event
        for ( int iPMT = 0; iPMT < numPMT; iPMT++ ) {

            //DS::MCPMT object contains all the information about a single PMT
            //for a given event including all the hits on the PMT
            RAT::DS::MCPMT* PMT = mc->GetMCPMT( iPMT );
            int numHits = PMT->GetMCPhotonCount()*lightYieldCorrection;

            //Get the PMT position; each PMT has a unique ID specifying 
            //it in the simulation
            int PMTID = PMT->GetID();
            TVector3 PMTPos = pmtinfo->GetPosition(PMTID);

            //Calculate the photon travel time. This is not a perfect calculation. We
            //assume the photon originates at the primary vertex and do not resolve
            //where on the PMT it hits. Would also need to use the wavelength-dependent
            //index of refraction rather than fixing it. Could be improved later.
            double distX = (positionV[0] - PMTPos.X());        
            double distY = (positionV[1] - PMTPos.Y());        
            double distZ = (positionV[2] - PMTPos.Z());        
            double distance = TMath::Sqrt(distX*distX + distY*distY + distZ*distZ);
            
            //Effective speed of light is 299.972 mm/ns divided by index
            //of refraction which is 1.5 for scintillator + acrylic
            double travelTime = distance/(299.792/1.5); 

            for (int iHit = 0; iHit < PMT->GetMCPhotonCount(); iHit++) {

                //The DS::MCPhoton object contains all the information on a single
                //PE generated in the PMT including the hit time, wavelength, and origin
                RAT::DS::MCPhoton* hit = PMT->GetMCPhoton( iHit );
	            double wavelen = hit->GetLambda() * 1.0e6; //Convert mm to nm
               
                //Hit position in local PMT coordinates (in mm)
                TVector3 hitPos = hit->GetPosition();
                PMTHitMap->Fill(hitPos.X(), hitPos.Y());
 
                //Check for dark hits. Not turned on so it would be a surprise
                //if we saw one.
                if ( hit->IsDarkHit() ) {
                    if (verbose) std::cout << "We've got a dark hit!" << std::endl;
                    continue;
                }
 
	            double tHit = hit->GetHitTime();
                //Rough corrected time
	            double correctedTime = tHit - travelTime;

	            wavelengthVsTime->Fill(tHit, wavelen, lightYieldCorrection);

                //Fill some histograms for each hit. Fill specific ones based
                //on the creator process. Note that Cerenkov photons do not
                //get the light yield correction because they are not produced
                //according to the light yield.
                if (hit->GetOriginFlag() == 0 ) {
                    numPE += 1;
                    cherenkovHitTimeHist->Fill(tHit);
	                cherenkovCorrectedTimeHist->Fill(correctedTime);
  	                cherenkovWavelen->Fill(wavelen);
	                totalHitTimeHist->Fill(tHit);
	                totalCorrectedTimeHist->Fill(correctedTime);
 	            }
	            else if (hit->GetOriginFlag() == 1) {
                    numPE += lightYieldCorrection;
   	                scintHitTimeHist->Fill(tHit, lightYieldCorrection);
	                scintCorrectedTimeHist->Fill(correctedTime, lightYieldCorrection);
	                scintWavelen->Fill(wavelen, lightYieldCorrection);
	                totalHitTimeHist->Fill(tHit, lightYieldCorrection);
	                totalCorrectedTimeHist->Fill(correctedTime, lightYieldCorrection);
                }
                else {
                    numPE += lightYieldCorrection;
 	                reScatHitTimeHist->Fill(tHit, lightYieldCorrection);
	                reScatCorrectedTimeHist->Fill(correctedTime, lightYieldCorrection);
  	                reemitWavelen->Fill(wavelen, lightYieldCorrection);
	                totalHitTimeHist->Fill(tHit, lightYieldCorrection);
	                totalCorrectedTimeHist->Fill(correctedTime, lightYieldCorrection);
                }
            }
        }
 
        //Get some event-level information and fill some histograms
        double energyDep = mc->GetMCSummary()->GetTotalScintEdep();
        double numScintPhotons = mc->GetMCSummary()->GetNumScintPhoton();
        double numReemitPhotons = mc->GetMCSummary()->GetNumReemitPhoton();
        numPEHist->Fill(numPE);
        numPEPerMeV->Fill((double)numPE/energyDep); 
        energyDepHist->Fill(energyDep);
        numScintPhotonHist->Fill(numScintPhotons);
        numReemitPhotonHist->Fill(numReemitPhotons);
       
        if (verbose) { 
            std::cout << "------------------------------------------" << std::endl;
            std::cout << "EVENT " << ievent << std::endl;
            std::cout << "PEs: " << numPE << " PMTs: " << numPMT << std::endl;
            std::cout << "Position: " << positionV[0] << ", " << positionV[1] << ", " << positionV[2] << std::endl;
            std::cout << "Total Energy Deposited: " << energyDep << " MeV" << std::endl;
            std::cout << "Number of Scint Photons Generated: " << numScintPhotons << std::endl;
            std::cout << "Number of Reemit Photons Generated: " << numReemitPhotons << std::endl;
        }
        ievent++; 
    
    } //End of while loop

    std::cout << "Writing." << std::endl;
   
    //Make some nice plots and fill the output file

    out->cd();

    TCanvas* canTiming = new TCanvas("Timing Histograms", "", 800, 600);
    totalHitTimeHist->SetStats(0);
    totalHitTimeHist->SetLineColor(1);
    totalHitTimeHist->GetXaxis()->SetTitle("Time (ns)");
    totalHitTimeHist->GetYaxis()->SetTitle("Normalized");
    totalHitTimeHist->GetYaxis()->SetTitleOffset(1.5);
    totalHitTimeHist->Scale(1.0/(double)totalHitTimeHist->GetEntries());
    totalHitTimeHist->Draw(); 
    cherenkovHitTimeHist->SetStats(0);
    cherenkovHitTimeHist->SetLineColor(4);
    cherenkovHitTimeHist->Scale(1.0/(double)totalHitTimeHist->GetEntries());
    cherenkovHitTimeHist->Draw("same");
    scintHitTimeHist->SetStats(0);
    scintHitTimeHist->SetLineColor(2);
    scintHitTimeHist->Scale(1.0/(double)totalHitTimeHist->GetEntries());
    scintHitTimeHist->Draw("same");  
    reScatHitTimeHist->SetStats(0);
    reScatHitTimeHist->SetLineColor(8);
    reScatHitTimeHist->Scale(1.0/(double)totalHitTimeHist->GetEntries());
    reScatHitTimeHist->Draw("same");
    double fracCherenkov = 0, fracScint = 0, fracReScat = 0;
    fracCherenkov = 100*((double)cherenkovHitTimeHist->GetEntries()/(double)totalHitTimeHist->GetEntries());
    fracScint = 100*((double)scintHitTimeHist->GetEntries()/(double)totalHitTimeHist->GetEntries());
    fracReScat = 100*((double)reScatHitTimeHist->GetEntries()/(double)totalHitTimeHist->GetEntries());
    std::stringstream entry1, entry2, entry3;
    entry1 << "Cherenkov Light ~ " << fracCherenkov << " %";
    entry2 << "Scintillation Light ~ " << fracScint << " %";
    entry3 << "Re-emitted Light ~ " << fracReScat << " %";
    TLegend* legend = new TLegend(0.5, 0.85, 0.9, 0.65); 
    legend->AddEntry(totalHitTimeHist, "Total Light", "l");
    legend->AddEntry(cherenkovHitTimeHist, entry1.str().c_str(), "l");
    legend->AddEntry(scintHitTimeHist, entry2.str().c_str(), "l");
    legend->AddEntry(reScatHitTimeHist, entry3.str().c_str(), "l");
    legend->Draw("same");
    canTiming->SetLogy();
    canTiming->Write();
    
    totalHitTimeHist->Write();
    scintHitTimeHist->GetXaxis()->SetTitle("Time (ns)");
    scintHitTimeHist->Write();
    reScatHitTimeHist->GetXaxis()->SetTitle("Time (ns)");
    reScatHitTimeHist->Write();
    cherenkovHitTimeHist->GetXaxis()->SetTitle("Time (ns)");
    cherenkovHitTimeHist->Write();
    
    TCanvas* canTimingCorrected = new TCanvas("Corrected Timing Histograms", "", 800, 600);
    totalCorrectedTimeHist->SetStats(0);
    totalCorrectedTimeHist->SetLineColor(1);
    totalCorrectedTimeHist->GetXaxis()->SetTitle("Time (ns)");
    totalCorrectedTimeHist->GetYaxis()->SetTitle("Normalized");
    totalCorrectedTimeHist->GetYaxis()->SetTitleOffset(1.5);
    totalCorrectedTimeHist->Scale(1.0/(double)totalCorrectedTimeHist->GetEntries());
    totalCorrectedTimeHist->Draw();
    cherenkovCorrectedTimeHist->SetStats(0);
    cherenkovCorrectedTimeHist->SetLineColor(4);
    cherenkovCorrectedTimeHist->Scale(1.0/(double)totalCorrectedTimeHist->GetEntries());
    cherenkovCorrectedTimeHist->Draw("same");
    scintCorrectedTimeHist->SetStats(0);
    scintCorrectedTimeHist->SetLineColor(2);
    scintCorrectedTimeHist->Scale(1.0/(double)totalCorrectedTimeHist->GetEntries());
    scintCorrectedTimeHist->Draw("same");  
    reScatCorrectedTimeHist->SetStats(0);
    reScatCorrectedTimeHist->SetLineColor(8);
    reScatCorrectedTimeHist->Scale(1.0/(double)totalCorrectedTimeHist->GetEntries());
    reScatCorrectedTimeHist->Draw("same");
    TLegend* legendCorrected = new TLegend(0.5, 0.85, 0.9, 0.65); 
    legendCorrected->AddEntry(totalCorrectedTimeHist, "Total Light", "l");
    legendCorrected->AddEntry(cherenkovCorrectedTimeHist, entry1.str().c_str(), "l");
    legendCorrected->AddEntry(scintCorrectedTimeHist, entry2.str().c_str(), "l");
    legendCorrected->AddEntry(reScatCorrectedTimeHist, entry3.str().c_str(), "l");
    legendCorrected->Draw("same");
    canTimingCorrected->SetLogy();
    canTimingCorrected->Write();
    
    totalCorrectedTimeHist->Write();
    scintCorrectedTimeHist->GetXaxis()->SetTitle("Time (ns)");
    scintCorrectedTimeHist->Write();
    reScatCorrectedTimeHist->GetXaxis()->SetTitle("Time (ns)");
    reScatCorrectedTimeHist->Write();
    cherenkovCorrectedTimeHist->GetXaxis()->SetTitle("Time (ns)");
    cherenkovCorrectedTimeHist->Write();


    TCanvas* wavelenCan = new TCanvas("wavelengthPlot", "", 800, 600);
    cherenkovWavelen->SetStats(0);
    cherenkovWavelen->SetLineColor(4); 
    scintWavelen->GetXaxis()->SetTitle("Wavelength (nm)");
    scintWavelen->GetYaxis()->SetTitle("Normalized");
    scintWavelen->GetYaxis()->SetTitleOffset(1.5);
    scintWavelen->SetStats(0);
    scintWavelen->SetLineColor(2); 
    reemitWavelen->SetStats(0);
    reemitWavelen->SetLineColor(8); 
    TLegend* legendWavelen = new TLegend(0.5, 0.85, 0.9, 0.65); 
    legendWavelen->AddEntry(cherenkovWavelen, entry1.str().c_str(), "l");
    legendWavelen->AddEntry(scintWavelen, entry2.str().c_str(), "l");
    legendWavelen->AddEntry(reemitWavelen, entry3.str().c_str(), "l");
    scintWavelen->DrawNormalized();
    reemitWavelen->DrawNormalized("same");
    cherenkovWavelen->DrawNormalized("same");
    legendWavelen->Draw("same");
    wavelenCan->Write();

    numPEHist->SetLineWidth(2);
    numPEHist->GetXaxis()->SetTitle("# of PE");
    numPEHist->SetStats(0);
    numPEHist->Write();
    
    numPEPerMeV->SetLineWidth(2);
    numPEPerMeV->GetXaxis()->SetTitle("# of PE Per MeV");
    numPEPerMeV->SetStats(0);
    numPEPerMeV->Write();

    energyDepHist->GetXaxis()->SetTitle("Energy Deposited (MeV)");
    energyDepHist->SetStats(0);
    energyDepHist->SetLineWidth(2);
    energyDepHist->Write();
    
    numScintPhotonHist->GetXaxis()->SetTitle("Number of Scint Photons Generated");
    numScintPhotonHist->SetStats(0);
    numScintPhotonHist->SetLineWidth(2);
    numScintPhotonHist->Write();
    
    numReemitPhotonHist->GetXaxis()->SetTitle("Number of Reemit Photons Generated");
    numReemitPhotonHist->SetStats(0); 
    numReemitPhotonHist->SetLineWidth(2);
    numReemitPhotonHist->Write();
    
    wavelengthVsTime->GetXaxis()->SetTitle("Time (ns)");
    wavelengthVsTime->GetYaxis()->SetTitle("Wavelength (nm)");
    wavelengthVsTime->SetStats(0);
    wavelengthVsTime->Write();
    
    PMTHitMap->GetXaxis()->SetTitle("X Position on PMT Surface (mm)");
    PMTHitMap->GetYaxis()->SetTitle("Y Position on PMT Surface (mm)");
    PMTHitMap->SetStats(0);
    PMTHitMap->SetDrawOption("COLZ");
    PMTHitMap->Write();

    std::cout << "Finished." << std::endl;

    return 0;
}
