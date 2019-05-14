#include <iostream>

#include <TROOT.h>
#include <TString.h>

#include "TAsymmetry.hpp"

TAsymmetry::TAsymmetry()
{
  fPulseShapeTh[0] = 0.;
  fPulseShapeTh[1] = 0.;
  fTimeTh[0] = 38.;
  fTimeTh[1] = 0.;

  fIndex = 0;
}

TAsymmetry::TAsymmetry(TH2 *hist, int index) : TAsymmetry()
{
  fIndex = index;
  fHist.reset((TH2D *)hist->Clone(Form("Hist2D%02d", fIndex)));
  fHist->SetTitle("PS vs TOF");

  fHistTime.reset((TH1D *)hist->ProjectionX(Form("HistTime%02d", fIndex)));
  fHistTime->SetTitle("TOF");

  fHistPS.reset((TH1D *)hist->ProjectionY(Form("HistPS%02d", fIndex)));
  fHistPS->SetTitle("PS");
}

TAsymmetry::~TAsymmetry() {}

template <typename T>
void TAsymmetry::SetPosition(T &obj, double x1, double y1, double x2, double y2)
{
  obj->SetX1(x1);
  obj->SetY1(y1);
  obj->SetX2(x2);
  obj->SetY2(y2);
}

void TAsymmetry::Plot()
{
  if (!fCanvas) {
    fCanvas.reset(
        new TCanvas(Form("glcanvas%02d", fIndex), "PS and TOF", 1400, 1000));
    fCanvas->Divide(2, 2);
  }
  if (!fVerLine) {
    fVerLine.reset(new TLine());
    fVerLine->SetLineWidth(2);
    fVerLine->SetLineColor(kRed);
    fVerLine->SetLineStyle(10);
  }
  if (!fSpectrum) {
    constexpr auto nPeaks = 1;
    fSpectrum.reset(new TSpectrum(nPeaks));
  }

  if (fHist) {
    fCanvas->cd(1);
    fHist->Draw("COLZ");
    fCanvas->cd(1)->SetLogz();
  }
  if (fHistPS) {
    fCanvas->cd(2);
    fHistPS->Draw();
  }
  if (fHistTime) {
    fCanvas->cd(3);
    fHistTime->Draw();
  }
  if (fHistResult) {
    fCanvas->cd(4);
    // fHistResult->Draw();
    fSpectrum->Search(fHistResult.get());
  }
}

void TAsymmetry::DataAnalysis()
{
  PulseShapeCut();
  TimeCut();
}

void TAsymmetry::TimeCut()
{
  if (!fHistResult) PulseShapeCut();

  constexpr auto thRatio = 0.01;
  const auto maxBin = fHistResult->GetMaximumBin();
  const auto max = fHistResult->GetBinContent(maxBin);
  const auto th = max * thRatio;
  const auto nBins = fHistResult->GetNbinsX();

  for (auto i = maxBin; i < nBins; i++) {
    auto binContent = fHistResult->GetBinContent(i);
    if (binContent < th) {
      fHistResult->GetXaxis()->SetRange(i, nBins);
      std::cout << i << std::endl;
      break;
    }
  }
}

void TAsymmetry::PulseShapeCut()
{
  constexpr auto thRatio = 0.075;
  const auto maxBin = fHistPS->GetMaximumBin();
  const auto max = fHistPS->GetBinContent(maxBin);
  const auto th = max * thRatio;

  for (auto i = maxBin; i > 0; i--) {
    auto binContent = fHistPS->GetBinContent(i);
    if (binContent < th) {
      fHistResult.reset(
          fHist->ProjectionX(Form("HistResult%02d", fIndex), 1, i));
      fHistResult->Rebin(4);
      std::cout << i << std::endl;
      break;
    }
  }
}

double TAsymmetry::Integral()
{
  constexpr auto window = 4.;
  const auto peak = fSpectrum->GetPositionX()[0];
  const auto startBin = fHistResult->FindBin(peak - window / 2.);
  const auto stopBin = fHistResult->FindBin(peak + window / 2.);

  return fHistResult->Integral(startBin, stopBin);
}
