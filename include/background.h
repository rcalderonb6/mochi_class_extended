/** @file background.h Documented includes for background module */

#ifndef __BACKGROUND__
#define __BACKGROUND__

#include "common.h"
#include "quadrature.h"
#include "growTable.h"
#include "arrays.h"
#include "dei_rkck.h"
#include "parser.h"

/** list of possible types of spatial curvature */

enum spatial_curvature {flat,open,closed};

/** list of possible parametrisations of the DE equation of state */

enum equation_of_state {CLP,EDE};


/** list of possible theories and models (_smg) */

enum gravity_model {propto_omega, propto_scale,no_slip,
    constant_alphas,
    eft_alphas_power_law, eft_gammas_power_law, eft_gammas_exponential,
    external_alphas, // MC parametrization with external alpha_X's
    stable_params, // MC parametrization using external stable functions as in 1810.05225
    galileon, nkgb,
    brans_dicke,
    quintessence_monomial, quintessence_tracker,
    alpha_attractor_canonical
};

/** parameterized expansion, only for non-self consistent Horndeski theories (_smg) */

enum expansion_model {lcdm, wowa, wowa_w, wede, wext, rho_de};

/** list of possible parametrizations of the varying fundamental constants */

enum varconst_dependence {varconst_none,varconst_instant};

/** list of formats for the vector of background quantities */

enum vecback_format {short_info, normal_info, long_info};

/** list of interpolation methods: search location in table either
    by bisection (inter_normal), or step by step starting from given
    index (inter_closeby) */

enum interpolation_method {inter_normal, inter_closeby};

/**
 * background structure containing all the background information that
 * other modules need to know.
 *
 * Once initialized by the backgound_init(), contains all necessary
 * information on the background evolution (except thermodynamics),
 * and in particular, a table of all background quantities as a
 * function of time and scale factor, used for interpolation in other
 * modules.
 */

struct background
{
  /** @name - input parameters initialized by user in input module
   *  (all other quantities are computed in this module, given these parameters
   *   and the content of the 'precision' structure)
   *
   * The background cosmological parameters listed here form a parameter
   * basis which is directly usable by the background module. Nothing
   * prevents from defining the input cosmological parameters
   * differently, and to pre-process them into this format, using the input
   * module (this might require iterative calls of background_init()
   * e.g. for dark energy or decaying dark matter). */

  //@{

  double H0; /**< \f$ H_0 \f$: Hubble parameter (in fact, [\f$H_0/c\f$]) in \f$ Mpc^{-1} \f$ */
  double h;  /**< reduced Hubble parameter */

  double Omega0_g; /**< \f$ \Omega_{0 \gamma} \f$: photons */
  double T_cmb;    /**< \f$ T_{cmb} \f$: current CMB temperature in Kelvins */

  double Omega0_b; /**< \f$ \Omega_{0 b} \f$: baryons */

  double Omega0_ur; /**< \f$ \Omega_{0 \nu r} \f$: ultra-relativistic neutrinos */

  double Omega0_cdm;      /**< \f$ \Omega_{0 cdm} \f$: cold dark matter */

  double Omega0_idm; /**< \f$ \Omega_{0 idm} \f$: interacting dark matter with photons, baryons, and idr */


  double Omega0_idr; /**< \f$ \Omega_{0 idr} \f$: interacting dark radiation */
  double T_idr;      /**< \f$ T_{idr} \f$: current temperature of interacting dark radiation in Kelvins */

  double Omega0_dcdmdr;   /**< \f$ \Omega_{0 dcdm}+\Omega_{0 dr} \f$: decaying cold dark matter (dcdm) decaying to dark radiation (dr) */
  double Omega_ini_dcdm;  /**< \f$ \Omega_{ini,dcdm} \f$: rescaled initial value for dcdm density (see 1407.2418 for definitions) */
  double Gamma_dcdm;      /**< \f$ \Gamma_{dcdm} \f$: decay constant for decaying cold dark matter */
  double tau_dcdm;

  int N_ncdm;                            /**< Number of distinguishable ncdm species */
  /* the following parameters help to define tabulated ncdm p-s-d passed in file */
  char * ncdm_psd_files;                 /**< list of filenames for tabulated p-s-d */
  int * got_files;                       /**< list of flags for each species, set to true if p-s-d is passed through file */
  /* the following parameters help to define the analytical ncdm phase space distributions (p-s-d) */
  double * ncdm_psd_parameters;          /**< list of parameters for specifying/modifying ncdm p.s.d.'s, to be customized for given model
                                            (could be e.g. mixing angles) */
  double * M_ncdm;                       /**< vector of masses of non-cold relic: dimensionless ratios m_ncdm/T_ncdm */
  double * m_ncdm_in_eV;                 /**< list of ncdm masses in eV (inferred from M_ncdm and other parameters above) */
  double * Omega0_ncdm, Omega0_ncdm_tot; /**< Omega0_ncdm for each species and for the total Omega0_ncdm */
  double * T_ncdm,T_ncdm_default;        /**< list of 1st parameters in p-s-d of non-cold relics: relative temperature
                                            T_ncdm1/T_gamma; and its default value */
  double * ksi_ncdm, ksi_ncdm_default;   /**< list of 2nd parameters in p-s-d of non-cold relics: relative chemical potential
                                            ksi_ncdm1/T_ncdm1; and its default value */
  double * deg_ncdm, deg_ncdm_default;    /**< vector of degeneracy parameters in factor of p-s-d: 1 for one family of neutrinos
                                             (= one neutrino plus its anti-neutrino, total g*=1+1=2, so deg = 0.5 g*); and its
                                             default value */
  int * ncdm_input_q_size; /**< Vector of numbers of q bins */
  double * ncdm_qmax;      /**< Vector of maximum value of q */

