#ifndef PMEventAction_h
#define PMEventAction_h 1

#include <G4UserEventAction.hh>

#include "PMHit.hpp"

class PMEventAction : public G4UserEventAction
{
public:
   PMEventAction();
   virtual ~PMEventAction();

   virtual void BeginOfEventAction(const G4Event *);
   virtual void EndOfEventAction(const G4Event *);

private:
   PMHitsCollection *GetHitsCollection(G4int hcID, const G4Event *event) const;
   
   G4int fHitsCollectionID;
};

#endif
