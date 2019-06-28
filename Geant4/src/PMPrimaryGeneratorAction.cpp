#include <random>

#include <TFile.h>
#include <TRandom3.h>

#include <G4AutoLock.hh>
#include <G4ChargedGeantino.hh>
#include <G4IonTable.hh>
#include <G4ParticleTable.hh>
#include <G4SystemOfUnits.hh>
#include <Randomize.hh>

#include <g4root.hh>
#include "PMPrimaryGeneratorAction.hpp"

static G4int nEveInPGA = 0;
G4Mutex mutexConst = G4MUTEX_INITIALIZER;
G4Mutex mutexGen = G4MUTEX_INITIALIZER;
G4Mutex mutexCounter = G4MUTEX_INITIALIZER;

PMPrimaryGeneratorAction::PMPrimaryGeneratorAction(G4double beamEne,
                                                   G4bool unpolarizedFlag)
    : G4VUserPrimaryGeneratorAction(), fParticleGun(nullptr)
{
  fUnpolarizedFlag = unpolarizedFlag;
  fGammaEne = beamEne;
  fGammaSigma = beamEne * 0.005 / (2 * sqrt(2 * log(2)));

  fParticleGun = new G4ParticleGun(1);
  auto particleTable = G4ParticleTable::GetParticleTable();
  auto particle = particleTable->FindParticle("neutron");
  fParticleGun->SetParticleDefinition(particle);

  G4AutoLock lock(&mutexConst);
  fAngDist.reset(new TF2("angdist",
                         "sin(x)*([0]+[1]*pow(sin(x),2)+[2]*pow(sin(2*x),2)-"
                         "cos(2*y)*([1]*pow(sin(x),2)+[2]*pow(sin(2*x),2)))",
                         0, CLHEP::pi, 0, CLHEP::pi2));
  fAngDist->SetParameter(0, 0.01);
  fAngDist->SetParameter(1, 0.99);
  fAngDist->SetParameter(2, 0.);
  fAngDist->SetNpx(1000);
  fAngDist->SetNpy(1000);

  std::random_device rndSeed;
  gRandom->SetSeed(rndSeed());

  auto file = new TFile("KEneAng.root", "READ");
  fKEneAng.reset((TGraph2D *)file->Get("KEneAng"));
  fKEneAng->SetDirectory(0);
  file->Close();
  delete file;
}

PMPrimaryGeneratorAction::~PMPrimaryGeneratorAction() { delete fParticleGun; }

void PMPrimaryGeneratorAction::GenBeam()
{
  G4AutoLock lock(&mutexGen);
  fAngDist->GetRandom2(fBeamTheta, fBeamPhi);
  fBeamEne = fKEneAng->Interpolate(fBeamTheta * 180 / CLHEP::pi,
                                   gRandom->Gaus(fGammaEne, fGammaSigma));
}

void PMPrimaryGeneratorAction::GeneratePrimaries(G4Event *event)
{
  auto beamR = 1. * sqrt(G4UniformRand());  // Uniform in 2cm dia
  auto posTheta = G4UniformRand() * CLHEP::pi2;
  auto beamPos =
      G4ThreeVector(beamR * cos(posTheta) * cm, beamR * sin(posTheta) * cm,
                    G4UniformRand() * 3.7 - 3.7 / 2.0);
  // GenBeam();
  fAngDist->GetRandom2(fBeamTheta, fBeamPhi);
  fBeamEne = fKEneAng->Interpolate(fBeamTheta * 180 / CLHEP::pi,
                                   gRandom->Gaus(fGammaEne, fGammaSigma));
  if (fUnpolarizedFlag) fBeamPhi = G4UniformRand() * CLHEP::pi2;

  auto beamP = G4ThreeVector(sin(fBeamTheta) * cos(fBeamPhi),
                             sin(fBeamTheta) * sin(fBeamPhi), cos(fBeamTheta));

  fParticleGun->SetParticleEnergy(fBeamEne);
  fParticleGun->SetParticlePosition(beamPos);
  fParticleGun->SetParticleMomentumDirection(beamP);
  fParticleGun->GeneratePrimaryVertex(event);

  G4AutoLock lock(&mutexCounter);
  if (nEveInPGA++ % 10000 == 0)
    G4cout << nEveInPGA - 1 << " events done" << G4endl;
}
