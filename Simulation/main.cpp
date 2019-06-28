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

// Reference physics lists
#include <QGSP_BERT_HP.hh>

// User defined classes
#include "GBSActionInitialization.hpp"
#include "GBSDetectorConstruction.hpp"
#include "GBSParameters.hpp"
#include "GBSPhysicsList.hpp"

void PrintUsage()
{
  G4cerr << " Usage: " << G4endl;
  G4cerr << " ./polMeter [-m macro filename]\n"
         << " -e Gamma energy.  5, 6, 7, 15, 20 are available\n"
         << " -u Unpolarized beam\n"
         << G4endl;
}

int main(int argc, char **argv)
{
  SimParameters par;
  par.Polarization = true;
  par.Label = GBSLabel::Ene7;
  G4String macro = "";

  for (G4int i = 1; i < argc; i++) {
    if (G4String(argv[i]) == "-m") {
      macro = argv[++i];
    } else if (G4String(argv[i]) == "-u") {
      par.Polarization = false;
    } else if (G4String(argv[i]) == "-e") {
      G4String type = *argv[++i];
      if (type == "5") {
        par.Label = GBSLabel::Ene5;
      } else if (type == "6") {
        par.Label = GBSLabel::Ene6;
      } else if (type == "7") {
        par.Label = GBSLabel::Ene7;
      } else if (type == "15") {
        par.Label = GBSLabel::Ene15;
      } else if (type == "20") {
        par.Label = GBSLabel::Ene20;
      } else {
        G4cout << "Beam energy is limited now." << G4endl;
        PrintUsage();
        return 1;
      }
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
  runManager->SetNumberOfThreads(G4Threading::G4GetNumberOfCores());
#else
  auto runManager = new G4RunManager();
#endif

  // Detector construction
  runManager->SetUserInitialization(new GBSDetectorConstruction());

  // Physics list
  // auto physicsList = new GBSPhysicsList();
  auto physicsList = new QGSP_BERT_HP();
  runManager->SetUserInitialization(physicsList);

  // Primary generator action and User action intialization
  runManager->SetUserInitialization(new GBSActionInitialization(par));

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
  if (argc != 1) {
    // execute an argument macro file if exist
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    UImanager->ApplyCommand(command + fileName);
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
