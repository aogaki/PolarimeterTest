#ifndef GBSPrimaryGeneratorAction_h
#define GBSPrimaryGeneratorAction_h 1

#include <map>
#include <memory>

#include <TF2.h>
#include <TH2.h>

#include <G4Event.hh>
#include <G4GenericMessenger.hh>
#include <G4ParticleGun.hh>
#include <G4Threading.hh>
#include <G4ThreeVector.hh>
#include <G4VUserPrimaryGeneratorAction.hh>

#include "GBSParameters.hpp"

class GBSPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
 public:
  GBSPrimaryGeneratorAction(SimParameters par);
  virtual ~GBSPrimaryGeneratorAction();

  virtual void GeneratePrimaries(G4Event *);

 private:
  SimParameters fSimPar;

  std::unique_ptr<G4ParticleGun> fParticleGun;
  std::unique_ptr<TF2> fAngGenerator;
  std::unique_ptr<TF1> fEneGenerator;

  G4ThreeVector GetPosition();
  G4ThreeVector GetDirection();
  G4double fDivergence;

  void SetParameters();
  std::map<GBSLabel, GunParameters> fParameters;
};

#endif
