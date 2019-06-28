#ifndef GBSSD_h
#define GBSSD_h 1

#include <G4VSensitiveDetector.hh>
#include <G4ThreeVector.hh>
#include <G4LogicalVolume.hh>

#include "GBSHit.hpp"


class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

class GBSSD: public G4VSensitiveDetector
{
public:
   GBSSD(const G4String &name,
            const G4String &hitsCollectionName);
   virtual ~GBSSD();

   virtual void Initialize(G4HCofThisEvent *hce);
   virtual G4bool ProcessHits(G4Step *step, G4TouchableHistory *history);

private:
   GBSHitsCollection *fHitsCollection;
};

#endif
