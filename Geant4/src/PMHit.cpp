#include <iomanip>

#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"

#include "PMHit.hpp"


G4ThreadLocal G4Allocator<PMHit> *PMHitAllocator = 0;


PMHit::PMHit()
   : G4VHit()
{}

PMHit::~PMHit()
{}

PMHit::PMHit(const PMHit & /*right*/)
   : G4VHit()
{}

const PMHit &
PMHit::operator=(const PMHit & /*right*/)
{
   return *this;
}

int PMHit::operator==(const PMHit & /*right*/) const
{
   return 0;
}
