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

#include "GBSPrimaryGeneratorAction.hpp"

static G4int nEveInPGA = 0;
G4Mutex mutexInPGA = G4MUTEX_INITIALIZER;

GBSPrimaryGeneratorAction::GBSPrimaryGeneratorAction(SimParameters par)
    : G4VUserPrimaryGeneratorAction(), fParticleGun(nullptr)
{
  fSimPar = par;
  SetParameters();

  fParticleGun.reset(new G4ParticleGun(1));
  auto parTable = G4ParticleTable::GetParticleTable();
  auto particle = parTable->FindParticle("neutron");
  fParticleGun->SetParticleDefinition(particle);
  fParticleGun->SetParticlePosition(G4ThreeVector(0.0, 0.0, 0.0));
  fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0, 0, 1));

  std::random_device rndSeed;
  gRandom->SetSeed(rndSeed());
  G4AutoLock lock(&mutexInPGA);

  fAngGenerator.reset(
      new TF2("angdist",
              "sin(x)*([0]+[1]*pow(sin(x),2)+[2]*pow(sin(2*x),2)-cos(2*y)"
              "*([1]*pow(sin(x),2)+[2]*pow(sin(2*x),2)))",
              0, TMath::Pi(), 0, TMath::TwoPi()));
  fAngGenerator->SetParameter(0, fParameters[fSimPar.Label].AngPar[0]);
  fAngGenerator->SetParameter(1, fParameters[fSimPar.Label].AngPar[1]);
  fAngGenerator->SetParameter(2, fParameters[fSimPar.Label].AngPar[2]);
  fAngGenerator->SetNpx(1000);
  fAngGenerator->SetNpy(1000);

  fEneGenerator.reset(new TF1("fEneGenerator", "pol3", 0, 180));
  fEneGenerator->SetParameter(0, fParameters[fSimPar.Label].EnePar[0]);
  fEneGenerator->SetParameter(1, fParameters[fSimPar.Label].EnePar[1]);
  fEneGenerator->SetParameter(2, fParameters[fSimPar.Label].EnePar[2]);
  fEneGenerator->SetParameter(3, fParameters[fSimPar.Label].EnePar[3]);
}

GBSPrimaryGeneratorAction::~GBSPrimaryGeneratorAction() {}

void GBSPrimaryGeneratorAction::SetParameters()
{
  // copy from higs_dgn
  std::vector<double> ang{0.01, 0.99, 0.};
  std::vector<double> ene5{1.52296, 5.24553e-05, -2.76625e-05, 1.06252e-07};
  fParameters.insert(std::make_pair(GBSLabel::Ene5, GunParameters(ang, ene5)));

  std::vector<double> ene6{2.13315, 6.83885e-05, -3.99906e-05, 1.54236e-07};
  fParameters.insert(std::make_pair(GBSLabel::Ene6, GunParameters(ang, ene6)));

  std::vector<double> ene7{2.63643, 6.73762e-05, -5.05328e-05, 1.9453e-07};
  fParameters.insert(std::make_pair(GBSLabel::Ene7, GunParameters(ang, ene7)));

  std::vector<double> ene15{7.26197, -0.000177025, -0.000173958, 6.78406e-07};
  fParameters.insert(
      std::make_pair(GBSLabel::Ene15, GunParameters(ang, ene15)));

  std::vector<double> ene20{10.2641, -0.00064475, -0.000270657, 1.06321e-06};
  fParameters.insert(
      std::make_pair(GBSLabel::Ene20, GunParameters(ang, ene20)));
}

void GBSPrimaryGeneratorAction::GeneratePrimaries(G4Event *event)
{
  fParticleGun->SetParticlePosition(GetPosition());
  fParticleGun->SetParticleMomentumDirection(GetDirection());

  // Energy
  // Copy and paste from original.  I can not understand this
  auto nubar = 7.;
  auto eg = nubar;
  auto nen = fEneGenerator->Eval(fDivergence * 180.0 / CLHEP::pi) *
             (1 + (nubar - eg) / (eg - 2.2));

  auto beam_energy_width = 0.110;
  // Probably author wants inside two sigmas.
  // auto nen_orig = nen;
  // do {
  //   nen = gRandom->Gaus(nen, beam_energy_width);
  // } while (nen - nen_orig > nen_orig + 2 * beam_energy_width);
  auto ene = gRandom->Gaus(nen, beam_energy_width);
  fParticleGun->SetParticleEnergy(ene * MeV);

  fParticleGun->GeneratePrimaryVertex(event);

  G4AutoLock lock(&mutexInPGA);
  if (nEveInPGA++ % 10000 == 0)
    G4cout << nEveInPGA - 1 << " events done" << G4endl;
}

G4ThreeVector GBSPrimaryGeneratorAction::GetDirection()
{
  G4double phi;
  fAngGenerator->GetRandom2(fDivergence, phi);
  if (!fSimPar.Polarization) phi = gRandom->Rndm() * TMath::TwoPi();

  auto vx = sin(fDivergence) * cos(phi);
  auto vy = sin(fDivergence) * sin(phi);
  auto vz = cos(fDivergence);

  return G4ThreeVector(vx, vy, vz);
}

G4ThreeVector GBSPrimaryGeneratorAction::GetPosition()
{
  // Position
  auto beamDia = 20.0 / 10.0;
  auto r = sqrt(gRandom->Rndm()) * beamDia / 2.;
  auto theta = gRandom->Rndm() * TMath::TwoPi();
  auto xPos = r * cos(theta) * cm;
  auto yPos = r * sin(theta) * cm;
  auto zPos = (gRandom->Rndm() * 3.7 - 3.7 / 2.0) * cm;

  return G4ThreeVector(xPos, yPos, zPos);
}
