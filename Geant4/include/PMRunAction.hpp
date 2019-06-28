#ifndef PMRunAction_h
#define PMRunAction_h 1

#include <G4UserRunAction.hh>
#include <G4Run.hh>


class PMRunAction: public G4UserRunAction
{
public:
   PMRunAction();
   virtual ~PMRunAction();

   virtual void BeginOfRunAction(const G4Run *);
   virtual void EndOfRunAction(const G4Run *);

private:

};

#endif