  double Omega0_k;         /**< \f$ \Omega_{0_k} \f$: curvature contribution */

  double Omega0_lambda;    /**< \f$ \Omega_{0_\Lambda} \f$: cosmological constant */
  double Omega0_fld;       /**< \f$ \Omega_{0 de} \f$: fluid */
  double Omega0_scf;       /**< \f$ \Omega_{0 scf} \f$: scalar field */
  short use_ppf; /**< flag switching on PPF perturbation equations instead of true fluid equations for perturbations. It could have been defined inside
                    perturbation structure, but we leave it here in such way to have all fld parameters grouped. */
  double c_gamma_over_c_fld; /**< ppf parameter defined in eq. (16) of 0808.3125 [astro-ph] */
  enum equation_of_state fluid_equation_of_state; /**< parametrisation scheme for fluid equation of state */
  double w0_fld;   /**< \f$ w0_{DE} \f$: current fluid equation of state parameter */
  double wa_fld;   /**< \f$ wa_{DE} \f$: fluid equation of state parameter derivative */
  double cs2_fld;  /**< \f$ c^2_{s~DE} \f$: sound speed of the fluid in the frame comoving with the fluid (so, this is
                      not [delta p/delta rho] in the synchronous or newtonian gauge!) */
  double Omega_EDE;        /**< \f$ wa_{DE} \f$: Early Dark Energy density parameter */
  double * scf_parameters; /**< list of parameters describing the scalar field potential */
  short attractor_ic_scf;  /**< whether the scalar field has attractor initial conditions */
  int scf_tuning_index;    /**< index in scf_parameters used for tuning */
  double phi_ini_scf;      /**< \f$ \phi(t_0) \f$: scalar field initial value */
  double phi_prime_ini_scf;/**< \f$ d\phi(t_0)/d\tau \f$: scalar field initial derivative wrt conformal time */
  int scf_parameters_size; /**< size of scf_parameters */
  double varconst_alpha; /**< finestructure constant for varying fundamental constants */
  double varconst_me; /**< electron mass for varying fundamental constants */
  enum varconst_dependence varconst_dep; /**< dependence of the varying fundamental constants as a function of time */
  double varconst_transition_redshift; /**< redshift of transition between varied fundamental constants and normal fundamental constants in the 'varconst_instant' case*/

  //@}


  /** @name - hi_class (_smg) related parameters */

  double hubble_friction; /** friction coefficient in H' equation: H' = ... + H_friction*(H^2 - rho_crit) [NOT ONLY IN SMG!] */
  int hubble_evolution; /** whether to evolve H' from the equation */

  enum gravity_model gravity_model_smg; /** Horndeski model */
  //   enum gravity_model_subclass gravity_submodel_smg; /** Horndeski model */
  enum expansion_model expansion_model_smg; /* choice of expansion rate */

  short initial_conditions_set_smg; /* whether IC have been established. For printing and information */
  short parameters_tuned_smg; /* whether model has been tuned. For doing stability tests, etc... */
  short is_quintessence_smg; /* is the scalar field from a quintessence model?*/

  double Omega0_smg; /**< \f$ \Omega_{0_\phi} \f$ : scalar field energy fraction */
  double Omega_smg_debug; /**< debug value when no tuning is wanted */
  short attractor_ic_smg; /** < whether the scalar field has attractor initial conditions */
  short has_smg_file; /* MC flag checking if external input file with alpha_X(lna) is provided*/
  FileName smg_file_name; /* MC path to external input file */
  short has_expansion_file; /* MC flag checking if external input file with w(lna) is provided*/
  FileName expansion_file_name; /* MC path to external input file */

  double xi_0_smg; /** < final value of xi = phi' H/(aH_0^2)  */
  double phi_0_smg; /** < final value of phi  */
  double M2_0_smg; /** < final value of M_*^2  */

  double cs2_safe_smg; /**< threshold for the speed of sound to consider it negative */
  double D_safe_smg; /* threshold to consider the kinetic term of scalars negative in the stability check */
  double ct2_safe_smg; /* threshold to consider the sound speed of tensors negative in the stability check */
  double M2_safe_smg; /* threshold to consider the kinetic term of tensors (M2) negative in the stability check */
  double kineticity_safe_smg; /**< minimum value of the kineticity, to avoid problems with the perturbations */
  double quintessence_w_safe_smg; /**< threshold to consider the quintessence equation of state less than -1 in the stability check */

  double min_M2_smg; /**< minimum value of planck mass (for stability test) */
  double min_ct2_smg; /**< minimum value of tensor speed of sound squared (for stability test) */
  double min_D_smg; /**< minimum value of scalar kinetic term (for stability test) */
  double min_cs2_smg; /**< minimum value of scalar speed of sound squared (for stability test) */
  double a_min_M2_smg; /**< scale factor of the minimum value of planck mass (for stability test) */
  double a_min_ct2_smg; /**< scale factor of the minimum value of tensor speed of sound squared (for stability test) */
  double a_min_D_smg; /**< scale factor of the minimum value of scalar kinetic term (for stability test) */
  double a_min_cs2_smg; /**< scale factor of the minimum value of scalar speed of sound squared (for stability test) */

  double min_bra_smg; /**< minimum value of the braiding */
  double max_bra_smg; /**< maximum value of the braiding */

  int skip_stability_tests_smg; /**< specify if you want to skip the stability tests for the field perturbations */
  double a_min_stability_test_smg; /** < skip stability tests for a < a_min */


