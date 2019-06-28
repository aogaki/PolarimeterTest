#ifndef GBSHit_h
#define GBSHit_h 1

#include "G4Allocator.hh"
#include "G4THitsCollection.hh"
#include "G4ThreeVector.hh"
#include "G4Types.hh"
#include "G4VHit.hh"

class GBSHit : public G4VHit
{
 public:
  GBSHit();
  virtual ~GBSHit();
  GBSHit(const GBSHit &right);
  const GBSHit &operator=(const GBSHit &right);
  int operator==(const GBSHit &right) const;

  inline void *operator new(size_t);
  inline void operator delete(void *);

  // add setter/getter methods
  void SetEventID(G4int id) { fEventID = id; };
  G4int GetEventID() { return fEventID; };

  void SetPDGCode(G4int code) { fPDGCode = code; };
  G4int GetPDGCode() { return fPDGCode; };

  void SetDepositEnergy(G4double ene) { fDepositEnergy = ene; };
  G4double GetDepositEnergy() { return fDepositEnergy; };

  void SetTime(G4double time) { fTime = time; };
  G4double GetTime() { return fTime; };

  void SetVolumeName(G4String name) { fVolumeName = name; };
  G4String GetVolumeName() { return fVolumeName; };

  void SetTrackID(G4int id) { fTrackID = id; };
  G4int GetTrackID() { return fTrackID; };

 private:
  G4int fEventID;
  G4int fTrackID;
  G4int fPDGCode;
  G4String fVolumeName;
  G4double fDepositEnergy;
  G4double fTime;
};

typedef G4THitsCollection<GBSHit> GBSHitsCollection;

extern G4ThreadLocal G4Allocator<GBSHit> *GBSHitAllocator;

inline void *GBSHit::operator new(size_t)
{
  if (!GBSHitAllocator) GBSHitAllocator = new G4Allocator<GBSHit>;
  return (void *)GBSHitAllocator->MallocSingle();
}

inline void GBSHit::operator delete(void *hit)
{
  GBSHitAllocator->FreeSingle((GBSHit *)hit);
}

#endif
