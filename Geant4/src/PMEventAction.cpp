#include <G4Event.hh>
#include <G4SDManager.hh>
#include <g4root.hh>

#include "PMEventAction.hpp"

PMEventAction::PMEventAction() : G4UserEventAction(), fHitsCollectionID(-1) {}

PMEventAction::~PMEventAction() {}

PMHitsCollection *PMEventAction::GetHitsCollection(G4int hcID,
                                                   const G4Event *event) const
{
  PMHitsCollection *hitsCollection =
      static_cast<PMHitsCollection *>(event->GetHCofThisEvent()->GetHC(hcID));

  if (!hitsCollection) {
    G4ExceptionDescription msg;
    msg << "Cannot access hitsCollection ID " << hcID;
    // check how to use G4Exception
    G4Exception("PMEventAction::GetHitsCollection()", "PMCode0003",
                FatalException, msg);
  }

  return hitsCollection;
}

void PMEventAction::BeginOfEventAction(const G4Event *) {}

void PMEventAction::EndOfEventAction(const G4Event *event)
{
  if (fHitsCollectionID == -1)
    fHitsCollectionID = G4SDManager::GetSDMpointer()->GetCollectionID("HC");
  PMHitsCollection *hc = GetHitsCollection(fHitsCollectionID, event);

  auto anaMan = G4AnalysisManager::Instance();
  auto eveID = event->GetEventID();

  const G4int kHit = hc->entries();
  for (G4int iHit = 0; iHit < kHit; iHit++) {
    PMHit *newHit = (*hc)[iHit];

    auto trackID = newHit->GetTrackID();
    auto parentID = newHit->GetParentID();
    auto pdg = newHit->GetPDGCode();
    auto volName = newHit->GetVolumeName();
    auto time = newHit->GetTime();
    auto ene = newHit->GetDepositEnergy();

    anaMan->FillNtupleIColumn(0, eveID);
    anaMan->FillNtupleIColumn(1, trackID);
    anaMan->FillNtupleIColumn(2, parentID);
    anaMan->FillNtupleIColumn(3, pdg);
    anaMan->FillNtupleSColumn(4, volName);
    anaMan->FillNtupleDColumn(5, time);
    anaMan->FillNtupleDColumn(6, ene);

    anaMan->AddNtupleRow();
  }
}
