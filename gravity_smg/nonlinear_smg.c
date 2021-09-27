#include "nonlinear_smg.h"

int nonlinear_hmcode_correct_Delta_v_0_smg(
  struct background *pba,
  double z_at_tau,
  double Delta_v_0_lcdm,
  double * Delta_v_0
) {

  // Corrections are implemented only for Brans-Dicke
  if (pba->gravity_model_smg == brans_dicke) {

    // Local variables
    double d0, fac;
    double omega = pba->parameters_smg[1];
    if(omega<50.){
      printf("WARNING: Currently HMcode has been fitted only for omega>50. Setting omega=50.\n");
      omega = 50.;
    }

    d0  = 320.0+40.0*pow(z_at_tau, 0.26);
    fac = atan(pow(abs(omega-50.0)*0.001, 0.2))*2.0/acos(-1.0);

    // Fitting formula based on simulations
    *Delta_v_0 = d0 + (Delta_v_0_lcdm - d0) * fac;
  }
  else{
    printf("WARNING: Currently HMcode is implemented only for Brans-Dicke.\n");
    *Delta_v_0 = Delta_v_0_lcdm;
  }

  return _SUCCESS_;
}
