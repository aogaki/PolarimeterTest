#ifndef TPOLARYMETER_HPP
#define TPOLARYMETER_HPP 1

#include <memory>
#include <vector>

#include <TCanvas.h>
#include <TGraph.h>

using std::vector;

class TPolarimeter
{
 public:
  TPolarimeter();
  ~TPolarimeter();

  void Plot();

  void SetSignals(vector<short> &trigger, vector<short> &signal1,
                  vector<short> &signal2);
  void SetThLED(int val) { fThLED = val; };
  void SetThCFD(int val) { fThCFD = val; };

 private:
  // To plot the results [0]
  // To calculate the trigger time to use TGraph::Eval() [1]
  std::unique_ptr<TGraph> fTriggerSignal;
  std::unique_ptr<TGraph> fSignal1;
  std::unique_ptr<TGraph> fSignal2;
  int fThLED;
  int fThCFD;
  int fThTrg;

  std::unique_ptr<TCanvas> fCanvas;
};

#endif