  int field_evolution_smg; /**< does the model require solving the equation for the scalar field at the background? this is typically not the case for parameterized models */
  int M_pl_evolution_smg; /**< does the model require integrating the Planck mass from alpha_M? */
  int rho_evolution_smg; /**< does the model require integrating the energy density? */

   /* Modified gravity parameters
   * parameters_smg -> contains the primary parameters. Any param that might be varied to determine Omega_smg should be here
   * tuning_index_smg -> which parameter is varied to obtain the right Omega_smg
   * parameters_2_smg -> contains auxiliary parameters. These will not be varied to obtain Omega_smg
   * for non-dynamical models: the expansion history in parameters_smg, while the alphas are in parameters_2_smg
   */
  double * parameters_smg;  /**< list of parameters describing the modified gravity model (must contain the shooting parameter) */
  int parameters_size_smg;  /**< size of scf_parameters */
  int tuning_index_smg;     /**< index in parameters_smg used for tuning */
  double tuning_dxdy_guess_smg; /**< guess for the scale of the tuning value */

  double * parameters_2_smg;  /**< list of auxiliary parameters describing the modified gravity model */
  int parameters_2_size_smg; /**< size of parameters_smg */


  /* -- MC: found external alphas parametrization to be numerically unstable. Moved to stable parametrization */
  int ext_alphas_size_smg; /* stores total number of rows in input file */
  int ext_num_alphas; /* how many alpha parameters*/
  double * ext_alphas_lna_smg; /* array of ln(a) values in input file */
  double * ext_alphas_smg; /* array of size ext_alphas_size_smg*num_ext_alphas_smg containing all alpha_X*/
  double * ext_ddalphas_smg; /* array of size ext_alphas_size_smg*num_ext_alphas_smg containing all alpha_X second derivatives for interpolation*/

  int M_pl_tuning_smg; /**< whether we want secondary tuning for M_pl(today) */
  int tuning_index_2_smg;     /**< index in scf_parameters used for tuning (the Planck mass) */
  double M_pl_today_smg;

  short output_background_smg; /**< flag regulating the amount of information printed onbackground.dat output */

  /** @name - related parameters */

  //@{

  double age; /**< age in Gyears */
  double conformal_age; /**< conformal age in Mpc */
  double K; /**< \f$ K \f$: Curvature parameter \f$ K=-\Omega0_k*a_{today}^2*H_0^2\f$; */
  int sgnK; /**< K/|K|: -1, 0 or 1 */
  double Neff; /**< so-called "effective neutrino number", computed at earliest time in interpolation table */
  double Omega0_dcdm; /**< \f$ \Omega_{0 dcdm} \f$: decaying cold dark matter */
  double Omega0_dr; /**< \f$ \Omega_{0 dr} \f$: decay radiation */
  double Omega0_m;  /**< total non-relativistic matter today */
  double Omega0_r;  /**< total ultra-relativistic radiation today */
  double Omega0_de; /**< total dark energy density today, currently defined as 1 - Omega0_m - Omega0_r - Omega0_k */
  double Omega0_nfsm; /**< total non-free-streaming matter, that is, cdm, baryons and wdm */
  double a_eq;      /**< scale factor at radiation/matter equality */
  double H_eq;      /**< Hubble rate at radiation/matter equality [Mpc^-1] */
  double z_eq;      /**< redshift at radiation/matter equality */
  double tau_eq;    /**< conformal time at radiation/matter equality [Mpc] */

  //@}


  /** @name - all indices for the vector of background (=bg) quantities stored in table */

  //@{

  int index_bg_a;             /**< scale factor (in fact (a/a_0), see
                                 normalisation conventions explained
                                 at beginning of background.c) */
  int index_bg_H;             /**< Hubble parameter in \f$Mpc^{-1}\f$ */
  int index_bg_H_prime;       /**< its derivative w.r.t. conformal time */

  /* end of vector in short format, now quantities in normal format */

  int index_bg_rho_g;         /**< photon density */
  int index_bg_rho_b;         /**< baryon density */
  int index_bg_rho_cdm;       /**< cdm density */
  int index_bg_rho_idm;       /**< idm density */
  int index_bg_rho_lambda;    /**< cosmological constant density */
  int index_bg_rho_fld;       /**< fluid density */
  int index_bg_w_fld;         /**< fluid equation of state */
  int index_bg_rho_idr;       /**< density of interacting dark radiation */
  int index_bg_rho_ur;        /**< relativistic neutrinos/relics density */
  int index_bg_rho_dcdm;      /**< dcdm density */
  int index_bg_rho_dr;        /**< dr density */

  int index_bg_phi_scf;       /**< scalar field value */
  int index_bg_phi_prime_scf; /**< scalar field derivative wrt conformal time */
  int index_bg_V_scf;         /**< scalar field potential V */
  int index_bg_dV_scf;        /**< scalar field potential derivative V' */
  int index_bg_ddV_scf;       /**< scalar field potential second derivative V'' */
  int index_bg_rho_scf;       /**< scalar field energy density */
  int index_bg_p_scf;         /**< scalar field pressure */
  int index_bg_p_prime_scf;         /**< scalar field pressure */

  /** @name - hi_class (_smg) related parameters */

