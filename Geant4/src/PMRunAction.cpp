#include <g4root.hh>

#include "PMRunAction.hpp"

PMRunAction::PMRunAction() : G4UserRunAction()
{
  auto anaMan = G4AnalysisManager::Instance();
  // anaMan->SetNtupleMerging(true);
  anaMan->SetVerboseLevel(1);
  G4String fileName = "result";
  anaMan->SetFileName(fileName);

  anaMan->CreateNtuple("PolMeter", "neutron info");

  anaMan->CreateNtupleIColumn("EventID");
  anaMan->CreateNtupleIColumn("TrackID");
  anaMan->CreateNtupleIColumn("ParentID");
  anaMan->CreateNtupleIColumn("PDGCode");
  anaMan->CreateNtupleSColumn("VolName");
  anaMan->CreateNtupleDColumn("Time");
  anaMan->CreateNtupleDColumn("DepositEnergy");

  anaMan->FinishNtuple();
}

PMRunAction::~PMRunAction() {}

void PMRunAction::BeginOfRunAction(const G4Run *)
{
  G4AnalysisManager *anaMan = G4AnalysisManager::Instance();
  anaMan->OpenFile();
}

void PMRunAction::EndOfRunAction(const G4Run *)
{
  G4AnalysisManager *anaMan = G4AnalysisManager::Instance();
  anaMan->Write();
  anaMan->CloseFile();
}
