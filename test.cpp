#include <math.h>
#include <iostream>

#include <TFile.h>
#include <TH2.h>
#include <TString.h>

#include "TAsymmetry.cpp"
#include "TAsymmetry.hpp"

TH2D *histIn;
TH2D *histOut;

void test(TString fileName = "Data/hists_wave9.root")
{
  auto file = new TFile(fileName, "READ");

  histIn = (TH2D *)file->Get("HistIn");
  auto inPlane = new TAsymmetry(histIn, 0);
  inPlane->DataAnalysis();
  inPlane->Plot();
  auto yieldIn = inPlane->GetYield();

  histOut = (TH2D *)file->Get("HistOut");
  auto outPlane = new TAsymmetry(histOut, 1);
  outPlane->DataAnalysis();
  outPlane->Plot();
  auto yieldOut = outPlane->GetYield();

  std::cout << yieldOut << "\t" << yieldIn << "\t"
            << fabs(yieldIn - yieldOut) / (yieldIn + yieldOut) << std::endl;
}
