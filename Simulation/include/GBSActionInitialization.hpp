#ifndef GBSActionInitialization_h
#define GBSActionInitialization_h 1

#include <G4VUserActionInitialization.hh>
#include <globals.hh>

#include "GBSParameters.hpp"

class GBSActionInitialization : public G4VUserActionInitialization
{
 public:
  // GBSActionInitialization();
  GBSActionInitialization(SimParameters par);
  virtual ~GBSActionInitialization();

  virtual void BuildForMaster() const;
  virtual void Build() const;

 private:
  SimParameters fSimPar;
};

#endif
