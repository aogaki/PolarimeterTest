#include "PMActionInitialization.hpp"
#include "PMEventAction.hpp"
#include "PMPrimaryGeneratorAction.hpp"
#include "PMRunAction.hpp"

PMActionInitialization::PMActionInitialization(G4double beamEne,
                                               G4bool unpolarizedFlag)
    : G4VUserActionInitialization()
{
  fBeamEne = beamEne;
  fUnpolarizedFlag = unpolarizedFlag;
}

PMActionInitialization::~PMActionInitialization() {}

void PMActionInitialization::BuildForMaster() const
{
  SetUserAction(new PMRunAction());
}

void PMActionInitialization::Build() const
{
  SetUserAction(new PMPrimaryGeneratorAction(fBeamEne, fUnpolarizedFlag));
  SetUserAction(new PMRunAction());
  SetUserAction(new PMEventAction());
}
