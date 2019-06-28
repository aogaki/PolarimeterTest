#include <G4Event.hh>
#include <G4SDManager.hh>
#include <g4root.hh>

#include "GBSEventAction.hpp"

GBSEventAction::GBSEventAction() : G4UserEventAction(), fHitsCollectionID(-1) {}

GBSEventAction::~GBSEventAction() {}

GBSHitsCollection *GBSEventAction::GetHitsCollection(G4int hcID,
                                                     const G4Event *event) const
{
  GBSHitsCollection *hitsCollection =
      static_cast<GBSHitsCollection *>(event->GetHCofThisEvent()->GetHC(hcID));

  if (!hitsCollection) {
    G4ExceptionDescription msg;
    msg << "Cannot access hitsCollection ID " << hcID;
    // check how to use G4Exception
    G4Exception("GBSEventAction::GetHitsCollection()", "GBSCode0003",
                FatalException, msg);
  }

  return hitsCollection;
}

void GBSEventAction::BeginOfEventAction(const G4Event *) {}

void GBSEventAction::EndOfEventAction(const G4Event *event)
{
  if (fHitsCollectionID == -1)
    fHitsCollectionID = G4SDManager::GetSDMpointer()->GetCollectionID("HC");

  auto anaMan = G4AnalysisManager::Instance();
  auto eveID = event->GetEventID();

  GBSHitsCollection *hc = GetHitsCollection(fHitsCollectionID, event);
  const G4int kHit = hc->entries();
  for (G4int iHit = 0; iHit < kHit; iHit++) {
    GBSHit *newHit = (*hc)[iHit];

    anaMan->FillNtupleIColumn(0, eveID);  // EventID

    auto pdgCode = newHit->GetPDGCode();
    anaMan->FillNtupleIColumn(1, pdgCode);

    auto volumeName = newHit->GetVolumeName();
    anaMan->FillNtupleSColumn(2, volumeName);

    auto depositEnergy = newHit->GetDepositEnergy();
    anaMan->FillNtupleDColumn(3, depositEnergy);

    auto time = newHit->GetTime();
    anaMan->FillNtupleDColumn(4, time);

    auto trackID = newHit->GetTrackID();
    anaMan->FillNtupleIColumn(5, trackID);

    anaMan->AddNtupleRow();
  }
}
