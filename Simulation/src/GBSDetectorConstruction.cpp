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

#include "GBSDetectorConstruction.hpp"
#include "GBSSD.hpp"

GBSDetectorConstruction::GBSDetectorConstruction()
{
  fCheckOverlap = true;

  DefineMaterials();
}

GBSDetectorConstruction::~GBSDetectorConstruction()
{
  for (auto &&vis : fVisAttributes) delete vis;
}

void GBSDetectorConstruction::DefineMaterials()
{
  // These are copy from someones code given by Catalin.
  // I seriously don't know why authors NOT use NIST database
  // Check!!
  auto z = 1;
  auto a = 1.008 * g / mole;
  auto elH = new G4Element("Hydrogen", "H", z, a);

  z = 1;
  a = 2.014 * g / mole;
  auto isoD = new G4Isotope("Deuteron", z, 2, a);
  auto elD = new G4Element("Deuterium", "D", 1);
  elD->AddIsotope(isoD, 1.0);

  z = 6;
  auto isoC12 = new G4Isotope("Carbon-12", z, 12);
  auto isoC13 = new G4Isotope("Carbon-13", z, 13);
  auto elC = new G4Element("Carbon", "C", 2);
  elC->AddIsotope(isoC12, 0.9893);
  elC->AddIsotope(isoC13, 0.0107);

  z = 7;
  a = 14.01 * g / mole;
  auto elN = new G4Element("Nitrogen", "N", z, a);

  z = 8;
  auto isoO16 = new G4Isotope("Oxygen-16", z, 16);
  auto isoO17 = new G4Isotope("Oxygen-17", z, 17);
  auto isoO18 = new G4Isotope("Oxygen-18", z, 18);
  auto elO = new G4Element("Oxygen", "O", 3);
  elO->AddIsotope(isoO16, 0.9976);
  elO->AddIsotope(isoO17, 0.0004);
  elO->AddIsotope(isoO18, 0.0020);

  // Air for world
  auto density = 1.275 * kg / m3;
  fAirMat.reset(new G4Material("Air", density, 2));
  fAirMat->AddElement(elN, 0.7);
  fAirMat->AddElement(elO, 0.3);

  // Target
  density = 1.1056 * g / cm3;
  fTargetMat.reset(new G4Material("D2O", density, 2));
  fTargetMat->AddElement(elD, 2);
  fTargetMat->AddElement(elO, 1);

  // Detector
  density = 0.874 * g / cm3;
  fScintiMat.reset(new G4Material("BC501A", density, 2));
  fScintiMat->AddElement(elH, 1212);
  fScintiMat->AddElement(elC, 1000);

  // NIST database materials
  // G4NistManager *manager = G4NistManager::Instance();
  // fAirMat.reset(manager->FindOrBuildMaterial("G4_Air"));
}

G4VPhysicalVolume *GBSDetectorConstruction::Construct()
{
  // world volume
  auto worldX = 5.1 * m;
  auto worldY = 5.1 * m;
  auto worldZ = 5.1 * m;

  auto worldS = new G4Box("World", worldX / 2., worldY / 2., worldZ / 2.);
  auto worldLV = new G4LogicalVolume(worldS, fAirMat.get(), "World");

  auto visAttributes = new G4VisAttributes(G4Colour::White());
  visAttributes->SetVisibility(false);
  worldLV->SetVisAttributes(visAttributes);
  fVisAttributes.push_back(visAttributes);

  // Target
  auto targetLV = GetTarget();
  visAttributes = new G4VisAttributes(G4Colour::Cyan());
  visAttributes->SetVisibility(true);
  targetLV->SetVisAttributes(visAttributes);
  fVisAttributes.push_back(visAttributes);
  new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0), targetLV, "Target",
                    worldLV, false, 0, fCheckOverlap);

  // Detector
  auto detectorLV = GetDetector();
  visAttributes = new G4VisAttributes(G4Colour::Magenta());
  visAttributes->SetVisibility(true);
  detectorLV->SetVisAttributes(visAttributes);
  fVisAttributes.push_back(visAttributes);

  auto nDet = 2;
  for (auto i = 0; i < nDet; i++) {
    auto l = 70. * cm;
    auto phi = 90. * i * deg;

    auto rot = new G4RotationMatrix(0., 0., 0.);
    // rot->rotateX(90. * deg);
    // rot->rotateY(phi - 90. * deg);
    rot->rotateX(phi);
    rot->rotateY(phi - 90. * deg);
    new G4PVPlacement(rot, G4ThreeVector(l * cos(phi), l * sin(phi), 0),
                      detectorLV, "Detector" + std::to_string(i), worldLV,
                      false, i, fCheckOverlap);
  }

  auto worldPV = new G4PVPlacement(nullptr, G4ThreeVector(), worldLV, "World",
                                   nullptr, false, 0, fCheckOverlap);
  return worldPV;
}

G4LogicalVolume *GBSDetectorConstruction::GetTarget()
{
  auto rmin = 0.0 * cm;
  auto rmax = 3.2 / 2.0 * cm;
  auto length = 3.7 * cm;
  auto targetS =
      new G4Tubs("Target", rmin, rmax, length / 2.0, 0., CLHEP::twopi);

  return new G4LogicalVolume(targetS, fScintiMat.get(), "Target");
}

G4LogicalVolume *GBSDetectorConstruction::GetDetector()
{
  auto rmin = 0.0 * cm;
  auto rmax = 12.68 * .5 * cm;
  auto length = 5.08 * cm;
  auto detectorS =
      new G4Tubs("Detector", rmin, rmax, length / 2.0, 0., CLHEP::twopi);

  return new G4LogicalVolume(detectorS, fScintiMat.get(), "Detector");
}

void GBSDetectorConstruction::ConstructSDandField()
{
  // Sensitive Detectors
  G4VSensitiveDetector *SD = new GBSSD("SD", "HC");
  G4SDManager::GetSDMpointer()->AddNewDetector(SD);

  G4LogicalVolumeStore *lvStore = G4LogicalVolumeStore::GetInstance();
  for (auto &&lv : *lvStore) {
    if (lv->GetName().contains("Detector"))
      SetSensitiveDetector(lv->GetName(), SD);
  }
}
