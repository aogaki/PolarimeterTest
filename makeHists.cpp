#include <iostream>
#include <string>
#include <vector>

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TTree.h>
#include <TStyle.h>

#include "TBeamSignal.cpp"
#include "TBeamSignal.hpp"
#include "TPolarimeter.cpp"
#include "TPolarimeter.hpp"
#include "TSignal.cpp"
#include "TSignal.hpp"

TTree *tree;
TPolarimeter *polMeter;
TSignal *wave1;
TSignal *wave2;
TBeamSignal *beam;
std::vector<short> *trace[3]{nullptr};
Int_t counter = 0;

TH1D *HisShort1;
TH1D *HisLong1;
TH1D *HisTOF1;
TH1D *HisPS1;
TH2D *HisTimeLong1;
TH2D *HisTimeShort1;
TH2D *HisLongShort1;

TH1D *HisShort2;
TH1D *HisLong2;
TH1D *HisTOF2;
TH1D *HisPS2;
TH2D *HisTimeLong2;
TH2D *HisTimeShort2;
TH2D *HisLongShort2;

void GoNext()
{
  tree->GetEntry(counter++);

  polMeter->SetSignals(*trace[2], *trace[0], *trace[1]);
  polMeter->Plot();

  wave1->ProcessSignal();
  wave1->Plot();

  wave2->ProcessSignal();
  wave2->Plot();

  beam->ProcessSignal();
  beam->Plot();
}

void InitHists()
{
  HisLong1 = new TH1D("HisLong1", "Long gate 1", 6000, 0., 60000.);
  HisShort1 = new TH1D("HisShort1", "Short gate 1", 6000, 0., 60000.);
  HisTOF1 = new TH1D("HisTOF1", "TOF 1", 10000, 0., 100.);
  HisPS1 = new TH1D("HisPS1", "PS 1", 1100, 0., 1.1);

  HisLong2 = new TH1D("HisLong2", "Long gate 2", 6000, 0., 60000.);
  HisShort2 = new TH1D("HisShort2", "Short gate 2", 6000, 0., 60000.);
  HisTOF2 = new TH1D("HisTOF2", "TOF 2", 10000, 0., 100.);
  HisPS2 = new TH1D("HisPS2", "PS 2", 1100, 0., 1.1);

  HisTimeLong1 = new TH2D("HisTimeLong1", "Long gate charge 1", 1000, 0., 100.,
                          6000, 0., 60000.);
  HisTimeShort1 = new TH2D("HisTimeShort1", "Short gate charge 1", 1000, 0.,
                           100., 1000, 0., 1.);
  HisLongShort1 = new TH2D("HisLongShort1", "Long and shortgate charge 1", 600,
                           0., 60000., 100, 0., 1.);

  HisTimeLong2 = new TH2D("HisTimeLong2", "Long gate charge 2", 1000, 0., 100.,
                          6000, 0., 60000.);
  HisTimeShort2 = new TH2D("HisTimeShort2", "Short gate charge 2", 1000, 0.,
                           100., 1000, 0., 1.);
  HisLongShort2 = new TH2D("HisLongShort2", "Long and shortgate charge 2", 600,
                           0., 60000., 100, 0., 1.);
}

void makeHists(TString fileName = "Data/wave6.root", Int_t shortGate = 20)
{
  InitHists();

  auto file = new TFile(fileName, "READ");
  tree = (TTree *)file->Get("wave");
  tree->SetBranchStatus("*", kFALSE);

  tree->SetBranchStatus("trace0", kTRUE);
  tree->SetBranchAddress("trace0", &trace[0]);
  tree->SetBranchStatus("trace1", kTRUE);
  tree->SetBranchAddress("trace1", &trace[1]);
  tree->SetBranchStatus("trace8", kTRUE);
  tree->SetBranchAddress("trace8", &trace[2]);

  polMeter = new TPolarimeter();

  wave1 = new TSignal(trace[0], 500, shortGate, 150);
  wave2 = new TSignal(trace[1], 500, shortGate, 150);
  beam = new TBeamSignal(trace[2]);

  const int nHit = tree->GetEntries();
  for (auto iHit = 0; iHit < nHit; iHit++) {
    tree->GetEntry(iHit);
    wave1->ProcessSignal();
    wave2->ProcessSignal();
    beam->ProcessSignal();

    auto beamTrg = beam->GetTrgTime();

    auto long1 = wave1->GetLongCharge();
    auto short1 = wave1->GetShortCharge();
    auto ps1 = short1 / long1;
    constexpr auto timeOffset1 = 0.;
    auto tof1 = wave1->GetTrgTime() - beamTrg + timeOffset1;

    auto long2 = wave2->GetLongCharge();
    auto short2 = wave2->GetShortCharge();
    auto ps2 = short2 / long2;
    constexpr auto timeOffset2 = 10.14 + 1.04;  // Check Aogaki
    auto tof2 = wave2->GetTrgTime() - beamTrg + timeOffset2;

    // std::cout << ps1 << "\t" << ps2 << std::endl;
    if (tof1 > 0.) {
      HisLong1->Fill(long1);
      HisShort1->Fill(short1);
      // HisTOF1->Fill(tof1);
      if (ps1 < 0.71) HisTOF1->Fill(tof1);
      HisPS1->Fill(ps1);

      HisTimeLong1->Fill(tof1, long1);
      HisTimeShort1->Fill(tof1, ps1);
      HisLongShort1->Fill(long1, ps1);
    }
    if (tof2 > 0.) {
      HisLong2->Fill(long2);
      HisShort2->Fill(short2);
      // HisTOF2->Fill(tof2);
      if (ps2 < 0.78) HisTOF2->Fill(tof2);
      HisPS2->Fill(ps2);

      HisTimeLong2->Fill(tof2, long2);
      HisTimeShort2->Fill(tof2, ps2);
      HisLongShort2->Fill(long2, ps2);
    }
  }

  // HisLongShort2->Draw("COLZ");
  HisTOF2->Draw();
  HisTOF1->Draw("SAME");

  auto output = new TFile("hists.root", "RECREATE");
  HisTimeShort1->Write("HistOut");
  HisTimeShort2->Write("HistIn");
  output->Close();
  // delete output;
}
