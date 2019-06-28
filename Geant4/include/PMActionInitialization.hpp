#ifndef PMActionInitialization_h
#define PMActionInitialization_h 1

#include <G4VUserActionInitialization.hh>
#include <globals.hh>

class PMActionInitialization : public G4VUserActionInitialization
{
 public:
  PMActionInitialization(G4double beamEne, G4bool unpolarizedFlag);
  virtual ~PMActionInitialization();

  virtual void BuildForMaster() const;
  virtual void Build() const;

 private:
  G4double fBeamEne;
  G4bool fUnpolarizedFlag;
};

#endif
