#ifndef GBSEventAction_h
#define GBSEventAction_h 1

#include <G4UserEventAction.hh>

#include "GBSHit.hpp"

class GBSEventAction : public G4UserEventAction
{
public:
   GBSEventAction();
   virtual ~GBSEventAction();

   virtual void BeginOfEventAction(const G4Event *);
   virtual void EndOfEventAction(const G4Event *);

private:
   GBSHitsCollection *GetHitsCollection(G4int hcID, const G4Event *event) const;
   
   G4int fHitsCollectionID;
};

#endif
