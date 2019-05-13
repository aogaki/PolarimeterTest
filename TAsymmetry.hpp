#ifndef TASYMMETRY_HPP
#define TASYMMETRY_HPP 1

#include <memory>

#include <TBox.h>
#include <TCanvas.h>
#include <TH2.h>
#include <TLine.h>

class TAsymmetry
{
 public:
  TAsymmetry();
  TAsymmetry(TH2 *hist);
  ~TAsymmetry();

  void SetHist(TH2 *hist) { fHist.reset((TH2D *)(hist->Clone())); };

  void Plot();

  void DataAnalysis();

  TH1D *GetTOF();
  double GetYield() { return Integral(); };

 private:
  std::unique_ptr<TH2D> fHist;
  std::unique_ptr<TCanvas> fCanvas;

  void PulseShapeCut();
  double fPulseShapeTh[2];  // 0: start, 1: end
  std::unique_ptr<TLine> fHorLine;

  void TimeCut();
  double fTimeTh[2];  // 0: start, 1: end
  std::unique_ptr<TLine> fVerLine;

  double Integral();
  std::unique_ptr<TBox> fArea;
};

#endif
