#ifndef PMSD_h
#define PMSD_h 1

#include <G4VSensitiveDetector.hh>
#include <G4ThreeVector.hh>
#include <G4LogicalVolume.hh>

#include "PMHit.hpp"


class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

class PMSD: public G4VSensitiveDetector
{
public:
   PMSD(const G4String &name,
            const G4String &hitsCollectionName);
   virtual ~PMSD();

   virtual void Initialize(G4HCofThisEvent *hce);
   virtual G4bool ProcessHits(G4Step *step, G4TouchableHistory *history);

private:
   PMHitsCollection *fHitsCollection;
};

#endif