  int index_bg_phi_smg;       /**< scalar field value */
  int index_bg_phi_prime_smg; /**< scalar field derivative wrt conformal time */
  int index_bg_phi_prime_prime_smg; /**< scalar field second derivative wrt conformal time */
  int index_bg_rho_smg;       /**< scalar field energy density */
  int index_bg_p_smg;         /**< scalar field pressure */
  int index_bg_rho_prime_smg;       /**< derivative of the scalar field energy density */
  int index_bg_current_smg;       /**< scalar field current */
  int index_bg_shift_smg;       /**< scalar field shift */
  int index_bg_M2_smg;   /**< relative Planck mass */
  int index_bg_delta_M2_smg;   /**< relative Planck mass -1. */
  int index_bg_kineticity_over_phiphi_smg;/**< scalar field kineticity alpha_k*(a*H/phi')^2 (BS eq A.8)*/
  int index_bg_braiding_over_phi_smg;/**< scalar field braiding alpha_b*a*H/phi' (BS eq A.9)*/
  int index_bg_beyond_horndeski_over_phi_smg;/**<scalar field beyond horndeski alpha_H*a*H/phi'*/
  int index_bg_braiding_over_phi_prime_smg;/**< scalar field braiding alpha_b*a*H/phi' derivative*/
  int index_bg_beyond_horndeski_over_phi_prime_smg;/**<scalar field beyond horndeski alpha_H*a*H/phi' derivative*/
  int index_bg_kineticity_smg;/**< scalar field kineticity alpha_k (BS eq A.8)*/
  int index_bg_braiding_smg;/**< scalar field braiding alpha_b (BS eq A.9)*/
  int index_bg_tensor_excess_smg;/**< scalar field tensor excess alpha_t (BS eq A.10)*/
  int index_bg_mpl_running_smg; /**< scalar field relative Planck mass running*/
  int index_bg_beyond_horndeski_smg;/**<scalar field beyond horndeski alpha_H*/
  int index_bg_kineticity_prime_smg;/**< derivative of kineticity wrt tau (BS eq A.8)*/
  int index_bg_braiding_prime_smg;/**< derivative of braiding wrt tau (BS eq A.9)*/
  int index_bg_mpl_running_prime_smg;/**< derivative of Planck mass running wrt tau (BS eq A.7)*/
  int index_bg_tensor_excess_prime_smg;/**< derivative of tensor excess wrt tau (BS eq A.10)*/
  int index_bg_beyond_horndeski_prime_smg;/**<derivative of beyond horndeski alpha_H*/
  int index_bg_cs2_smg; /**< speed of sound for scalar perturbations */
  /* -- MC: found external alphas parametrization to be unstable. Moved to stable parametrization */
  int index_bg_ext_kineticity_smg; /* MC scalar field kineticity alpha_k for external input file */
  int index_bg_ext_braiding_smg;/* MC scalar field braiding alpha_b for external input file */
  int index_bg_ext_mpl_running_smg;/* MC  scalar field relative Planck mass running alpha_m for external input file */
  int index_bg_ext_tensor_excess_smg;/* MC scalar field tensor excess alpha_t for external input file*/

  int index_bg_E0_smg; /**< Hubble constraint */
  int index_bg_E1_smg; /**< Hubble constraint */
  int index_bg_E2_smg; /**< Hubble constraint */
  int index_bg_E3_smg; /**< Hubble constraint */

  int index_bg_P0_smg; /**< Hubble dynamical */
  int index_bg_P1_smg; /**< Hubble dynamical */
  int index_bg_P2_smg; /**< Hubble dynamical */
  int index_bg_R0_smg; /**< Klein-Gordon */
  int index_bg_R1_smg; /**< Klein-Gordon */
  int index_bg_R2_smg; /**< Klein-Gordon */

  int index_bg_A0_smg;
  int index_bg_A1_smg;
  int index_bg_A2_smg;
  int index_bg_A3_smg;
  int index_bg_A4_smg;
  int index_bg_A5_smg;
  int index_bg_A6_smg;
  int index_bg_A7_smg;
  int index_bg_A8_smg;
  int index_bg_A9_smg;
  int index_bg_A10_smg;
  int index_bg_A11_smg;
  int index_bg_A12_smg;
  int index_bg_A13_smg;
  int index_bg_A14_smg;
  int index_bg_A15_smg;
  int index_bg_A16_smg;
  int index_bg_A9_prime_smg;
  int index_bg_A10_prime_smg;
  int index_bg_A12_prime_smg;
  int index_bg_A13_prime_smg;

  int index_bg_B0_smg;
  int index_bg_B1_smg;
  int index_bg_B2_smg;
  int index_bg_B3_smg;
  int index_bg_B4_smg;
  int index_bg_B5_smg;
  int index_bg_B6_smg;
  int index_bg_B7_smg;
  int index_bg_B8_smg;
  int index_bg_B9_smg;
  int index_bg_B10_smg;
  int index_bg_B11_smg;
  int index_bg_B12_smg;

  int index_bg_C0_smg;
  int index_bg_C1_smg;
  int index_bg_C2_smg;
  int index_bg_C3_smg;
  int index_bg_C4_smg;
  int index_bg_C5_smg;
  int index_bg_C6_smg;
  int index_bg_C7_smg;
  int index_bg_C8_smg;
  int index_bg_C9_smg;
  int index_bg_C10_smg;
  int index_bg_C11_smg;
  int index_bg_C12_smg;
  int index_bg_C13_smg;
  int index_bg_C14_smg;
  int index_bg_C15_smg;
  int index_bg_C16_smg;
  int index_bg_C9_prime_smg;
  int index_bg_C10_prime_smg;
  int index_bg_C12_prime_smg;
  int index_bg_C13_prime_smg;

  int index_bg_kinetic_D_smg;
  int index_bg_kinetic_D_prime_smg;
  int index_bg_kinetic_D_over_phiphi_smg;
  int index_bg_kinetic_D_over_phiphi_prime_smg;

