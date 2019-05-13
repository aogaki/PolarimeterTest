#include <string>
#include <iostream>
#include <vector>
#include <fcntl.h>
#include <termios.h>

#include <TSystem.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TTree.h>
#include <TH2.h>

TH2D *his0;
TH2D *his1;
TH2D *his8;


int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if (ch != EOF) {
    ungetc(ch, stdin);
    ch = EOF;
    return 1;
  }

  return 0;
}

void analysis()
{
   auto file = new TFile("wave.root", "READ");
   auto tree = (TTree*)file->Get("wave");
   tree->SetBranchStatus("*", kFALSE);
   
   std::vector<short> *trace[3]{nullptr};
   tree->SetBranchStatus("trace0", kTRUE);
   tree->SetBranchAddress("trace0", &trace[0]);
   tree->SetBranchStatus("trace1", kTRUE);
   tree->SetBranchAddress("trace1", &trace[1]);
   tree->SetBranchStatus("trace8", kTRUE);
   tree->SetBranchAddress("trace8", &trace[2]);


   his0 = new TH2D("his0", "test", 250, 0., 250., 18000, 0., 18000.);
   his1 = new TH2D("his1", "test", 250, 0., 250., 18000, 0., 18000.);
   his8 = new TH2D("his8", "test", 250, 0., 250., 18000, 0., 18000.);

   auto canvas = new TCanvas();
   
   const Int_t nEve = tree->GetEntries();
   for(auto iEve = 0; iEve < nEve; iEve++){
      tree->GetEntry(iEve);
      for(auto i = 0; i < 250; i++){
         his0->Fill(i, (*trace[0])[i]);
         his1->Fill(i, (*trace[1])[i]);
         his8->Fill(i, (*trace[2])[i]);
      }
   }

}
