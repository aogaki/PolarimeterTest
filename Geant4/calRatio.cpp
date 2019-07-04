#include <iostream>
#include <memory>

#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>
#include <TString.h>
#include <TTree.h>


const TString outName = "hists.root";

void GetResults(Double_t ene = 15.1)
{
   auto fileName = Form("pol%2.1f.root", ene);
   std::unique_ptr<TFile> file(new TFile(fileName, "READ"));
   auto tree = (TTree *)file->Get("PolMeter");
   tree->SetBranchStatus("*", kFALSE);

   Int_t eventID;
   tree->SetBranchStatus("EventID", kTRUE);
   tree->SetBranchAddress("EventID", &eventID);

   Double_t depEne;
   tree->SetBranchStatus("DepositEnergy", kTRUE);
   tree->SetBranchAddress("DepositEnergy", &depEne);

   Double_t time;
   tree->SetBranchStatus("Time", kTRUE);
   tree->SetBranchAddress("Time", &time);

   Char_t volName[16];
   tree->SetBranchStatus("VolName", kTRUE);
   tree->SetBranchAddress("VolName", volName);

   std::unique_ptr<TH1D> hisIn(new TH1D("hisIn", "In plane", 2000, 0., 20.));
   // auto hisIn(new TH1D("hisIn", "In plane", 2000, 0., 20.));
   hisIn->SetXTitle("Deposit energy [MeV]");
   hisIn->SetDirectory(0);
   std::unique_ptr<TH1D> hisOut(new TH1D("hisOut", "Out plane", 2000, 0., 20.));
   // auto hisOut(new TH1D("hisOut", "Out plane", 2000, 0., 20.));
   hisOut->SetXTitle("Deposit energy [MeV]");
   hisOut->SetDirectory(0);


   constexpr Double_t timeWindow[2] = {0., 50000.};
   const Int_t nHit = tree->GetEntries();
   auto inEne = 0.;
   auto outEne = 0.;
   tree->GetEntry(0);
   auto currentEveID = eventID;
   for (auto iHit = 0; iHit < nHit; iHit++) {
      tree->GetEntry(iHit);
      auto detName = TString(volName);
      if (currentEveID == eventID) {
         if(time < timeWindow[0] || time > timeWindow[1]) continue;
         if (detName == "Detector1")
            inEne += depEne;
         else
            outEne += depEne;
      } else {
         if (inEne > 0.) hisIn->Fill(inEne);
         if (outEne > 0.) hisOut->Fill(outEne);
            inEne = 0.;
            outEne = 0.;
         if(time < timeWindow[0] || time > timeWindow[1]) continue;
         if (detName == "Detector1") {
            inEne = depEne;
         } else {
            outEne = depEne;
         }
      }
   }
   file->Close();

   // std::unique_ptr<TCanvas> canv(new TCanvas("canv", "", 1400, 500));
   // auto canv(new TCanvas("canv", "", 1400, 500));
   // canv->Divide(2, 1);
   // canv->cd(1);
   // hisIn->Draw();
   // canv->cd(2);
   // hisOut->Draw();

   auto nIn = hisIn->GetEntries();
   auto nOut = hisOut->GetEntries();
   auto ratio = (nIn - nOut) / (nIn + nOut);
   // auto ratio = nIn / (nIn + nOut);
   auto err = sqrt(ratio * (1 - ratio) / (nIn + nOut));
   std::cout << ene << "\t" << nIn << "\t" << nOut << "\t" << ratio << "\t"
             << err << std::endl;

   std::unique_ptr<TFile> output(new TFile(outName, "UPDATE"));
   output->cd();
   hisIn->Write(Form("HisIn%2.1f", ene));
   hisOut->Write(Form("HisOut%2.1f", ene));
   output->Close();
  
}

void calRatio()
{
   auto file = new TFile(outName, "RECREATE");
   file->Close();
   delete file;
   
   for (auto ene = 2.3; ene < 20.1; ene += 0.1) {
      GetResults(ene);
   }
}
