#ifndef PMDetectorConstruction_h
#define PMDetectorConstruction_h 1

#include <vector>

#include <G4GenericMessenger.hh>
#include <G4Material.hh>
#include <G4VPhysicalVolume.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4VisAttributes.hh>

class PMDetectorConstruction : public G4VUserDetectorConstruction
{
 public:
  PMDetectorConstruction();
  virtual ~PMDetectorConstruction();

  virtual G4VPhysicalVolume *Construct();
  virtual void ConstructSDandField();

 private:
  G4bool fCheckOverlap;

  // Materials
  void DefineMaterials();
  G4Material *fWorldMat;
  G4Material *fTargetMat;
  G4Material *fScintiMat;

  std::vector<G4VisAttributes *> fVisAttributes;
};

#endif
