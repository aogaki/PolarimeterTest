#include <iomanip>

#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"

#include "GBSHit.hpp"


G4ThreadLocal G4Allocator<GBSHit> *GBSHitAllocator = 0;


GBSHit::GBSHit()
   : G4VHit()
{}

GBSHit::~GBSHit()
{}

GBSHit::GBSHit(const GBSHit & /*right*/)
   : G4VHit()
{}

const GBSHit &
GBSHit::operator=(const GBSHit & /*right*/)
{
   return *this;
}

int GBSHit::operator==(const GBSHit & /*right*/) const
{
   return 0;
}