  int index_bg_lambda_1_smg;
  int index_bg_lambda_2_smg;
  int index_bg_lambda_3_smg;
  int index_bg_lambda_4_smg;
  int index_bg_lambda_5_smg;
  int index_bg_lambda_6_smg;
  int index_bg_lambda_7_smg;
  int index_bg_lambda_8_smg;
  int index_bg_lambda_9_smg;
  int index_bg_lambda_10_smg;
  int index_bg_lambda_11_smg;
  int index_bg_lambda_2_prime_smg;
  int index_bg_lambda_8_prime_smg;
  int index_bg_lambda_9_prime_smg;
  int index_bg_lambda_11_prime_smg;
  int index_bg_cs2num_smg;
  int index_bg_cs2num_prime_smg;

  int index_bg_rho_tot_wo_smg; /**< total density minus scalar field */
  int index_bg_p_tot_wo_smg; /**< total pressure minus scalar field */
  int index_bg_H_prime_prime; /**< second derivative of the hubble parameter (necessary for BS perturbations equation for h'') */
  int index_bg_p_tot_wo_prime_smg; /**< derivative of the total pressure minus scalar field */
  int index_bg_p_prime_smg; /**< derivative of the pressure of the scalar field */
  int index_bg_p_tot_wo_prime_prime_smg; /**< second derivative of the total pressure minus scalar field (necessary for QSA in stable parametrization) */
  int index_bg_p_prime_prime_smg; /**< second derivative of the pressure of the scalar field (necessary for QSA in stable parametrization) */
  int index_bg_w_smg; /**< equation of state of the scalar field */
  int index_bg_mu_p_smg; /**< mu_p in EFE QSA eq. 3.2 in 2011.05713*/
  int index_bg_mu_inf_smg; /**< mu_infinity in EFE QSA eq. 4.1 in 2011.05713*/
  int index_bg_muZ_inf_smg; /**< mu_{Z,infinity} in EFE QSA eq. 4.2 in 2011.05713*/
  int index_bg_mu_p_prime_smg; /**< conformal time derivative of mu_p*/
  int index_bg_mu_inf_prime_smg; /**< conformal time derivative of mu_infinity*/
  int index_bg_muZ_inf_prime_smg; /**< conformal time derivative of mu_{Z,infinity}*/

  int index_bg_G_eff_smg; /**< G effective in the infinite k limit */
  int index_bg_slip_eff_smg; /**< slip effective in the infinite k limit */

  int index_bg_rho_ncdm1;     /**< density of first ncdm species (others contiguous) */
  int index_bg_p_ncdm1;       /**< pressure of first ncdm species (others contiguous) */
  int index_bg_pseudo_p_ncdm1;/**< another statistical momentum useful in ncdma approximation */

  int index_bg_rho_tot;       /**< Total density */
  int index_bg_p_tot;         /**< Total pressure */
  int index_bg_p_tot_prime;   /**< Conf. time derivative of total pressure */

  int index_bg_Omega_r;       /**< relativistic density fraction (\f$ \Omega_{\gamma} + \Omega_{\nu r} \f$) */

  int index_bg_Omega_de;       /**< dark energy density fraction (\f$ \Omega_{\Lambda} + \Omega_{\rm quint} + + \Omega_{\rm fld} + + \Omega_{\rm smg} \f$) */

  /* end of vector in normal format, now quantities in long format */

  int index_bg_rho_crit;      /**< critical density */
  int index_bg_Omega_m;       /**< non-relativistic density fraction (\f$ \Omega_b + \Omega_cdm + \Omega_{\nu nr} \f$) */
  int index_bg_conf_distance; /**< conformal distance (from us) in Mpc */
  int index_bg_ang_distance;  /**< angular diameter distance in Mpc */
  int index_bg_lum_distance;  /**< luminosity distance in Mpc */
  int index_bg_time;          /**< proper (cosmological) time in Mpc */
  int index_bg_rs;            /**< comoving sound horizon in Mpc */

  int index_bg_D;             /**< scale independent growth factor D(a) for CDM perturbations */
  int index_bg_f;             /**< corresponding velocity growth factor [dlnD]/[dln a] */

  int index_bg_varc_alpha;    /**< value of fine structure constant in varying fundamental constants */
  int index_bg_varc_me;      /**< value of effective electron mass in varying fundamental constants */

  int bg_size_short;  /**< size of background vector in the "short format" */
  int bg_size_normal; /**< size of background vector in the "normal format" */
  int bg_size;        /**< size of background vector in the "long format" */

  //@}

  /** @name - vectors and parameters for stable parameterization */

  //@{

  double z_gr_smg; /**< transition redshift for GR -> MG when method_gr_smg is activated at early times */

  // perturbations
  double a_file_gr_smg; /* minimum scale factor in input file */
  int stable_params_size_smg; /* stores total number of rows in input file */
  int num_stable_params; /* how many input functions*/
  int num_stable_params_derived; /* how many derived functions*/
  int num_stable_params_aux; /* how many functions in auxiliary vector*/
  double * stable_params_lna_smg; /* array of ln(a) values in input file */
  double * stable_params_smg; /* array of size stable_params_size_smg*num_stable_params containing the input Delta_Mpl, D_kin, cs2 and the derived alpha_M*/
  double * ddstable_params_smg; /* array of size stable_params_size_smg*num_stable_params containing all input functions, derivatives and derived quantities*/
  double * stable_params_derived_smg; /* array of size stable_params_size_smg*2 containing the computed alpha_B and alpha_K*/
  double * ddstable_params_derived_smg; /* array of size stable_params_size_smg*2 containing second derivatives of computed alpha_B and alpha_K*/
  double * stable_params_aux_smg; // temporary vector of size stable_params_size_smg*3 to store Delta_Mpl, dMpl and ddMpl used in array_derive_spline_table_line_to_line

