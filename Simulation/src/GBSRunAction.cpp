#include <g4root.hh>

#include "GBSRunAction.hpp"

GBSRunAction::GBSRunAction() : G4UserRunAction()
{
  auto anaMan = G4AnalysisManager::Instance();
  // anaMan->SetNtupleMerging(true);
  anaMan->SetVerboseLevel(1);
  G4String fileName = "result";
  anaMan->SetFileName(fileName);

  anaMan->CreateNtuple("Polarization", "particle info");

  anaMan->CreateNtupleIColumn("EventID");
  anaMan->CreateNtupleIColumn("PDGCode");
  anaMan->CreateNtupleSColumn("VolumeName");
  anaMan->CreateNtupleDColumn("DepositEnergy");
  anaMan->CreateNtupleDColumn("Time");
  anaMan->CreateNtupleIColumn("TrackID");

  anaMan->FinishNtuple();
}

GBSRunAction::~GBSRunAction() { delete G4AnalysisManager::Instance(); }

void GBSRunAction::BeginOfRunAction(const G4Run *)
{
  G4AnalysisManager *anaMan = G4AnalysisManager::Instance();
  anaMan->OpenFile();
}

void GBSRunAction::EndOfRunAction(const G4Run *)
{
  G4AnalysisManager *anaMan = G4AnalysisManager::Instance();
  anaMan->Write();
  anaMan->CloseFile();
}
