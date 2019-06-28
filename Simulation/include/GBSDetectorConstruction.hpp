#ifndef GBSDetectorConstruction_h
#define GBSDetectorConstruction_h 1

#include <memory>
#include <vector>

#include <G4GenericMessenger.hh>
#include <G4Material.hh>
#include <G4VPhysicalVolume.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4VisAttributes.hh>

class GBSDetectorConstruction : public G4VUserDetectorConstruction
{
 public:
  GBSDetectorConstruction();
  virtual ~GBSDetectorConstruction();

  virtual G4VPhysicalVolume *Construct();
  virtual void ConstructSDandField();

 private:
  G4bool fCheckOverlap;

  // Materials
  void DefineMaterials();
  // G4Material *fVacuumMat;
  std::unique_ptr<G4Material> fAirMat;
  std::unique_ptr<G4Material> fTargetMat;
  std::unique_ptr<G4Material> fScintiMat;

  G4LogicalVolume *GetTarget();
  G4LogicalVolume *GetDetector();

  std::vector<G4VisAttributes *> fVisAttributes;
};

#endif