  // background expansion
  double a_file_lcdm_smg; /* minimum scale factor in input file */
  int stable_wext_size_smg; /* stores total number of rows in input file */
  double * stable_wext_lna_smg; /* array of ln(a) values in input file */
  double * stable_wext_smg; /* array of size wext_size_smg containing the input w */
  double * ddstable_wext_smg; /* array of size stable_params_size_smg containing second derivatives of input w */
  double * stable_rho_smg; /* array of size stable_params_size_smg containing the integrated rho_smg */
  double * ddstable_rho_smg; /* array of size stable_params_size_smg containing second derivatives of integrated rho_smg */
  // double rho_smg_final; /* value of rho_smg at final time of integration, loga_final */
  double loga_final_rho_smg; /* used to store loga_final  */

  //@}

  /** @name - separate set of indices for vector stable_params_smg used for backward integration when stable parametrization is chosen */

  //@{

  // perturbations
  int index_stable_Delta_Mpl_smg; /* scalar field Planck mass Delta(M_pl^2) = M_pl^2 - 1 for stable parameterization */
  int index_stable_Dkin_smg;/* scalar field D_kin = alpha_K + 3/2*alpha_B^2 for stable parameterization */
  int index_stable_cs2_smg;/* scalar field speed of sound c_s^2 for stable parameterization */
  int index_stable_Mpl_running_smg; /* scalar field Planck mass running for stable parameterization (derived from M_pl^2) */
  int index_aux_Delta_Mpl_smg; /* Planck mass Delta(M_pl^2) index for auxiliary vector*/
  int index_aux_dMpl_smg; /* index used in auxiliary vector for dMpl/dlna */
  int index_aux_ddMpl_smg; /* index used in auxiliary vector for d2Mpl/dlna2 */
  int index_derived_braiding_smg; /* index for derived scalar field braiding alpha_B (derived from ODE integration) */
  int index_derived_braiding_prime_smg; /* index for derived scalar field braiding alpha_B' (derived from ODE integration) -- experimental bit */
  int index_derived_kineticity_smg; /* index for derived scalar field kineticity alpha_K (derived from backward ODE integration) */

  // background expansion
  // int index_stable_w_smg; /* scalar field equation of state for stable parameterization */
  // int index_derived_rho_smg; /* index for derived scalar field background density (derived from backward ODE integration) */

  //@}

  /** @name - background interpolation tables */

  //@{

  int bt_size;               /**< number of lines (i.e. time-steps) in the four following array */
  double * loga_table;       /**< vector loga_table[index_loga] with values of log(a) (in fact \f$ log(a/a0) \f$, logarithm of relative scale factor compared to today) */
  double * tau_table;        /**< vector tau_table[index_loga] with values of conformal time \f$ \tau \f$ (in fact \f$ a_0 c tau \f$, see normalisation conventions explained at beginning of background.c) */
  double * z_table;          /**< vector z_table[index_loga] with values of \f$ z \f$ (redshift) */
  double * background_table; /**< table background_table[index_tau*pba->bg_size+pba->index_bg] with all other quantities (array of size bg_size*bt_size) **/
  double * background_table_late; /**< late-time background table when smg stable and wext (or rho_de) parametrisations are both ON. It's used to store functions and derivatives for loga>=loga_final_bw_integration **/

  //@}

  /** @name - background backward integration tables and forward time vector for integrated rho_smg (use if expansion_model_smg == wext) */

  //@{

  int bt_bw_size;               /**< size of vector (i.e. time-steps) used to store result of backward integration for alpha_B */
  int bt_bw_rho_smg_size;               /**< size of vector (i.e. time-steps) used to store result of backward integration for rho_smg */
  double loga_final_bw_integration;
  double * loga_bw_table;       /**< vector loga_bw_table[index_loga] with values of log(a). Note that loga_bw_table[0] correspond to a=1 and loga_bw_table[bt_bw_size-1] to the smallest scale factor provided by the user */
  double * loga_bw_table_rho_smg;       /**< vector loga_bw_table_rho_smg[index_loga] with values of log(a). Note that loga_bw_table_rho_smg[0] correspond to a=1 and loga_bw_table_rho_smg[bt_bw_size-1] to the smallest scale factor provided by the user */
  double * loga_fw_table_rho_smg;       /**< vector loga_fw_table_rho_smg[index_loga] with values of log(a) */

  //@}

  /** @name - table of their second derivatives, used for spline interpolation */

  //@{

  double * d2tau_dz2_table; /**< vector d2tau_dz2_table[index_loga] with values of \f$ d^2 \tau / dz^2 \f$ (conformal time) */
  double * d2z_dtau2_table; /**< vector d2z_dtau2_table[index_loga] with values of \f$ d^2 z / d\tau^2 \f$ (conformal time) */
  double * d2background_dloga2_table; /**< table d2background_dtau2_table[index_loga*pba->bg_size+pba->index_bg] with values of \f$ d^2 b_i / d\log(a)^2 \f$ */
  double * d2background_dloga2_table_late; /**< table of second derivatives for background_table_late */

  //@}


  /** @name - all indices for the vector of background quantities to be integrated (=bi)
   *
   * Most background quantities can be immediately inferred from the
   * scale factor. Only few of them require an integration with
   * respect to conformal time (in the minimal case, only one quantity needs to
   * be integrated with time: the scale factor, using the Friedmann
   * equation). These indices refer to the vector of
   * quantities to be integrated with time.
   * {B} quantities are needed by background_functions() while {C} quantities are not.
   */

