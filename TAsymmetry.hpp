#ifndef TASYMMETRY_HPP
#define TASYMMETRY_HPP 1

#include <memory>

#include <TBox.h>
#include <TCanvas.h>
#include <TH2.h>
#include <TLine.h>
#include <TSpectrum.h>

class TAsymmetry
{
 public:
  TAsymmetry();
  TAsymmetry(TH2 *hist, int index = 0);
  ~TAsymmetry();

  void SetHist(TH2 *hist) { fHist.reset((TH2D *)(hist->Clone())); };

  void Plot();

  void DataAnalysis();

  double GetYield() { return Integral(); };

 private:
  int fIndex;

  std::unique_ptr<TH2D> fHist;
  std::unique_ptr<TH1D> fHistTime;
  std::unique_ptr<TH1D> fHistPS;
  std::unique_ptr<TH1D> fHistResult;
  std::unique_ptr<TCanvas> fCanvas;
  std::unique_ptr<TSpectrum> fSpectrum;

  template <typename T>
  void SetPosition(T &obj, double x1, double y1, double x2, double y2);

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
