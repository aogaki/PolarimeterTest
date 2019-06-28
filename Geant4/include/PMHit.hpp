#ifndef PMHit_h
#define PMHit_h 1

#include "G4Allocator.hh"
#include "G4THitsCollection.hh"
#include "G4ThreeVector.hh"
#include "G4Types.hh"
#include "G4VHit.hh"

class PMHit : public G4VHit
{
 public:
  PMHit();
  virtual ~PMHit();
  PMHit(const PMHit &right);
  const PMHit &operator=(const PMHit &right);
  int operator==(const PMHit &right) const;

  inline void *operator new(size_t);
  inline void operator delete(void *);

  // add setter/getter methods
  void SetVolumeName(G4String volumeName) { fVolumeName = volumeName; };
  G4String GetVolumeName() { return fVolumeName; };

  void SetTime(G4double time) { fTime = time; };
  G4double GetTime() { return fTime; };

  void SetTrackID(G4int id) { fTrackID = id; };
  G4int GetTrackID() { return fTrackID; };

  void SetParentID(G4int id) { fParentID = id; };
  G4int GetParentID() { return fParentID; };

  void SetDepositEnergy(G4double ene) { fDepositEnergy = ene; };
  G4double GetDepositEnergy() { return fDepositEnergy; };

  void SetPDGCode(G4int code) { fPDGCode = code; };
  G4int GetPDGCode() { return fPDGCode; };

 private:
  G4String fVolumeName;
  G4double fTime;
  G4int fTrackID;
  G4int fParentID;
  G4double fDepositEnergy;
  G4int fPDGCode;
};

typedef G4THitsCollection<PMHit> PMHitsCollection;

extern G4ThreadLocal G4Allocator<PMHit> *PMHitAllocator;

inline void *PMHit::operator new(size_t)
{
  if (!PMHitAllocator) PMHitAllocator = new G4Allocator<PMHit>;
  return (void *)PMHitAllocator->MallocSingle();
}

inline void PMHit::operator delete(void *hit)
{
  PMHitAllocator->FreeSingle((PMHit *)hit);
}

#endif