  //@{

  int index_bi_rho_dcdm;/**< {B} dcdm density */
  int index_bi_rho_dr;  /**< {B} dr density */
  int index_bi_rho_fld; /**< {B} fluid density */
  int index_bi_phi_scf;       /**< {B} scalar field value */
  int index_bi_phi_prime_scf; /**< {B} scalar field derivative wrt conformal time */

  int index_bi_logH;       /**< {B} Hubble rate factor */
  int index_bi_phi_smg;   /**< scalar field */
  int index_bi_phi_prime_smg;   /**< scalar field derivative wrt conformal time*/
  int index_bi_delta_M_pl_smg; //*> integrate the Planck mass (only in certain parameterizations **/
  int index_bi_rho_smg; //*> integrate the smg energy density (only in certain parameterizations) **/
  /* backward integrated quantities (bibw). ICs set at a=1, not at a=a_ini */
  int index_bibw_B_tilde_smg; // auxiliary \tilde{B} variable for alpha_B
  int index_bibw_dB_tilde_smg; // \tilde{B} first derivative wrt lna

  int index_bi_time;    /**< {C} proper (cosmological) time in Mpc */
  int index_bi_rs;      /**< {C} sound horizon */
  int index_bi_tau;     /**< {C} conformal time in Mpc */
  int index_bi_D;       /**< {C} scale independent growth factor D(a) for CDM perturbations. */
  int index_bi_D_prime; /**< {C} D satisfies \f$ [D''(\tau)=-aHD'(\tau)+3/2 a^2 \rho_M D(\tau) \f$ */

  int bi_B_size;        /**< Number of {B} parameters */
  int bi_size;          /**< Number of {B}+{C} parameters */
  int bi_bw_B_size;     /** MC < Number of {B} parameters which have been integrated backward in time*/

  //@}

  /** @name - flags describing the absence or presence of cosmological
      ingredients
      *
      * having one of these flag set to zero allows to skip the
      * corresponding contributions, instead of adding null contributions.
      */


  //@{

  short has_cdm;       /**< presence of cold dark matter? */
  short has_idm;       /**< presence of interacting dark matter with photons, baryons, and idr */
  short has_dcdm;      /**< presence of decaying cold dark matter? */
  short has_dr;        /**< presence of relativistic decay radiation? */
  short has_scf;       /**< presence of a scalar field? */
  short has_ncdm;      /**< presence of non-cold dark matter? */
  short has_lambda;    /**< presence of cosmological constant? */
  short has_fld;       /**< presence of fluid with constant w and cs2? */
  short has_ur;        /**< presence of ultra-relativistic neutrinos/relics? */
  short has_idr;       /**< presence of interacting dark radiation? */
  short has_smg;       /**< presence of scalar field? */
  short has_curvature; /**< presence of global spatial curvature? */
  short has_varconst;  /**< presence of varying fundamental constants? */

  //@}


  /**
   *@name - arrays related to sampling and integration of ncdm phase space distributions
   */

  //@{

  int * ncdm_quadrature_strategy; /**< Vector of integers according to quadrature strategy. */
  double ** q_ncdm_bg;  /**< Pointers to vectors of background sampling in q */
  double ** w_ncdm_bg;  /**< Pointers to vectors of corresponding quadrature weights w */
  double ** q_ncdm;     /**< Pointers to vectors of perturbation sampling in q */
  double ** w_ncdm;     /**< Pointers to vectors of corresponding quadrature weights w */
  double ** dlnf0_dlnq_ncdm; /**< Pointers to vectors of logarithmic derivatives of p-s-d */
  int * q_size_ncdm_bg; /**< Size of the q_ncdm_bg arrays */
  int * q_size_ncdm;    /**< Size of the q_ncdm arrays */
  double * factor_ncdm; /**< List of normalization factors for calculating energy density etc.*/

  //@}

  /** @name - technical parameters */

  //@{

  short shooting_failed;  /**< flag is set to true if shooting failed. */
  ErrorMsg shooting_error; /**< Error message from shooting failed. */

  short background_verbose; /**< flag regulating the amount of information sent to standard output (none if set to zero) */

  ErrorMsg error_message; /**< zone for writing error messages */

  //@}
};


/**
 * temporary parameters and workspace passed to the background_derivs function
 */

struct background_parameters_and_workspace {

  /* structures containing fixed input parameters (indices, ...) */
  struct background * pba;

  /* workspace */
  double * pvecback;

  /* workspaces used only for hi_class stable parametrization */
  double * pvec_stable_params_smg;
  double * pvecback_B;

};

/**
 * temporary parameters and workspace passed to phase space distribution function
 */

struct background_parameters_for_distributions {

  /* structures containing fixed input parameters (indices, ...) */
  struct background * pba;

  /* Additional parameters */

  /* Index of current distribution function */
  int n_ncdm;

  /* Used for interpolating in file of tabulated p-s-d: */
  int tablesize;
  double *q;
  double *f0;
  double *d2f0;
  int last_index;

};

/**************************************************************/
/* @cond INCLUDE_WITH_DOXYGEN */
/*
 * Boilerplate for C++
 */
