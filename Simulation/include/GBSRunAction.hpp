#ifndef GBSRunAction_h
#define GBSRunAction_h 1

#include <G4UserRunAction.hh>
#include <G4Run.hh>


class GBSRunAction: public G4UserRunAction
{
public:
   GBSRunAction();
   virtual ~GBSRunAction();

   virtual void BeginOfRunAction(const G4Run *);
   virtual void EndOfRunAction(const G4Run *);

private:

};

#endif
