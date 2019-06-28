#include <G4Box.hh>
#include <G4LogicalVolume.hh>
#include <G4LogicalVolumeStore.hh>
#include <G4NistManager.hh>
#include <G4PVPlacement.hh>
#include <G4PVReplica.hh>
#include <G4RunManager.hh>
#include <G4SDManager.hh>
#include <G4SystemOfUnits.hh>
#include <G4Tubs.hh>

#include "PMDetectorConstruction.hpp"
#include "PMSD.hpp"

PMDetectorConstruction::PMDetectorConstruction()
    : fWorldMat(nullptr), fTargetMat(nullptr), fScintiMat(nullptr)
{
  fCheckOverlap = true;

  DefineMaterials();
}

PMDetectorConstruction::~PMDetectorConstruction()
{
  for (auto &&pointer : fVisAttributes) delete pointer;
}

void PMDetectorConstruction::DefineMaterials()
{
  auto manager = G4NistManager::Instance();

  // NIST database materials
  fWorldMat = manager->FindOrBuildMaterial("G4_AIR");

  // D2O
  auto isoD = new G4Isotope("Deuteron", 1, 2, 2.014 * g / mole);
  auto eleD = new G4Element("Deuterium", "D", 1);
  eleD->AddIsotope(isoD, 1.0);
  G4Element *eleO = manager->FindOrBuildElement("O");
  fTargetMat = new G4Material("D2O", 1.1056 * g / cm3, 2);
  fTargetMat->AddElement(eleD, 2);
  fTargetMat->AddElement(eleO, 1);

  // BC501A
  G4Element *eleH = manager->FindOrBuildElement("H");
  G4Element *eleC = manager->FindOrBuildElement("C");
  fScintiMat = new G4Material("BC501A", .874 * g / cm3, 2);
  fScintiMat->AddElement(eleH, 1212);
  fScintiMat->AddElement(eleC, 1000);
}

G4VPhysicalVolume *PMDetectorConstruction::Construct()
{
  // world volume
  G4double worldX = 2. * m;
  G4double worldY = 2. * m;
  G4double worldZ = 2. * m;

  G4Box *worldS = new G4Box("World", worldX / 2., worldY / 2., worldZ / 2.);
  G4LogicalVolume *worldLV = new G4LogicalVolume(worldS, fWorldMat, "World");

  G4VisAttributes *visAttributes = new G4VisAttributes(G4Colour::White());
  visAttributes->SetVisibility(false);
  worldLV->SetVisAttributes(visAttributes);
  fVisAttributes.push_back(visAttributes);

  // Target
  auto targetR = 3.2 / 2.0 * cm;
  auto targetL = 3.7 * cm;
  auto targetS =
      new G4Tubs("Target", 0., targetR, targetL / 2.0, 0., CLHEP::pi2);
  auto targetLV = new G4LogicalVolume(targetS, fTargetMat, "Target");
  visAttributes = new G4VisAttributes(G4Colour::Cyan());
  visAttributes->SetVisibility(true);
  targetLV->SetVisAttributes(visAttributes);
  fVisAttributes.push_back(visAttributes);
  new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0), targetLV, "Target",
                    worldLV, false, 0, fCheckOverlap);

  // Detectors
  G4double detR = 12.68 * .5 * cm;
  G4double detL = 5.08 * cm;
  auto detS = new G4Tubs("Detector", 0., detR, detL / 2.0, 0., CLHEP::pi2);
  auto detLV = new G4LogicalVolume(detS, fScintiMat, "Detector");
  visAttributes = new G4VisAttributes(G4Colour::Green());
  visAttributes->SetVisibility(true);
  detLV->SetVisAttributes(visAttributes);
  fVisAttributes.push_back(visAttributes);

  for (auto i = 0; i < 2; i++) {
    auto detCenter = 70 * cm;
    auto detPhi = i * 90.0 * deg;

    auto rotMat = new G4RotationMatrix();
    if (i % 2 == 0)
      rotMat->rotateY(90.0 * deg);
    else
      rotMat->rotateX(90.0 * deg);
    auto detPos =
        G4ThreeVector(detCenter * cos(detPhi), detCenter * sin(detPhi), 0.);
    new G4PVPlacement(rotMat, detPos, detLV, "Detector" + std::to_string(i),
                      worldLV, false, 0, fCheckOverlap);
  }

  G4VPhysicalVolume *worldPV = new G4PVPlacement(
      nullptr, G4ThreeVector(), worldLV, "World", 0, false, 0, fCheckOverlap);
  return worldPV;
}

void PMDetectorConstruction::ConstructSDandField()
{
  // Sensitive Detectors
  G4VSensitiveDetector *SD = new PMSD("SD", "HC");
  G4SDManager::GetSDMpointer()->AddNewDetector(SD);

  G4LogicalVolumeStore *lvStore = G4LogicalVolumeStore::GetInstance();
  for (auto &&lv : *lvStore) {
    if (lv->GetName().contains("Detector"))
      SetSensitiveDetector(lv->GetName(), SD);
  }
}