#ifdef __cplusplus
extern "C" {
#endif

  int background_at_z(
                      struct background *pba,
                      double a_rel,
                      enum vecback_format return_format,
                      enum interpolation_method inter_mode,
                      int * last_index,
                      double * pvecback
                      );

  int background_at_tau(
                        struct background *pba,
                        double tau,
                        enum vecback_format return_format,
                        enum interpolation_method inter_mode,
                        int * last_index,
                        double * pvecback
                        );

  int background_tau_of_z(
                          struct background *pba,
                          double z,
                          double * tau
                          );

  int background_z_of_tau(
                          struct background *pba,
                          double tau,
                          double * z
                          );

  int background_functions(
                           struct background *pba,
                           double a_rel,
                           double * pvecback_B,
                           enum vecback_format return_format,
                           double * pvecback
                           );

  int background_w_fld(
                       struct background * pba,
                       double a,
                       double * w_fld,
                       double * dw_over_da_fld,
                       double * integral_fld);

  int background_varconst_of_z(
                               struct background* pba,
                               double z,
                               double* alpha,
                               double* me
                               );

  int background_init(
                      struct precision *ppr,
                      struct background *pba
                      );

  int background_free(
                      struct background *pba
                      );

  int background_free_noinput(
                              struct background *pba
                              );

  int background_free_input(
                            struct background *pba
                            );

  int background_indices(
                         struct background *pba
                         );

  int background_ncdm_distribution(
                                   void *pba,
                                   double q,
                                   double * f0
                                   );

  int background_ncdm_test_function(
                                    void *pba,
                                    double q,
                                    double * test
                                    );

  int background_ncdm_init(
                           struct precision *ppr,
                           struct background *pba
                           );

  int background_ncdm_momenta(
                              double * qvec,
                              double * wvec,
                              int qsize,
                              double M,
                              double factor,
                              double z,
                              double * n,
                              double * rho,
                              double * p,
                              double * drho_dM,
                              double * pseudo_p
                              );

  int background_ncdm_M_from_Omega(
                                   struct precision *ppr,
                                   struct background *pba,
                                   int species
                                   );

  int background_checks(
                        struct precision * ppr,
                        struct background *pba
                        );

  int background_solve(
                       struct precision *ppr,
                       struct background *pba
                       );

  int background_initial_conditions(
                                    struct precision *ppr,
                                    struct background *pba,
                                    double * pvecback,
                                    double * pvecback_integration,
                                    double * pvecback_bw_integration,
                                    double * loga_ini
                                    );

  int background_find_equality(
                               struct precision *ppr,
                               struct background *pba
                               );


  int background_output_titles(struct background * pba,
                               char titles[_MAXTITLESTRINGLENGTH_]
                               );

  int background_output_data(
                             struct background *pba,
                             int number_of_titles,
                             double *data);

  int background_derivs(
                        double loga,
                        double * y,
                        double * dy,
                        void * parameters_and_workspace,
                        ErrorMsg error_message
                        );

  int background_sources(
                         double loga,
                         double * y,
                         double * dy,
                         int index_loga,
                         void * parameters_and_workspace,
                         ErrorMsg error_message
                         );

  int background_timescale(
                           double loga,
                           void * parameters_and_workspace,
                           double * timescale,
                           ErrorMsg error_message
                           );

  int background_output_budget(
                               struct background* pba
                               );

  /** Scalar field potential and its derivatives **/
  double V_scf(
               struct background *pba,
               double phi
               );

  double dV_scf(
                struct background *pba,
                double phi
                );

  double ddV_scf(
                 struct background *pba,
                 double phi
                 );

  /** Coupling between scalar field and matter **/
  double Q_scf(
               struct background *pba,
               double phi,
               double phi_prime
               );

#ifdef __cplusplus
}
#endif

/**************************************************************/

/**
 * @name Some conversion factors and fundamental constants needed by background module:
 */

//@{

#define _Mpc_over_m_ 3.085677581282e22  /**< conversion factor from meters to megaparsecs */
/* remark: CAMB uses 3.085678e22: good to know if you want to compare  with high accuracy */

#define _Gyr_over_Mpc_ 3.06601394e2 /**< conversion factor from megaparsecs to gigayears
                                       (c=1 units, Julian years of 365.25 days) */
#define _c_ 2.99792458e8            /**< c in m/s */
#define _G_ 6.67428e-11             /**< Newton constant in m^3/Kg/s^2 */
#define _eV_ 1.602176487e-19        /**< 1 eV expressed in J */

/* parameters entering in Stefan-Boltzmann constant sigma_B */
#define _k_B_ 1.3806504e-23
#define _h_P_ 6.62606896e-34
/* remark: sigma_B = 2 pi^5 k_B^4 / (15h^3c^2) = 5.670400e-8
   = Stefan-Boltzmann constant in W/m^2/K^4 = Kg/K^4/s^3 */

//@}

/**
 * @name Some limits on possible background parameters
 */

//@{

#define _h_BIG_ 1.5            /**< maximal \f$ h \f$ */
#define _h_SMALL_ 0.3         /**< minimal \f$ h \f$ */
#define _omegab_BIG_ 0.039    /**< maximal \f$ omega_b \f$ */
#define _omegab_SMALL_ 0.005  /**< minimal \f$ omega_b \f$ */

//@}

/**
 * @name Some limits imposed in other parts of the module:
 */

//@{

#define _SCALE_BACK_ 0.1  /**< logarithmic step used when searching
                             for an initial scale factor at which ncdm
                             are still relativistic */

#define _PSD_DERIVATIVE_EXP_MIN_ -30 /**< for ncdm, for accurate computation of dlnf0/dlnq, q step is varied in range specified by these parameters */
#define _PSD_DERIVATIVE_EXP_MAX_ 2  /**< for ncdm, for accurate computation of dlnf0/dlnq, q step is varied in range specified by these parameters */

#define _zeta3_ 1.2020569031595942853997381615114499907649862923404988817922 /**< for quandrature test function */
#define _zeta5_ 1.0369277551433699263313654864570341680570809195019128119741 /**< for quandrature test function */

//@}


#endif
/* @endcond */
