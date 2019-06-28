#ifndef GBSPARAMETERS_HPP
#define GBSPARAMETERS_HPP 1

#include <vector>

enum class GBSLabel {
  Ene5,
  Ene6,
  Ene7,
  Ene15,
  Ene20,
};

class SimParameters
{
 public:
  GBSLabel Label;
  bool Polarization;
  double Energy;
};

class GunParameters
{
 public:
  std::vector<double> AngPar;
  std::vector<double> EnePar;

  GunParameters()
  {
    AngPar.resize(3);
    EnePar.resize(4);
  }

  GunParameters(double ang0, double ang1, double ang2, double ene0, double ene1,
                double ene2, double ene3)
  {
    AngPar.resize(3);
    AngPar[0] = ang0;
    AngPar[1] = ang1;
    AngPar[2] = ang2;

    EnePar.resize(4);
    EnePar[0] = ene0;
    EnePar[1] = ene1;
    EnePar[2] = ene2;
    EnePar[3] = ene3;
  };

  GunParameters(std::vector<double> vAng, std::vector<double> vEne)
  {
    AngPar = vAng;
    EnePar = vEne;
  };
};

#endif
