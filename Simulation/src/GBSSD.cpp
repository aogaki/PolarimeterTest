#include <G4HCofThisEvent.hh>
#include <G4Material.hh>
#include <G4SDManager.hh>
#include <G4Step.hh>
#include <G4SystemOfUnits.hh>
#include <G4TouchableHistory.hh>
#include <G4VProcess.hh>
#include <G4ios.hh>
#include <g4root.hh>

#include "GBSHit.hpp"
#include "GBSSD.hpp"

GBSSD::GBSSD(const G4String &name, const G4String &hitsCollectionName)
    : G4VSensitiveDetector(name)
{
  collectionName.insert(hitsCollectionName);
}

GBSSD::~GBSSD() {}

void GBSSD::Initialize(G4HCofThisEvent *hce)
{
  fHitsCollection =
      new GBSHitsCollection(SensitiveDetectorName, collectionName[0]);

  G4int hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  hce->AddHitsCollection(hcID, fHitsCollection);
}

G4bool GBSSD::ProcessHits(G4Step *step, G4TouchableHistory * /*history*/)
{
  G4double depositEnergy = step->GetTotalEnergyDeposit();
  if (!(depositEnergy > 0.)) return false;
  auto newHit = new GBSHit;
  newHit->SetDepositEnergy(depositEnergy);

  G4StepPoint *preStepPoint = step->GetPreStepPoint();
  G4String volumeName = preStepPoint->GetPhysicalVolume()->GetName();
  newHit->SetVolumeName(volumeName);

  G4Track *track = step->GetTrack();
  G4ParticleDefinition *particle = track->GetDefinition();
  G4int pdgCode = particle->GetPDGEncoding();
  newHit->SetPDGCode(pdgCode);

  G4int trackID = track->GetTrackID();
  newHit->SetTrackID(trackID);

  G4StepPoint *postStepPoint = step->GetPostStepPoint();
  G4double time = postStepPoint->GetGlobalTime();
  newHit->SetTime(time);

  fHitsCollection->insert(newHit);
  return true;
}
