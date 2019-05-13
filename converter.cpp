#include <iostream>
#include <memory>
#include <string>
#include <fstream>

#include <TFile.h>
#include <TSystem.h>
#include <TTree.h>

#include "./Trace.h"
#include "./WaveFile.h"
#include "./Trace.cpp"
#include "./WaveFile.cpp"

void converter(std::string buf)
{
   constexpr Int_t nFiles = 3;
   WaveFile wave[nFiles];
   Int_t nEvents[nFiles];
   std::string fileName[nFiles] = {
      "data/" + buf.replace(buf.find("-8-"), 3, "-0-"),
      "data/" + buf.replace(buf.find("-0-"), 3, "-1-"),
      "data/" + buf.replace(buf.find("-1-"), 3, "-8-")
   };
  
   for (auto i = 0; i < nFiles; i++) {
      wave[i].Set_Filename(fileName[i]);
      wave[i].Open_File();
      wave[i].Check_For_Header();
      wave[i].Find_Record_Length();
      wave[i].Find_Number_Of_Traces();

      nEvents[i] = wave[i].Get_Num_Traces();
      wave[i].Get_Next_Trace();
   }

   // I asuume number of events of each channels are same.
   // If not, using time stamp and select?  Fucking nightmare
   std::cout << nEvents[0] << "\t" << nEvents[1] << "\t" << nEvents[2]
             << std::endl;

   // ROOT make memory error, if i use unique_ptr
   // std::unique_ptr<TFile> output(new TFile("wave.root", "RECREATE"));
   // std::unique_ptr<TTree> tree(new TTree("wave", "from wavedump"));
   auto output = new TFile("wave.root", "RECREATE");
   auto tree = new TTree("wave", "from wavedump");

   std::vector<short> trace0[nFiles];
   tree->Branch("trace0", &trace0[0]);
   tree->Branch("trace1", &trace0[1]);
   tree->Branch("trace8", &trace0[2]);

   UInt_t time0[nFiles];
   tree->Branch("time0", &time0[0], "time0/i");
   tree->Branch("time1", &time0[1], "time1/i");
   tree->Branch("time8", &time0[2], "time8/i");

   for (auto iEve = 0; iEve < nEvents[0]; iEve++) {
      for (auto iCh = 0; iCh < nFiles; iCh++) {
         trace0[iCh] = wave[iCh].Get_A_Trace_Ptr()->Get_Raw_Trace();
         time0[iCh] = wave[iCh].Get_A_Trace_Ptr()->Get_Trig_Time_Offset();
         wave[iCh].Get_Next_Trace();
      }
      tree->Fill();
   }

   tree->Write();
   output->Close();

   // delete tree;
   // delete output;
     
}
