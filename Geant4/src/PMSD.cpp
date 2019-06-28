#include <G4HCofThisEvent.hh>
#include <G4Material.hh>
#include <G4SDManager.hh>
#include <G4Step.hh>
#include <G4SystemOfUnits.hh>
#include <G4TouchableHistory.hh>
#include <G4VProcess.hh>
#include <G4ios.hh>
#include <g4root.hh>

#include "PMHit.hpp"
#include "PMSD.hpp"

PMSD::PMSD(const G4String &name, const G4String &hitsCollectionName)
    : G4VSensitiveDetector(name)
{
  collectionName.insert(hitsCollectionName);
}

PMSD::~PMSD() {}

void PMSD::Initialize(G4HCofThisEvent *hce)
{
  fHitsCollection =
      new PMHitsCollection(SensitiveDetectorName, collectionName[0]);

  G4int hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  hce->AddHitsCollection(hcID, fHitsCollection);
}

G4bool PMSD::ProcessHits(G4Step *step, G4TouchableHistory * /*history*/)
{
  auto newHit = new PMHit();

  auto preStepPoint = step->GetPreStepPoint();
  auto volumeName = preStepPoint->GetPhysicalVolume()->GetName();
  newHit->SetVolumeName(volumeName);

  auto track = step->GetTrack();
  auto particle = track->GetDefinition();
  auto pdgCode = particle->GetPDGEncoding();
  newHit->SetPDGCode(pdgCode);

  auto trackID = track->GetTrackID();
  newHit->SetTrackID(trackID);
  auto parentID = track->GetParentID();
  newHit->SetParentID(parentID);

  auto depositEnergy = step->GetTotalEnergyDeposit();
  newHit->SetDepositEnergy(depositEnergy);

  auto postStepPoint = step->GetPostStepPoint();
  auto time = postStepPoint->GetGlobalTime();
  newHit->SetTime(time);

  fHitsCollection->insert(newHit);
  return true;
}
