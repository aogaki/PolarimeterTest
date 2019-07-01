#include <memory>

#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>
#include <TString.h>
#include <TTree.h>

void calRatio(Double_t ene = 15.1)
{
  auto fileName = Form("pol%2.1f.root", ene);
  auto file = new TFile(fileName, "RRAD");
  auto tree = (TTree *)file->Get("PolMeter");
  tree->SetBranchStatus("*", kFALSE);

  Int_t eventID;
  tree->SetBranchStatus("EventID", kTRUE);
  tree->SetBranchAddress("EventID", &eventID);

  Double_t depEne;
  tree->SetBranchStatus("DepositEnergy", kTRUE);
  tree->SetBranchAddress("DepositEnergy", &depEne);

  Char_t volName[16];
  tree->SetBranchStatus("VolName", kTRUE);
  tree->SetBranchAddress("VolName", volName);

  // std::unique_ptr<TH1D> hisIn(new TH1D("hisIn", "In plane", 2000, 0., 20.));
  auto hisIn(new TH1D("hisIn", "In plane", 2000, 0., 20.));
  hisIn->SetXTitle("Deposit energy [MeV]");
  // std::unique_ptr<TH1D> hisOut(new TH1D("hisOut", "Out plane", 2000, 0., 20.));
  auto hisOut(new TH1D("hisOut", "Out plane", 2000, 0., 20.));
  hisOut->SetXTitle("Deposit energy [MeV]");

  const Int_t nHit = tree->GetEntries();
  auto inEne = 0.;
  auto outEne = 0.;
  tree->GetEntry(0);
  auto currentEveID = eventID;
  for (auto iHit = 0; iHit < nHit; iHit++) {
    tree->GetEntry(iHit);
    auto detName = TString(volName);
    if (currentEveID == eventID) {
      if (detName == "Detector1")
        inEne += depEne;
      else
        outEne += depEne;
    } else {
      if (inEne > 0.) hisIn->Fill(inEne);
      if (outEne > 0.) hisOut->Fill(outEne);
      if (detName == "Detector1") {
        inEne = depEne;
        outEne = 0.;
      } else {
        inEne = 0.;
        outEne = depEne;
      }
    }
  }

  // std::unique_ptr<TCanvas> canv(new TCanvas("canv", "", 1400, 500));
  auto canv(new TCanvas("canv", "", 1400, 500));
  canv->Divide(2, 1);
  canv->cd(1);
  hisIn->Draw();
  canv->cd(2);
  hisOut->Draw();
}
