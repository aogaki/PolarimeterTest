#include "TAsymmetry.hpp"

TAsymmetry::TAsymmetry()
{
  fPulseShapeTh[0] = 0.;
  fPulseShapeTh[1] = 0.;
  fTimeTh[0] = 0.;
  fTimeTh[1] = 0.;
}

TAsymmetry::TAsymmetry(TH2 *hist) : TAsymmetry()
{
  fHist.reset((TH2D *)(hist->Clone()));
}

TAsymmetry::~TAsymmetry() {}

TH1D *TAsymmetry::GetTOF()
{
  auto startBin = fHist->FindBin(100., fPulseShapeTh[0]);
  auto lastBin = fHist->FindBin(100., fPulseShapeTh[1]);
  return (TH1D *)(fHist->ProjectionX("", startBin, lastBin));
}

void TAsymmetry::Plot()
{
  if (!fCanvas) {
    fCanvas.reset(new TCanvas("canvas", "PS and TOF", 1400, 1000));
    fCanvas->Divide(2, 2);
  }

  fCanvas->cd(1);
  if (fHist) {
    fHist->Draw("COLZ");
  }
}

void TAsymmetry::DataAnalysis()
{
  TimeCut();
  PulseShapeCut();
}
