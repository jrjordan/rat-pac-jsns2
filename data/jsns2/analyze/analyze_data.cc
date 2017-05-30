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
    double lightYieldCorrection = 5.0; //Simulate with low light yield for simulation
                                       //speed up. Correct in analysis

    bool usePMTLinearity = true; //Incorporate PMT linearity effects into the analysis
    double linearityCutoff = 250.0; //Assume the PMT is linear up to this number of PEs
    
    //Keep track of the number of events where more than a certain fraction of PEs are
    //lost due to saturation of the PMTs
    double highFraction = 0.1; //Cutoff to count event in lowPEEvents
    int lowPEEvents = 0;

    //PMT Linearity data measured by Double Chooz; digitized using WebPlotDigitizer
    //Data is somewhat sparse. Significant interpolation will be necessary

    //trueChargeVals = Expected number of PEs on the photocathode from the laser
    //measuredChargeVals = Measured PEs on the photocathode
    double trueChargeVals[11] = {9.6653, 19.0692, 30.7615, 57.7198, 105.5897, 208.2803, 340.1239, 541.5076, 989.7065, 1949.5120, 3180.4581};
    double measuredChargeVals[11] = {9.4985, 19.0252, 31.0185, 60.5512, 110.8390, 213.6038, 330.7905, 480.3571, 743.8894, 1108.3904, 1433.5887};

    //Used for random number generation in PMT linearity
    //Seed of 0 gives a new seed everytime the code is run
    TRandom3 rand(0);

    if ( nargs < 4 ) {
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

    //Uncorrected hit times
    TH1D* totalHitTimeHist = new TH1D("totalHitTimeHist", "", 1200, -100., 500.);
    TH1D* cherenkovHitTimeHist = new TH1D("cherekovHitTimeHist", "", 1200, -100., 500.);
    TH1D* scintHitTimeHist = new TH1D("scintHitTimeHist", "", 1200, -100, 500.);
    TH1D* reScatHitTimeHist = new TH1D("reScatHitTimeHist", "", 1200, -100., 500.);
  
    //Corrected hit times
    TH1D* totalCorrectedTimeHist = new TH1D("totalCorrectedTimeHist", "", 1200, -100., 500.);
    TH1D* cherekovCorrectedTimeHist = new TH1D("cherekovCorrectedTimeHist", "", 1200, -100., 500.);
    TH1D* scintCorrectedTimeHist = new TH1D("scintCorrectedTimeHist", "", 1200, -100., 500.);
    TH1D* reScatCorrectedTimeHist = new TH1D("reScatCorrectedTimeHist", "", 1200, -100., 500.);


    //Travel times of photons
    TH1D* travelTimeHist = new TH1D("travelTimeHist", "", 200, 0, 100);

    //Wavelength of incident photons for different creator processes
    TH1D* cherenkovWavelen = new TH1D("cherenkovWavelen", "", 200, 0, 1000);
    TH1D* scintWavelen = new TH1D("scintWavelen", "", 200, 0, 1000);
    TH1D* reemitWavelen = new TH1D("reemitWavelen", "", 200, 0, 1000);
    TH2D* wavelengthVsTime = new TH2D("wavelengthVsTime", "", 200, -140, 260, 150, 400, 700);
    
    //Event-level information
    TH1D* energyDepHist = new TH1D("energyDepHist", "", 100, 20, 60);
    TH1D* energyDepNoQuenchHist = new TH1D("energyDepNoQuenchHist", "", 100, 20, 60);
    TH1D* numScintPhotonHist = new TH1D("numScintPhotons", "", 200, 0, 20000);
    TH1D* numReemitPhotonHist = new TH1D("numReemitPhotons", "", 200, 0, 20000);
    //Number of photoelectrons created in a given event
    TH1D* numPEHist = new TH1D("numPEHist", "", 250, 0, 100000);
    
    //Maximum number of photons on a single PMT in each event
    TH1D* maxPhotons = new TH1D("maxPhotons", "", 100, 0, 20000);
    //Number of PEs in each PMT over all events
    TH1D* numPhotonsPerPMT = new TH1D("numPhotonsPerPMT", "", 100, 0, 20000);
   
    //Fraction of PEs lost after linearity corrections 
    TH1D* fracLost = new TH1D("fracLost", "Fraction of PEs Lost from Linearity", 100, 0, 0.5);

    int ievent = 0;
    int nevents = ds->GetTotal();

    std::cout << "This file contains " << nevents << " events." << std::endl;
    std::cout << "Processesing events." << std::endl;

    //Use to make a graph of fraction of PEs detected as a function of
    //distance of closest approact to PMT
    double closestApproachDist[nevents];
    double fracPELostVals[nevents];

    //Also keep track of how many PEs are created as a function of
    //distance from the origin
    double originDistVals[nevents];
    double numPEVals[nevents];

    //Also keep track of the maximum number of hits on a single PMT
    double maxHitsVals[nevents];


    while (ievent < nevents) {
        int numBadTiming = 0;
        RAT::DS::Root* root = ds->NextEvent();

        int numPE = 0;
        int numPMT = 0;
        double positionV[3];
        positionV[0] = positionV[1] = positionV[2] = 0.0;
        double radius = 0.0;

        //Get the Monte Carlo truth information
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
        double originDist = sqrt(radius*radius + positionV[2]*positionV[2]);
        originDistVals[ievent] = originDist;

        //Number of photoelectrons generated in PMTs for this event
        numPE = mc->GetNumPE()*lightYieldCorrection;
        numPEHist->Fill(numPE);
        numPEVals[ievent] = numPE;
        //Number of PMTs that got hit in this event
        numPMT = mc->GetMCPMTCount();
        //Count the maximum number of hits in a single PMT
        int maxHits = 0;
        //Count how many PEs actually got detected after applying linearity
        int numPEsDetected = 0;

        //Distance from vertex to closest PMT; start at huge value
        double closestPMTDist = 100000.0; //in mm

        // Count Photons
        for ( int iPMT = 0; iPMT < numPMT; iPMT++ ) {
            RAT::DS::MCPMT* PMT = mc->GetMCPMT( iPMT );
            int numHits = PMT->GetMCPhotonCount()*lightYieldCorrection;
            numPhotonsPerPMT->Fill(numHits);

            if (numHits > maxHits) maxHits = numHits;
 
            //Check if we need to apply a linearity correction
            double linFactor = 1.0;
            if (usePMTLinearity && numHits > linearityCutoff) {
                //Determine linearity factor through interpolation
        
                //Search for the two points bounding the one
                //we are interested in and compute the linearity factor
                for (int k = 0; k < 10; ++k) {
                    if (numHits < trueChargeVals[k+1]) {
                        double true1 = trueChargeVals[k]; double true2 = trueChargeVals[k+1];
                        double measured1 = measuredChargeVals[k]; double measured2 = measuredChargeVals[k+1];
                        double slope = (measured2 - measured1)/(true2 - true1);
                        double measuredPE = measured1 + slope*((double)numHits - true1);
                        linFactor = measuredPE/(double)numHits;
                        break;
                    }
                }
                //If numHits is larger than the highest value in trueChargeVals, we cannot interpolate
                //In this case, set the linearity factor to what it is at the highest measured point
                if (numHits > trueChargeVals[10]) linFactor = measuredChargeVals[10]/trueChargeVals[10];
                
            }
 
            //Get the PMT position
            int PMTID = PMT->GetID();
            TVector3 PMTPos = pmtinfo->GetPosition(PMTID);

            //Calculate the photon travel time. This is not a perfect calculation. We
            //assume the photon originates at the primary vertex and do not resolve
            //where on the PMT it hits. Could be improved later.
            //Also keep track of the closest PMT distance to the vertex
            double distX = (positionV[0] - PMTPos.X());        
            double distY = (positionV[1] - PMTPos.Y());        
            double distZ = (positionV[2] - PMTPos.Z());        
            double distance = TMath::Sqrt(distX*distX + distY*distY + distZ*distZ);
            if (distance < closestPMTDist) closestPMTDist = distance;
            //Effective speed of light is 299.972 mm/ns divided by index
            //of refraction which is 1.5 for scintillator + acrylic
            double travelTime = distance/(299.792/1.5); 

            for (int iHit = 0; iHit < PMT->GetMCPhotonCount(); iHit++) {
                //If the PMT is saturated, we may need to throw this hit out
                //based on the linearity factor
                if (usePMTLinearity && numHits > linearityCutoff && rand.Rndm() > linFactor) continue;
                numPEsDetected += (int)lightYieldCorrection;

                RAT::DS::MCPhoton* hit = PMT->GetMCPhoton( iHit );
	            double wavelen = hit->GetLambda() * 1.0e6; //Convert mm to nm
                
                //Check for dark hits. Not turned on so it would be a surprise
                if ( hit->IsDarkHit() ) {
                    if (verbose) std::cout << "We've got a dark hit!" << std::endl;
                    continue;
                }
 
	            double tHit = hit->GetHitTime();
	            double correctedTime = tHit - travelTime;

	            wavelengthVsTime->Fill(tHit, wavelen, lightYieldCorrection);

                //Fill some histograms for each hit. Fill specific ones based
                //on the creator process
	            travelTimeHist->Fill(travelTime, lightYieldCorrection);
	            totalHitTimeHist->Fill(tHit, lightYieldCorrection);
	            totalCorrectedTimeHist->Fill(correctedTime, lightYieldCorrection);
                if (hit->GetOriginFlag() == 0 ) {
                    cherenkovHitTimeHist->Fill(tHit, lightYieldCorrection);
	                cherekovCorrectedTimeHist->Fill(correctedTime, lightYieldCorrection);
  	                cherenkovWavelen->Fill(wavelen, lightYieldCorrection);
 	            }
	            else if (hit->GetOriginFlag() == 1) {
   	                scintHitTimeHist->Fill(tHit, lightYieldCorrection);
	                scintCorrectedTimeHist->Fill(correctedTime, lightYieldCorrection);
	                scintWavelen->Fill(wavelen, lightYieldCorrection);
                }
                else {
 	                reScatHitTimeHist->Fill(tHit, lightYieldCorrection);
	                reScatCorrectedTimeHist->Fill(correctedTime, lightYieldCorrection);
  	                reemitWavelen->Fill(wavelen, lightYieldCorrection);
                }
            }
        }
        double fracPELost = ( (double)numPE - (double)numPEsDetected )/(double)numPE;
        closestApproachDist[ievent] = closestPMTDist;
        fracPELostVals[ievent] = fracPELost;
        if (fracPELost > highFraction) lowPEEvents++; 
        fracLost->Fill(fracPELost);
        maxHitsVals[ievent] = maxHits;
        maxPhotons->Fill(maxHits);

        double energyDep = mc->GetMCSummary()->GetTotalScintEdep();
        double energyDepNoQuench = mc->GetMCSummary()->GetTotalScintEdepQuenched();
        double numScintPhotons = mc->GetMCSummary()->GetNumScintPhoton();
        double numReemitPhotons = mc->GetMCSummary()->GetNumReemitPhoton();
        energyDepHist->Fill(energyDep);
        energyDepNoQuenchHist->Fill(energyDepNoQuench);
        numScintPhotonHist->Fill(numScintPhotons);
        numReemitPhotonHist->Fill(numReemitPhotons);
       
        if (verbose) { 
            std::cout << "------------------------------------------" << std::endl;
            std::cout << "EVENT " << ievent << std::endl;
            std::cout << "PEs: " << numPE << " PMTs: " << numPMT << std::endl;
            std::cout << "Fraction of PEs Detected: " << fracPELost << std::endl;
            std::cout << "Position: " << positionV[0] << ", " << positionV[1] << ", " << positionV[2] << std::endl;
            std::cout << "Total Energy Deposited: " << energyDep << " MeV" << std::endl;
            std::cout << "Energy Deposited with Quenched Energy Removed: " << energyDepNoQuench << " MeV" << std::endl;
            std::cout << "Number of Scint Photons Generated: " << numScintPhotons << std::endl;
            std::cout << "Number of Reemit Photons Generated: " << numReemitPhotons << std::endl;
        }
        ievent++; 
    
    } //end of while loop

    std::cout << lowPEEvents << " events had more than " << highFraction*100 << "% of PEs lost." << std::endl;
    std::cout << "Writing." << std::endl;
    
    out->cd();
    travelTimeHist->Write();
    totalHitTimeHist->Write();
    scintHitTimeHist->Write();
    reScatHitTimeHist->Write();
    cherenkovHitTimeHist->Write();
    totalCorrectedTimeHist->Write();
    scintCorrectedTimeHist->Write();
    reScatCorrectedTimeHist->Write();
    cherekovCorrectedTimeHist->Write();

    TCanvas* canTotal = new TCanvas("totalTimeHist", "", 800, 600);
    totalHitTimeHist->SetStats(0);
    totalHitTimeHist->SetLineColor(1);
    totalHitTimeHist->Scale(1./(double)totalHitTimeHist->GetEntries());
    totalHitTimeHist->GetXaxis()->SetTitle("Time (ns)");
    totalHitTimeHist->GetYaxis()->SetTitle("Normalized Likelihood");
    totalHitTimeHist->GetYaxis()->SetTitleOffset(1.5);
    totalHitTimeHist->Draw();
    cherenkovHitTimeHist->SetStats(0);
    cherenkovHitTimeHist->SetLineColor(4);
    cherenkovHitTimeHist->Scale(1./(double)totalHitTimeHist->GetEntries());
    cherenkovHitTimeHist->Draw("same");
    scintHitTimeHist->SetStats(0);
    scintHitTimeHist->SetLineColor(2);
    scintHitTimeHist->Scale(1./(double)totalHitTimeHist->GetEntries());
    scintHitTimeHist->Draw("same");  
    reScatHitTimeHist->SetLineColor(8);
    reScatHitTimeHist->Scale(1./(double)totalHitTimeHist->GetEntries());
    reScatHitTimeHist->Draw("same");
    double fracCherenkovTot = 0, fracScintTot = 0, fracReScatTot = 0;
    fracCherenkovTot = 100*((double)cherenkovHitTimeHist->GetEntries()/(double)totalHitTimeHist->GetEntries());
    fracScintTot = 100*((double)scintHitTimeHist->GetEntries()/(double)totalHitTimeHist->GetEntries());
    fracReScatTot = 100*((double)reScatHitTimeHist->GetEntries()/(double)totalHitTimeHist->GetEntries());
    std::stringstream entry1Tot, entry2Tot, entry3Tot;
    entry1Tot << "Cherenkov Light ~ " << fracCherenkovTot << " %";
    entry2Tot << "Scintillation Light ~ " << fracScintTot << " %";
    entry3Tot << "Re-emitted Light ~ " << fracReScatTot << " %";
    TLegend* legendTot = new TLegend(0.6, 0.85, 0.9, 0.65); 
    legendTot->AddEntry(totalHitTimeHist, "Total Light", "l");
    legendTot->AddEntry(cherenkovHitTimeHist, entry1Tot.str().c_str(), "l");
    legendTot->AddEntry(scintHitTimeHist, entry2Tot.str().c_str(), "l");
    legendTot->AddEntry(reScatHitTimeHist, entry3Tot.str().c_str(), "l");
    legendTot->Draw("same");
    canTotal->SetLogy();

    canTotal->Write();

    TCanvas* wavelenCan = new TCanvas("wavelengthPlot", "", 800, 600);
    cherenkovWavelen->SetStats(0);
    cherenkovWavelen->SetLineColor(4); 
    cherenkovWavelen->GetYaxis()->SetRangeUser(0, 1.1*scintWavelen->GetMaximum());
    cherenkovWavelen->GetXaxis()->SetTitle("Wavelength (nm)");
    cherenkovWavelen->Draw();
    scintWavelen->SetStats(0);
    scintWavelen->SetLineColor(2); 
    scintWavelen->Draw("same");
    reemitWavelen->SetStats(0);
    reemitWavelen->SetLineColor(8); 
    reemitWavelen->Draw("same");
    TLegend* legendWavelen = new TLegend(0.6, 0.85, 0.9, 0.65); 
    legendWavelen->AddEntry(cherenkovWavelen, entry1Tot.str().c_str(), "l");
    legendWavelen->AddEntry(scintWavelen, entry2Tot.str().c_str(), "l");
    legendWavelen->AddEntry(reemitWavelen, entry3Tot.str().c_str(), "l");
    legendWavelen->Draw("same");
    wavelenCan->Write();
    
    TCanvas* fracLostCan = new TCanvas("fracLostCan", "", 800, 600);
    fracLost->SetLineWidth(2);
    fracLost->GetXaxis()->SetTitle("Fraction of PEs Lost - XX MeV");
    fracLost->GetXaxis()->SetTitleSize(0.05);
    fracLost->GetXaxis()->SetTitleOffset(0.9);
    fracLost->SetStats(0);
    fracLost->Draw();
    fracLostCan->SetLogy();

    TGraph* fracLostVsDist = new TGraph(nevents, closestApproachDist, fracPELostVals);
    TCanvas* fracLostVsDistCan = new TCanvas("fracLostVsDistCan", "", 800, 600);
    fracLostVsDist->SetTitle("Fraction Lost vs. Distance to Closest PMT - XX MeV");
    fracLostVsDist->SetMarkerStyle(7);
    fracLostVsDist->Draw("AP");
    fracLostVsDist->GetXaxis()->SetTitle("Distance (mm)");
    
    TGraph* fracLostVsOriginDist = new TGraph(nevents, originDistVals, fracPELostVals);
    TCanvas* fracLostVsOriginDistCan = new TCanvas("fracLostVsOriginDistCan", "", 800, 600);
    fracLostVsOriginDist->SetTitle("Fraction Lost vs. Distance from Origin - XX MeV");
    fracLostVsOriginDist->SetMarkerStyle(7);
    fracLostVsOriginDist->Draw("AP");
    fracLostVsOriginDist->GetXaxis()->SetTitle("Distance from Origin (mm)");
    
    TGraph* numPEVsOriginDist = new TGraph(nevents, originDistVals, numPEVals);
    TCanvas* numPEVsOriginDistCan = new TCanvas("numPEVsOriginDistCan", "", 800, 600);
    numPEVsOriginDist->SetTitle("# of PE vs. Distance from Origin - XX MeV");
    numPEVsOriginDist->SetMarkerStyle(7);
    numPEVsOriginDist->Draw("AP");
    numPEVsOriginDist->GetXaxis()->SetTitle("Distance from Origin (mm)");
    
    TGraph* maxPEVsOriginDist = new TGraph(nevents, originDistVals, maxHitsVals);
    TCanvas* maxPEVsOriginDistCan = new TCanvas("maxPEVsOriginDistCan", "", 800, 600);
    maxPEVsOriginDist->SetTitle("Max PE vs. Distance from Origin - XX MeV");
    maxPEVsOriginDist->SetMarkerStyle(7);
    maxPEVsOriginDist->Draw("AP");
    maxPEVsOriginDist->GetXaxis()->SetTitle("Distance from Origin (mm)");

    numPEHist->SetLineWidth(2);
    numPEHist->SetTitle("Number of PEs Created - XX MeV");
    numPEHist->GetXaxis()->SetTitle("# of PE");
    numPEHist->GetXaxis()->SetTitleSize(0.05);
    numPEHist->GetXaxis()->SetTitleOffset(0.9);
    numPEHist->SetStats(0);
    
    numPhotonsPerPMT->SetLineWidth(2);
    numPhotonsPerPMT->SetTitle("Number of PEs Created Per PMT - XX MeV");
    numPhotonsPerPMT->GetXaxis()->SetTitle("# of PE in each PMT");
    numPhotonsPerPMT->GetXaxis()->SetTitleSize(0.05);
    numPhotonsPerPMT->GetXaxis()->SetTitleOffset(0.9);
    numPhotonsPerPMT->SetStats(0);

    energyDepHist->Write();
    energyDepNoQuenchHist->Write();
    numScintPhotonHist->Write();
    numReemitPhotonHist->Write();
    numPEHist->Write();
    wavelengthVsTime->Write();
    maxPhotons->Write();
    numPhotonsPerPMT->Write();
    fracLost->Write();
    fracLostVsDist->Write();
    fracLostVsOriginDist->Write();
    numPEVsOriginDist->Write();
    maxPEVsOriginDist->Write();
    fracLostCan->Write();
    fracLostVsDistCan->Write();
    fracLostVsOriginDistCan->Write();
    numPEVsOriginDistCan->Write();
    maxPEVsOriginDistCan->Write();

    std::cout << "Finished." << std::endl;

    return 0;
}
