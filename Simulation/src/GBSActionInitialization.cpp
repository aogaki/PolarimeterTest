#include "GBSActionInitialization.hpp"
#include "GBSEventAction.hpp"
#include "GBSPrimaryGeneratorAction.hpp"
#include "GBSRunAction.hpp"

GBSActionInitialization::GBSActionInitialization(SimParameters par)
    : G4VUserActionInitialization()
{
  fSimPar = par;
}

GBSActionInitialization::~GBSActionInitialization() {}

void GBSActionInitialization::BuildForMaster() const
{
  SetUserAction(new GBSRunAction());
}

void GBSActionInitialization::Build() const
{
  SetUserAction(new GBSPrimaryGeneratorAction(fSimPar));
  SetUserAction(new GBSRunAction());
  SetUserAction(new GBSEventAction());
}
