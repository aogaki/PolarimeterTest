#include <iostream>

#include "TPolarimeter.hpp"

TPolarimeter::TPolarimeter()
{
  fCanvas.reset(new TCanvas("canv", "Trigger and signal", 700, 1000));
  fCanvas->Divide(1, 2);

  fTriggerSignal.reset(new TGraph());
  fTriggerSignal->SetLineWidth(2);
  fTriggerSignal->SetLineColor(kBlack);

  fSignal1.reset(new TGraph());
  fSignal1->SetLineWidth(2);
  fSignal1->SetLineColor(kRed);

  fSignal2.reset(new TGraph());
  fSignal2->SetLineWidth(2);
  fSignal2->SetLineColor(kBlue);

  fTriggerSignal->SetMinimum(0);
  fTriggerSignal->SetMaximum(18000);
}

TPolarimeter::~TPolarimeter() {}

void TPolarimeter::SetSignals(vector<short> &trigger, vector<short> &signal1,
                              vector<short> &signal2)
{
  // This will be bottleneck?
  const auto size = trigger.size();
  fTriggerSignal->Set(size);
  fSignal1->Set(size);
  fSignal2->Set(size);

  constexpr Int_t deltaT = 1;
  for (unsigned int i = 0; i < size; i++) {
    fTriggerSignal->SetPoint(i, i * deltaT, trigger[i]);
    fSignal1->SetPoint(i, i * deltaT, signal1[i]);
    fSignal2->SetPoint(i, i * deltaT, signal2[i]);
  }
}

void TPolarimeter::Plot()
{
  fCanvas->cd(1);
  fTriggerSignal->Draw("AL");
  fSignal1->Draw("SAME");

  fCanvas->cd(2);
  fTriggerSignal->Draw("AL");
  fSignal2->Draw("SAME");
  // fTriggerSignal[1]->Draw("AL");
}
