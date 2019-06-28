#include <random>

#include <Randomize.hh>

#ifdef G4MULTITHREADED
#include <G4MTRunManager.hh>
#include <G4Threading.hh>
#else
#include <G4RunManager.hh>
#endif

#include <G4UImanager.hh>
#ifdef G4VIS_USE
#include <G4TrajectoryParticleFilter.hh>
#include <G4VisExecutive.hh>
#endif
#ifdef G4UI_USE
#include <G4UIExecutive.hh>
#endif

// Reference physics lists and something
#include <G4NeutronTrackingCut.hh>
#include <G4SystemOfUnits.hh>
#include <QGSP_BERT_HP.hh>

// User defined classes
#include "PMActionInitialization.hpp"
#include "PMDetectorConstruction.hpp"
#include "PMPhysicsList.hpp"

void PrintUsage()
{
  G4cerr << " Usage: " << G4endl;
  G4cerr << " ./polarymeter [-m macro filename]\n"
         << " -e [kinetic energy in MeV]\n"
         << " -u: unpolarized beam\n"
         << G4endl;
}

int main(int argc, char **argv)
{
  G4String macro = "";
  G4double beamEne = 0.;
  G4bool unpolarizedFlag = false;

  for (G4int i = 1; i < argc; i++) {
    if (G4String(argv[i]) == "-m") {
      macro = argv[++i];
    } else if (G4String(argv[i]) == "-e") {
      beamEne = std::stod(argv[++i]);
    } else if (G4String(argv[i]) == "-u") {
      unpolarizedFlag = true;
    } else {
      PrintUsage();
      return 1;
    }
  }

  // Choose the Random engine
  // Need both?
  std::random_device rndSeed;  // Use C++11!
  CLHEP::HepRandom::setTheEngine(new CLHEP::MTwistEngine(rndSeed()));
  G4Random::setTheEngine(new CLHEP::MTwistEngine(rndSeed()));

// Construct the default run manager
#ifdef G4MULTITHREADED
  auto runManager = new G4MTRunManager();
  // runManager->SetNumberOfThreads(G4Threading::G4GetNumberOfCores());
  runManager->SetNumberOfThreads(8);
#else
  auto runManager = new G4RunManager();
#endif

  // Detector construction
  runManager->SetUserInitialization(new PMDetectorConstruction());

  // Physics list
  //auto physicsList = new PMPhysicsList();
  auto physicsList = new QGSP_BERT_HP();
  G4NeutronTrackingCut *mycut = new G4NeutronTrackingCut();
  // mycut->SetTimeLimit(60 * ns);
  // mycut->SetKineticEnergyLimit(15 * keV);
  physicsList->RegisterPhysics(mycut);
  runManager->SetUserInitialization(physicsList);

  // Primary generator action and User action intialization
  runManager->SetUserInitialization(
      new PMActionInitialization(beamEne, unpolarizedFlag));

  // Initialize G4 kernel
  //
  runManager->Initialize();

#ifdef G4VIS_USE
  // Initialize visualization
  auto visManager = new G4VisExecutive();
  visManager->Initialize();
#endif

  // Get the pointer to the User Interface manager
  auto UImanager = G4UImanager::GetUIpointer();
  if (macro != "") {
    // batch mode
    G4String command = "/control/execute ";
    UImanager->ApplyCommand(command + macro);
  } else {
    // interactive mode : define UI session
#ifdef G4UI_USE
    auto ui = new G4UIExecutive(argc, argv);
#ifdef G4VIS_USE
    UImanager->ApplyCommand("/control/execute init_vis.mac");
#else
    UImanager->ApplyCommand("/control/execute init.mac");
#endif
    ui->SessionStart();
    delete ui;
#endif
  }

#ifdef G4VIS_USE
  delete visManager;
#endif

  delete runManager;

  return 0;
}
