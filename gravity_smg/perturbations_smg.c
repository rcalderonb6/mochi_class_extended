#include "perturbations_smg.h"


//Here we do the smg tests. It is important to have them after perturb_indices_of_perturbs because we need
//quantities as k_min and k_max.
int perturb_tests_smg(struct precision * ppr,
                      struct background * pba,
                      struct perturbs * ppt) {

        class_test(ppt->gauge == newtonian,
                   ppt->error_message,
                   "Asked for scalar modified gravity AND Newtonian gauge. Not yet implemented");

        class_test(ppr->a_ini_test_qs_smg < ppr->a_ini_over_a_today_default,
                   ppt->error_message,
                   "The initial time for testing the QS approximation (qs_smg) must be bigger than the background initial time (a_ini_test_qs_smg>=a_ini_over_a_today_default).");

        if ( ppt->pert_initial_conditions_smg == gravitating_attr ) {
                class_test_except((ppt->has_cdi == _TRUE_) || (ppt->has_bi == _TRUE_) || (ppt->has_nid == _TRUE_) || (ppt->has_niv == _TRUE_),
                                  ppt->error_message,
                                  perturb_free_nosource(ppt),
                                  "Isocurvature initial conditions for early modified gravity (Gravitating Attractor) not implemented.");
        }


        // TODO: think of some suitable tests for the scalar field

        if (ppt->method_qs_smg == automatic) {
                //Check if at the initial time all the k modes start with the same kind of qs_smg approximation
                class_call_except(perturb_test_ini_qs_smg(ppr,
                                                          pba,
                                                          ppt,
                                                          ppt->k[ppt->index_md_scalars][0],
                                                          ppt->k[ppt->index_md_scalars][ppt->k_size[ppt->index_md_scalars]-1],
                                                          ppr->a_ini_test_qs_smg),
                                  ppt->error_message,
                                  ppt->error_message,
                                  perturb_free_nosource(ppt));
        }

        if (!((ppt->method_qs_smg == automatic) && (ppt->initial_approx_qs_smg==_TRUE_))) {

                // if scalar is dynamical or always quasi-static, test for stability at the initial time.
                // Only in the case it is QS because of a trigger test (through "automatic" method_qs),
                // we already know mass is positive and therefore can assume it is stable, so skip this.

                if( ppt->pert_initial_conditions_smg == gravitating_attr) {
                        // If we are in gravitating_attr ICs, make sure the standard solution is dominant at some early redshift.
                        // If it is not, curvature is not conserved and we have lost the connection between the amplitude from inflation and
                        // the initial amplitude supplied to hi_class.
                        class_call_except(perturb_test_ini_grav_ic_smg(ppr,
                                                                       pba,
                                                                       ppt),
                                          ppt->error_message,
                                          ppt->error_message,
                                          perturb_free_nosource(ppt));
                }
                else if( ppt->pert_initial_conditions_smg == ext_field_attr) {
                        //If we have the ext_field_attr, test for tachyon instability in RD before pert initialisation
                        // If have it, fail, because we can't set the ICs properly

                        class_call_except(perturb_test_ini_extfld_ic_smg(ppr,
                                                                         pba,
                                                                         ppt),
                                          ppt->error_message,
                                          ppt->error_message,
                                          perturb_free_nosource(ppt));
                }
        }

        return _SUCCESS_;

}

int perturb_qs_functions_at_tau_and_k_qs_smg(struct background * pba,
                                             struct perturbs * ppt,
                                             double k,
                                             double tau,
                                             double *mass2,
                                             double *mass2_p,
                                             double *rad2,
                                             double *friction,
                                             double *slope) {

        /* Definition of local variables */
        double mass2_qs, mass2_qs_p, rad2_qs, friction_qs, slope_qs;
        double * pvecback;
        int first_index_back;

        class_alloc(pvecback,pba->bg_size*sizeof(double),ppt->error_message);
        class_call(background_at_tau(pba,
                                     tau,
                                     pba->normal_info,
                                     pba->inter_normal,
                                     &first_index_back,
                                     pvecback),
                   pba->error_message,
                   ppt->error_message);

        double delM2, M2, kin, bra, ten, run, beh;
        double res, cD, cK, cB, cH;
        double c0, c1, c2, c3, c4, c5, c6, c7, c8;
        double c9, c10, c11, c12, c13, c14, c15, c16;
        double c9_p, c10_p, c12_p, c13_p;
        double res_p, cD_p, cB_p, cH_p;
        double x_prime_qs_smg_num, x_prime_qs_smg_den;
        double a, H, rho_tot, p_tot, rho_smg, p_smg, rho_r;
        double k2 = k*k;

        a = pvecback[pba->index_bg_a];
        H = pvecback[pba->index_bg_H];
        rho_r = pvecback[pba->index_bg_rho_g] + pvecback[pba->index_bg_rho_ur];
        rho_tot = pvecback[pba->index_bg_rho_tot_wo_smg];
        p_tot = pvecback[pba->index_bg_p_tot_wo_smg];
        rho_smg = pvecback[pba->index_bg_rho_smg];
        p_smg = pvecback[pba->index_bg_p_smg];

        class_call(
                get_gravity_coefficients_smg(
                        ppt, pba, pvecback,
                        &delM2, &M2, &kin, &bra, &ten, &run, &beh, &res,
                        &cD, &cK, &cB, &cH, &c0, &c1, &c2, &c3,
                        &c4, &c5, &c6, &c7, &c8, &c9, &c10, &c11,
                        &c12, &c13, &c14, &c15, &c16, &res_p, &cD_p, &cB_p,
                        &cH_p, &c9_p, &c10_p, &c12_p, &c13_p
                        ),
                ppt->error_message,
                ppt->error_message);


        mass2_qs = -(c12 + c13*k2*pow(a*H,-2))/cD;

        mass2_qs_p =
                -(
                        +c12_p - c12*cD_p/cD
                        + (c13_p - c13*cD_p/cD + (rho_tot + rho_smg + 3.*p_tot + 3.*p_smg)*c13*a/H)*pow(a*H,-2)*k2
                        )/cD;

        rad2_qs = 3.*mass2_qs*pow(H,4)*pow(rho_r,-2)*pow(a*H,2)/k2;

        friction_qs = -(c11 - c3*k2*pow(a*H,-2))/cD;

        slope_qs = -1./4.*(1. - 2.*friction_qs + 3.*(p_tot + p_smg)/(rho_tot + rho_smg) - mass2_qs_p/mass2_qs/a/H);

        *mass2 = mass2_qs;
        *mass2_p = mass2_qs_p;
        *rad2 = rad2_qs;
        *friction = friction_qs;
        *slope = slope_qs;

        free(pvecback);

        return _SUCCESS_;

}

int perturb_test_at_k_qs_smg(struct precision * ppr,
                             struct background * pba,
                             struct perturbs * ppt,
                             double k,
                             double tau,
                             int *approx) {

        //Define local variables
        double mass2_qs, mass2_qs_p, rad2_qs, friction_qs, slope_qs;

        perturb_qs_functions_at_tau_and_k_qs_smg(
                pba,
                ppt,
                k,
                tau,
                &mass2_qs,
                &mass2_qs_p,
                &rad2_qs,
                &friction_qs,
                &slope_qs);

        double tau_fd;
        short proposal;

        class_call(background_tau_of_z(pba,
                                       ppr->z_fd_qs_smg,
                                       &tau_fd),
                   pba->error_message,
                   ppt->error_message);
        //Approximation
        if ((mass2_qs > pow(ppr->trigger_mass_qs_smg,2)) && (rad2_qs > pow(ppr->trigger_rad_qs_smg,2))) {
                proposal = _TRUE_;
        }
        else {
                proposal = _FALSE_;
        }
        if (tau <= tau_fd) {
                *approx = proposal;
        }
        else {
                *approx = _FALSE_;
        }

        return _SUCCESS_;

}

int perturb_test_ini_qs_smg(struct precision * ppr,
                            struct background * pba,
                            struct perturbs * ppt,
                            double k_min,
                            double k_max,
                            double a_ini) {
        //Define local variables
        double * pvecback;
        int first_index_back;
        double tau;
        int approx_k_min, approx_k_max;

        //Get background quantities at a_ini
        class_call(background_tau_of_z(pba,
                                       1./a_ini-1.,
                                       &tau),
                   pba->error_message,
                   ppt->error_message);

        //Approximation for k_min
        perturb_test_at_k_qs_smg(
                ppr,
                pba,
                ppt,
                k_min,
                tau,
                &approx_k_min
                );

        //Approximation for k_max
        perturb_test_at_k_qs_smg(
                ppr,
                pba,
                ppt,
                k_max,
                tau,
                &approx_k_max
                );

        class_test_except(approx_k_min != approx_k_max,
                          ppt->error_message,
                          free(pvecback),
                          "\n All the k modes should start evolving with the same type of initial conditions (either fully_dynamic or quasi_static).\n This is not the case at a = %e. Try to decrease a_ini_over_a_today_default.\n", ppr->a_ini_over_a_today_default);

        ppt->initial_approx_qs_smg = approx_k_min;

        free(pvecback);

        return _SUCCESS_;

}

int perturb_find_scheme_qs_smg(struct precision * ppr,
                               struct background * pba,
                               struct perturbs * ppt,
                               struct perturb_workspace * ppw,
                               double k,
                               double tau_ini,
                               double tau_end) {

        int size_sample = ppr->n_max_qs_smg;

        double * tau_sample;
        double * mass2_sample;
        double * rad2_sample;
        double * slope_sample;

        class_alloc(tau_sample,size_sample*sizeof(double),ppt->error_message);
        class_alloc(mass2_sample,size_sample*sizeof(double),ppt->error_message);
        class_alloc(rad2_sample,size_sample*sizeof(double),ppt->error_message);
        class_alloc(slope_sample,size_sample*sizeof(double),ppt->error_message);

        /**
         * Input: background table
         * Output: sample of the time, mass, decaying rate of the oscillations (slope)
         *   and radiation density.
         **/
        sample_functions_qs_smg(
                ppr,
                pba,
                ppt,
                k,
                tau_ini,
                tau_end,
                tau_sample,
                mass2_sample,
                rad2_sample,
                slope_sample,
                &size_sample
                );


        int * approx_sample;

        class_alloc(approx_sample,size_sample*sizeof(int),ppt->error_message);

        /**
         * Input: sample of the time, mass and radiation density
         * Output: sample of the approx scheme
         **/
        functions_to_approx_qs_smg(
                ppr,
                pba,
                ppt,
                tau_ini,
                tau_end,
                tau_sample,
                mass2_sample,
                rad2_sample,
                approx_sample,
                size_sample
                );

        free(mass2_sample);
        free(rad2_sample);


        double * tau_array;
        double * slope_array;
        int * approx_array;
        int size_array = size_sample;

        class_alloc(tau_array,size_array*sizeof(double),ppt->error_message);
        class_alloc(slope_array,size_array*sizeof(double),ppt->error_message);
        class_alloc(approx_array,size_array*sizeof(int),ppt->error_message);

        /**
         * Input: sample of the time, slope and approximation scheme
         *   at small time interval
         * Output: arrays containing the time, the slope and the approximation
         * scheme only when it changes
         **/
        shorten_first_qs_smg(
                tau_sample,
                slope_sample,
                approx_sample,
                size_sample,
                tau_array,
                slope_array,
                approx_array,
                &size_array,
                tau_end
                );

        free(tau_sample);
        free(slope_sample);
        free(approx_sample);

        /**
         * Input: arrays with time, slope and approximation schemes
         * Output: arrays with time and approximation scheme corrected with the slope
         **/
        correct_with_slope_qs_smg(
                ppr,
                pba,
                ppt,
                tau_ini,
                tau_end,
                tau_array,
                slope_array,
                approx_array,
                size_array
                );

        free(slope_array);

        double * tau_scheme;
        int * approx_scheme;
        int size_scheme = size_array;

        class_alloc(tau_scheme,size_scheme*sizeof(double),ppt->error_message);
        class_alloc(approx_scheme,size_scheme*sizeof(int),ppt->error_message);

        /**
         * Input: arrays of time and approximation after correcting with the slope
         *   (there is the possibility that the same number in approx_array is repeated)
         * Output: shortened arrays of time and approximation
         **/
        shorten_second_qs_smg(
                tau_array,
                approx_array,
                size_array,
                tau_scheme,
                approx_scheme,
                &size_scheme
                );

        free(tau_array);
        free(approx_array);

        /**
         * Input: real approx_scheme and tau_scheme
         * Output: approx scheme (ppw->tau_scheme_qs_smg) adjusted to fit the implemented one
         **/
        fit_real_scheme_qs_smg(
                tau_end,
                approx_scheme,
                tau_scheme,
                size_scheme,
                ppw->tau_scheme_qs_smg
                );

        free(tau_scheme);
        free(approx_scheme);

//   // DEBUG: Initial and final times
//   printf("6 - Interval tau       = {%.1e, %.1e}\n", tau_ini, tau_end);
//   printf("7 - k mode             = {%.1e}\n", k);


        return _SUCCESS_;

}


int sample_functions_qs_smg(struct precision * ppr,
                            struct background * pba,
                            struct perturbs * ppt,
                            double k,
                            double tau_ini,
                            double tau_end,
                            double * tau_sample,
                            double * mass2_sample,
                            double * rad2_sample,
                            double * slope_sample,
                            int *size_sample) {

        /* Definition of local variables */
        double mass2_qs, mass2_qs_p, rad2_qs, friction_qs, slope_qs;
        double tau = tau_ini;
        double delta_tau = (tau_end - tau_ini)/ppr->n_max_qs_smg;
        int count = 0;


        /* Scan the time evolution and build several arrays containing
        * interesting quantities for the quasi-static approximation */
        while (tau < tau_end) {

                perturb_qs_functions_at_tau_and_k_qs_smg(
                        pba,
                        ppt,
                        k,
                        tau,
                        &mass2_qs,
                        &mass2_qs_p,
                        &rad2_qs,
                        &friction_qs,
                        &slope_qs);

//     DEBUG: To debug uncomment this and define a convenient function of time for each of these quantities
//     double x = (tau - tau_ini)/(tau_end - tau_ini);
//     mass2_qs = 1.5 + cos(10*_PI_*x);
//     rad2_qs = 1.;
//     slope_qs = 1.;

                tau_sample[count] = tau;
                mass2_sample[count] = mass2_qs;
                rad2_sample[count] = rad2_qs;
                slope_sample[count] = slope_qs;

                delta_tau = fabs(2.*mass2_qs/mass2_qs_p)/sqrt(ppr->n_min_qs_smg*ppr->n_max_qs_smg);
                delta_tau = MIN(delta_tau, (tau_end - tau_ini)/ppr->n_min_qs_smg);
                delta_tau = MAX(delta_tau, (tau_end - tau_ini)/ppr->n_max_qs_smg);

                tau += delta_tau;
                count += 1;

        }

        *size_sample = count;

        return _SUCCESS_;

}


int functions_to_approx_qs_smg(struct precision * ppr,
                               struct background * pba,
                               struct perturbs * ppt,
                               double tau_ini,
                               double tau_end,
                               double * tau_sample,
                               double * mass2_sample,
                               double * rad2_sample,
                               int * approx_sample,
                               int size_sample
                               ) {


        // Convert the input parameter z_fd into the corresponding conformal time
        double tau_fd;
        short proposal;

        class_call(background_tau_of_z(pba,
                                       ppr->z_fd_qs_smg,
                                       &tau_fd),
                   pba->error_message,
                   ppt->error_message);

        int i;
        for (i = 0; i < size_sample; i++) {

                if ((mass2_sample[i] > pow(ppr->trigger_mass_qs_smg,2)) && (rad2_sample[i] > pow(ppr->trigger_rad_qs_smg,2))) {
                        proposal = 1;
                }
                else {
                        proposal = 0;
                }

                if (tau_sample[i] <= tau_fd) {
                        approx_sample[i] = proposal;
                }
                else {
                        approx_sample[i] = 0;
                }

        }

        return _SUCCESS_;

}


int shorten_first_qs_smg(double * tau_sample,
                         double * slope_sample,
                         int * approx_sample,
                         int size_sample,
                         double * tau_array,
                         double * slope_array,
                         int * approx_array,
                         int *size_array,
                         double tau_end) {

        int i, j, count = 0;
        int last_switch = 0;
        int this_switch = 0;
        double slope_weighted;

        tau_array[0] = tau_sample[0];
        approx_array[0] = approx_sample[0];

        for (i = 1; i < size_sample; i++) {
                if (approx_sample[i] != approx_array[count]) {

                        count += 1;
                        // Shorten approximation scheme
                        approx_array[count] = approx_sample[i];

                        // Shorten time
                        if (approx_array[count-1] < approx_array[count]) {
                                this_switch = i;
                        }
                        else {
                                this_switch = i-1;
                        }
                        tau_array[count] = tau_sample[this_switch];

                        // Shorten slope
                        slope_weighted = 0.;
                        for (j = last_switch; j < this_switch; j++) {
                                slope_weighted += slope_sample[j]*(tau_sample[j+1] - tau_sample[j]);
                        }
                        slope_array[count -1] = slope_weighted/(tau_sample[this_switch] - tau_sample[last_switch]);
                        last_switch = this_switch;
                }
        }

        // Shorten slope last element
        slope_weighted = 0.;
        for (i = last_switch; i < size_sample-1; i++) {
                slope_weighted += slope_sample[i]*(tau_sample[i+1] - tau_sample[i]);
        }
        slope_array[count] = (slope_weighted + slope_sample[size_sample-1]*(tau_end - tau_sample[size_sample-1]))/(tau_end - tau_sample[last_switch]);

        *size_array = count + 1;

        return _SUCCESS_;

}


int correct_with_slope_qs_smg(struct precision * ppr,
                              struct background * pba,
                              struct perturbs * ppt,
                              double tau_ini,
                              double tau_end,
                              double * tau_array,
                              double * slope_array,
                              int * approx_array,
                              int size_array) {


        double * pvecback;
        int first_index_back;
        int i, j, count;
        for (i = 1; i < size_array; i++) {
                if ((approx_array[i-1] == 0) && (approx_array[i] == 1)) {

                        // Routine to calculate the time interval necessary to relax the oscillations
                        class_alloc(pvecback,pba->bg_size*sizeof(double),ppt->error_message);
                        class_call(background_at_tau(pba,
                                                     tau_array[i],
                                                     pba->short_info,
                                                     pba->inter_normal,
                                                     &first_index_back,
                                                     pvecback),
                                   pba->error_message,
                                   ppt->error_message);

                        double a_final = pvecback[pba->index_bg_a] * pow(ppr->eps_s_qs_smg, -1./slope_array[i]);
                        double tau_final;

                        class_call(background_tau_of_z(pba,
                                                       1./a_final-1.,
                                                       &tau_final),
                                   pba->error_message,
                                   ppt->error_message);

                        double delta_tau = tau_final - tau_array[i];

                        // Adjust time and approx to take into account the oscillations
                        double next_tau;
                        if (i+1<size_array) {
                                next_tau = tau_array[i+1];
                        }
                        else {
                                next_tau = tau_end;
                        }

                        if (tau_array[i] + delta_tau < next_tau) {
                                tau_array[i] += delta_tau;
                        }
                        else {
                                approx_array[i] = 0;
                        }

                        free(pvecback);
                }
        }

        return _SUCCESS_;

}


int shorten_second_qs_smg(double * tau_array,
                          int * approx_array,
                          int size_array,
                          double * tau_scheme,
                          int * approx_scheme,
                          int *size_scheme) {

        tau_scheme[0] = tau_array[0];
        approx_scheme[0] = approx_array[0];
        int i, j = 0;

        for (i = 0; i < size_array; i++) {
                if (approx_array[i] != approx_scheme[j]) {
                        j += 1;
                        approx_scheme[j] = approx_array[i];
                        tau_scheme[j] = tau_array[i];
                }
        }

        *size_scheme = j + 1;

        return _SUCCESS_;

}


int fit_real_scheme_qs_smg(double tau_end,
                           int * approx_scheme,
                           double * tau_scheme,
                           int size_scheme,
                           double * tau_export
                           ) {

        /* Definition of local variables */
        int implemented_scheme[] = _VALUES_QS_SMG_FLAGS_;
        int size_implemented_scheme = sizeof(implemented_scheme)/sizeof(int);

        int i, j;
        int start_position = 0;
        short scheme_fits = _FALSE_;

//   // DEBUG: print the implemented scheme
//   int count;
//   printf("1 - Implemented scheme = {");
//   for (count = 0; count < size_implemented_scheme; count++) {
//     printf("%d", implemented_scheme[count]);
//     if (count < size_implemented_scheme - 1) {
//       printf(", ");
//     }
//   }
//   printf("}\n");
//   // DEBUG: print the real scheme
//   printf("2 - Real scheme        = {");
//   for (count = 0; count < size_scheme; count++) {
//     printf("%d", approx_scheme[count]);
//     if (count < size_scheme - 1) {
//       printf(", ");
//     }
//   }
//   printf("}\n");


        while (scheme_fits == _FALSE_) {

                /* Check if the real approximation scheme fits the implemented one */
                for (i = 0; i < size_implemented_scheme - size_scheme + 1; i++) {
                        j = 0;
                        while (j < size_scheme - 1) {
                                if (approx_scheme[j] == implemented_scheme[i + j]) {
                                        j += 1;
                                }
                                else {
                                        break;
                                }
                        }
                        if ((j == size_scheme - 1) && (approx_scheme[j] == implemented_scheme[i + j])) {
                                start_position = i;
                                scheme_fits = _TRUE_;
                                break;
                        }
                }

                /* Shorten the real approximation scheme */
                if (scheme_fits == _FALSE_) {
                        if ((approx_scheme[size_scheme - 2]==0) && (approx_scheme[size_scheme - 1]==1)) {
                                size_scheme += -1;
                        }
                        else if ((approx_scheme[size_scheme - 3]==0) && (approx_scheme[size_scheme - 2]==1) && (approx_scheme[size_scheme - 1]==0)) {
                                size_scheme += -2;
                        }
                }
        }

        /* Generate the vector of times at which the approximation switches */
        for (i = 0; i < size_implemented_scheme; i++) {
                tau_export[i] = -1.;
        }

        for (i = 0; i < size_scheme; i++) {
                tau_export[start_position + i] = tau_scheme[i];
        }

        for (i = start_position + size_scheme; i < size_implemented_scheme; i++) {
                tau_export[i] = tau_end + 1.; // The +1 is here to make the final elements larger than everything else
        }

//   // DEBUG: print the fitted scheme
//   printf("3 - Fitted scheme      = {");
//   for (count = 0; count < size_scheme; count++) {
//     printf("%d", approx_scheme[count]);
//     if (count < size_scheme - 1) {
//       printf(", ");
//     }
//   }
//   printf("}\n");
//   // DEBUG: print the real tau switches
//   printf("4 - Real tau           = {");
//   for (count = 0; count < size_scheme; count++) {
//     printf("%.1e", tau_scheme[count]);
//     if (count < size_scheme - 1) {
//       printf(", ");
//     }
//   }
//   printf("}\n");
//   // DEBUG: print the tau switches after the fitting
//   printf("5 - Fitted tau         = {");
//   for (count = 0; count < size_implemented_scheme; count++) {
//     printf("%.1e", tau_export[count]);
//     if (count < size_implemented_scheme - 1) {
//       printf(", ");
//     }
//   }
//   printf("}\n");

        return _SUCCESS_;
}

/*
 * Test for stability of solutions in RD before initialisation of
 * perturbations: if standard solution not stable, cannot set ICs properly.
 */
int perturb_test_ini_grav_ic_smg(struct precision * ppr,
                                 struct background * pba,
                                 struct perturbs * ppt){
// test stability of gravitating_attr ICs

        double kin, bra, run, ten, DelM2, Omx, wx;
        double c3, c2, c1,c0, den1, den2, ic_regulator_smg;
        double tau_ini, z_ref;
        int i;
        double fastest_growth, wouldbe_adiab;
        double * pvecback;
        int first_index_back;
        double sols[3];
        int complex;

        class_alloc(pvecback,pba->bg_size*sizeof(double),ppt->error_message);

        z_ref = ppr->pert_ic_ini_z_ref_smg;

        class_call(background_tau_of_z(pba, z_ref,&tau_ini),
                   pba->error_message,
                   ppt->error_message);

        class_call(background_at_tau(pba,
                                     tau_ini,
                                     pba->long_info,
                                     pba->inter_normal,
                                     &first_index_back,
                                     pvecback),
                   pba->error_message,
                   ppt->error_message);

        // define alphas
        wx = pvecback[pba->index_bg_p_smg]/pvecback[pba->index_bg_rho_smg];
        Omx = pvecback[pba->index_bg_rho_smg]/pow(pvecback[pba->index_bg_H],2);
        kin = pvecback[pba->index_bg_kineticity_smg];
        bra = pvecback[pba->index_bg_braiding_smg];
        run = pvecback[pba->index_bg_mpl_running_smg];
        ten = pvecback[pba->index_bg_tensor_excess_smg];
        DelM2 = pvecback[pba->index_bg_delta_M2_smg];//M2-1

        /* Determine the solutions
         *
         *   h = C * tau^2+x
         *
         * where n is the solution of
         *
         *   c3 x^3 + c2 x^2 + c1 x + c0 = 0
         *
         * Note: if complex solutions then take the real part for the test
         * These solutions are exact when run=0. If not, then they are approximate.
         * These coefficients were obtain by solving the radiation+smg system
         * to obtain a 5th-order ODE for h. Removing two gauge modes, leaves
         * a cubic with three solutions relevant in the k->0 limit.
         * Note that we approximate the ci to O(run).
         */

        // Note: The denominators in the expressions below can be zero. We try to trap this and regulate.
        // We assume that M*^2>0 and D>0 which are tested for in the background routine.
        // Doing this gives wrong ICs, but it's better than segmentation faults.

        ic_regulator_smg =  ppr->pert_ic_regulator_smg;//  read in the minimum size that will get regulated
        ic_regulator_smg *= fabs(kin)+fabs(bra)+fabs(ten); //  scale it relative to the alphas

        c3  =   1.;

        c2  =   5. + 2.*run;

        den1 = (3.*bra*ten + kin*(2. + ten));

        if(ic_regulator_smg>0 &&(fabs(den1)<ic_regulator_smg)) {
                den1 = copysign(ic_regulator_smg,den1);
        }

        den2 =  4.*(9.*bra*(1. + DelM2) + (1. + DelM2)*kin - 12.*(DelM2 + Omx))*(3.*pow(bra,2.)*
                                                                                 (1. + DelM2) + 2.*kin*(DelM2 + Omx))*(-6.*(DelM2 + Omx)*(-2. + ten) + 9.*bra*(1. + DelM2)*(-1. + ten) + 2.*(1. + DelM2)*
                                                                                                                       kin*(1. + ten));

        if(ic_regulator_smg>0 &&(fabs(den2)<ic_regulator_smg)) {
                den2 = copysign(ic_regulator_smg,den2);
        }

        c2  +=  ((-1. + Omx)*run*(27.*pow(bra,4)*pow(1. + DelM2,2.)*ten*(432. - 373.*ten + 6.*pow(ten,2.)) + 9.*pow(bra,3.)*(1. + DelM2)*
                                  (864.*(DelM2 + Omx)*(-2. + ten)*ten + (1. + DelM2)*kin*(864. - 698.*ten - 329.*pow(ten,2.) + 6.*pow(ten,3.))) -
                                  3.*pow(bra,2.)*(1. + DelM2)*kin*(3456.*(DelM2 + Omx) - 4320.*(DelM2 + Omx)*ten + 6.*(1. + 446.*DelM2 + 445.*Omx)*
                                                                   pow(ten,2.) - 36.*(DelM2 + Omx)*pow(ten,3.) + (1. + DelM2)*kin*(768. + 227.*ten - 259.*pow(ten,2.) + 12.*pow(ten,3.))) -
                                  2.*pow(kin,2.)*(-6.*(DelM2 + Omx)*(-768.*(DelM2 + Omx) + (-1. + 191.*DelM2 + 192.*Omx)*pow(ten,2.)) + pow(1. + DelM2,2.)*
                                                  pow(kin,2.)*(-14. - 19.*ten - 4.*pow(ten,2.) + pow(ten,3.)) - (1. + DelM2)*kin*(-384.*(DelM2 + Omx) +
                                                                                                                                  (1. - 851.*DelM2 - 852.*Omx)*ten + (1. - 317.*DelM2 - 318.*Omx)*pow(ten,2.) + 6.*(DelM2 + Omx)*pow(ten,3.))) -
                                  6.*bra*kin*(-1152.*pow(DelM2 + Omx,2.)*(-2. + ten)*ten + pow(1. + DelM2,2.)*pow(kin,2.)*(-32. - 99.*ten - 40.*pow(ten,2.) +
                                                                                                                           3.*pow(ten,3.)) - (1. + DelM2)*kin*(1440.*(DelM2 + Omx) - 2.*(1. + 325.*DelM2 + 324.*Omx)*ten + (1. - 905.*DelM2 - 906.*Omx)*
                                                                                                                                                               pow(ten,2.) + 12.*(DelM2 + Omx)*pow(ten,3.)))))/(den2*den1);

        c1  =   (9*pow(bra,3)*pow(1 + DelM2,2)*(6 + 5*run)*ten + 3*pow(bra,2)*(1 + DelM2)*(-12*(-1 + Omx)*(-3 + run)*ten +
                                                                                           (1 + DelM2)*kin*(6 + 5*run)*(2 + ten)) + 6*bra*(-24*(-1 + Omx)*(DelM2 + Omx)*ten + (1 + DelM2)*kin*
                                                                                                                                           (12*(-1 + Omx) + (-2 + 6*DelM2 + 8*Omx + 5*(1 + DelM2)*run)*ten)) + 2*kin*((1 + DelM2)*kin*
                                                                                                                                                                                                                      (2*(2 + 3*DelM2 + Omx) + 5*(1 + DelM2)*run)*(2 + ten) - 12*(-1 + Omx)*((1 + DelM2)*run*ten + 2*(DelM2 + Omx)
                                                                                                                                                                                                                                                                                             *(2 + ten))))/(pow(1 + DelM2,2)*(3*pow(bra,2) + 2*kin)*den1);

        den2 = 4.*(1 + DelM2)*(9*bra*(1 + DelM2) + (1 + DelM2)*kin - 12*(DelM2 + Omx))*
               (3*pow(bra,2)*(1 + DelM2) + 2*kin*(DelM2 + Omx))*(-6*(DelM2 + Omx)*(-2 + ten) + 9*bra*(1 + DelM2)*(-1 + ten) +
                                                                 2*(1 + DelM2)*kin*(1 + ten));

        if(ic_regulator_smg>0 && (fabs(den2)<ic_regulator_smg)) {
                den2 = copysign(ic_regulator_smg,den2);
        }


        c1  +=  ((-1 + Omx)*run*(135*pow(bra,4)*pow(1 + DelM2,3)*ten*(288 - 229*ten + 6*pow(ten,2)) + 9*pow(bra,3)*
                                 pow(1 + DelM2,2)*(2880*(DelM2 + Omx)*(-2 + ten)*ten + (1 + DelM2)*kin*(3744 - 1780*ten - 1855*pow(ten,2) +
                                                                                                        66*pow(ten,3))) + 2*kin*(3456*pow(DelM2 + Omx,3)*(-2 + ten)*ten + 6*(1 + DelM2)*kin*(DelM2 + Omx)*(-2112*(DelM2 + Omx) -
                                                                                                                                                                                                           4*(1 + 25*DelM2 + 24*Omx)*ten + 3*(-1 + 95*DelM2 + 96*Omx)*pow(ten,2)) - pow(1 + DelM2,3)*pow(kin,3)*
                                                                                                                                 (-14 - 19*ten - 4*pow(ten,2) + pow(ten,3)) + pow(1 + DelM2,2)*pow(kin,2)*(-528*(DelM2 + Omx) +
                                                                                                                                                                                                           (1 - 1523*DelM2 - 1524*Omx)*ten + (1 - 545*DelM2 - 546*Omx)*pow(ten,2) + 18*(DelM2 + Omx)*pow(ten,3))) +
                                 3*pow(bra,2)*pow(1 + DelM2,2)*kin*((1 + DelM2)*kin*(-1296 - 2087*ten - 449*pow(ten,2) + 36*pow(ten,3)) +
                                                                    6*(-3072*(DelM2 + Omx) + 1532*(DelM2 + Omx)*ten + (-5 + 28*DelM2 + 33*Omx)*pow(ten,2) + 18*(DelM2 + Omx)*pow(ten,3))) -
                                 6*bra*(1 + DelM2)*kin*(576*pow(DelM2 + Omx,2)*(-4 + 5*ten) + pow(1 + DelM2,2)*pow(kin,2)*(-4 - 61*ten - 32*pow(ten,2) +
                                                                                                                           pow(ten,3)) - (1 + DelM2)*kin*(3552*(DelM2 + Omx) - 4*(1 + 121*DelM2 + 120*Omx)*ten - (1 + 1279*DelM2 + 1278*Omx)*
                                                                                                                                                          pow(ten,2) + 36*(DelM2 + Omx)*pow(ten,3)))))/(den2*den1);


        c0  =   (24*(-1 + Omx)*run*(4*kin*Omx - 3*pow(bra,2)*(-2 + ten) - DelM2*(3*pow(bra,2) + 2*kin)*(-2 + ten) +
                                    2*kin*(-2 + Omx)*ten + 6*bra*(-1 + Omx)*ten))/(pow(1 + DelM2,2)*(3*pow(bra,2) + 2*kin)*den1);

        den2 = (9*bra*(-1 + ten) + 2*(kin + 6*Omx + kin*ten - 3*Omx*ten) + DelM2*(9*bra*(-1 + ten) +
                                                                                  2*(6 + kin - 3*ten + kin*ten)));

        if(ic_regulator_smg>0 && (fabs(den2)<ic_regulator_smg)) {
                den2 = copysign(ic_regulator_smg,den2);
        }

        c0  +=  -((-1 + Omx)*run*(9*pow(bra,3)*(-288 + 98*ten + 119*pow(ten,2) + 6*pow(ten,3)) + 6*bra*(288*Omx*(-2 + ten)*
                                                                                                        ten + pow(kin,2)*(16 + 85*ten + 48*pow(ten,2) + 3*pow(ten,3)) + kin*(288*Omx + (314 - 216*Omx)*ten -
                                                                                                                                                                             (163 + 246*Omx)*pow(ten,2) - 12*(-1 + Omx)*pow(ten,3))) + 3*pow(bra,2)*(kin*(-480 + 335*ten + 383*pow(ten,2) +
                                                                                                                                                                                                                                                          18*pow(ten,3)) - 6*(-192*Omx + 48*(-3 + Omx)*ten + (85 + 83*Omx)*pow(ten,2) + 6*(-1 + Omx)*pow(ten,3))) +
                                  2*kin*(6*ten*(-192*Omx + (-1 + 97*Omx)*ten) + pow(kin,2)*(18 + 29*ten + 12*pow(ten,2) + pow(ten,3)) -
                                         kin*(192*Omx + (-107 + 300*Omx)*ten + (31 + 114*Omx)*pow(ten,2) + 6*(-1 + Omx)*pow(ten,3))) +
                                  DelM2*(9*pow(bra,3)*(-288 + 98*ten + 119*pow(ten,2) + 6*pow(ten,3)) + 2*kin*(576*(-2 + ten)*ten -
                                                                                                               kin*(192 + 193*ten + 145*pow(ten,2)) + pow(kin,2)*(18 + 29*ten + 12*pow(ten,2) + pow(ten,3))) +
                                         6*bra*(288*(-2 + ten)*ten + kin*(288 + 98*ten - 409*pow(ten,2)) + pow(kin,2)*(16 + 85*ten +
                                                                                                                       48*pow(ten,2) + 3*pow(ten,3))) + 3*pow(bra,2)*(-144*(-8 - 4*ten + 7*pow(ten,2)) + kin*(-480 + 335*ten +
                                                                                                                                                                                                              383*pow(ten,2) + 18*pow(ten,3))))))/(2.*(1 + DelM2)*(3*pow(bra,2) + 2*kin)*den1*den2);


        // Solve cubic to find the three solutions
        rf_solve_poly_3(c3,c2,c1,c0,sols,&complex);

        if (ppt->perturbations_verbose > 1) {
                printf("\nGravitating attractor ICs give growing modes at z=%e: \n (Approximate) polynomial",z_ref);
                printf(" solutions h ~ (k_tau)^n (complex = %i) with exponents: \n",complex);
        }

        fastest_growth = sols[0]; //want fastest
        wouldbe_adiab = sols[0]; //want closest to zero
        for (i=0; i<3; i+=1) {
                if (sols[i]  > fastest_growth) {
                        fastest_growth = sols[i];
                }
                if (fabs(sols[i]) < fabs(wouldbe_adiab)) {
                        wouldbe_adiab = sols[i];
                }
                if (ppt->perturbations_verbose > 1) {
                        printf("   n_%i = %f\n",i, 2+sols[i]);
                }
        }
        if (ppt->perturbations_verbose > 1) {
                printf("  fastest growing mode n = %f\n",2+fastest_growth);
                printf("  mode closest to standard adiabatic solution, n=%f\n",2+wouldbe_adiab);
                printf("  omx = %e, dM* = %e\n",Omx,DelM2);
        }

        // Check that would-be adiabatic mode is actually the fastest mode, otherwise
        // the would-be adiabatic attractor destabilises to the fastest mode, i.e. we cannot assume that the curvature was
        // conserved between inflation and the beginning of hi_class and therefore there is no
        // relation between the inflational amplitude A_S and the parameter we use for normalisation of curvature.

        /* We don't need this: te closest to zero mode actually conserves eta/zeta in any case
           class_test_except(ppr->pert_ic_tolerance_smg>0 && (fabs(wouldbe_adiab) > ppr->pert_ic_tolerance_smg),
                ppt->error_message,
                free(pvecback),
                "\n   Cannot set initial conditions for early_smg: adiabatic mode h ~ tau^2 lost, h ~ tau^n with n = %f",2+wouldbe_adiab);
         */

        if (fabs(fastest_growth)>fabs(wouldbe_adiab)) {
                class_test_except(ppr->pert_ic_tolerance_smg>0 && (fabs(fastest_growth) > ppr->pert_ic_tolerance_smg),
                                  ppt->error_message,
                                  free(pvecback),
                                  "\n   Cannot set initial conditions for early_smg:\n    There does exist a mode where curvature is conserved n=%f, but solution destabilises to a faster-growing non-conserving mode with n=%f.",2+wouldbe_adiab,2+fastest_growth);
        }

        free(pvecback);

        // If we get here, then initialise modes and evolve them!

        return _SUCCESS_;

}

/*
 * Test for tachyonic instability of x_smg in RD before initialisation of
 * perturbations: if not stable, cannot set ICs properly.
 */
int perturb_test_ini_extfld_ic_smg(struct precision * ppr,
                                   struct background * pba,
                                   struct perturbs * ppt){


        double kin, bra, run, ten, DelM2, Omx, wx;
        double l1,l2, l3, l4,l5,l6,l7,l8, cs2num, Dd;
        double B1_smg, B2_smg;
        double tau_ini, z_ref;
        double x_growth_smg;
        double * pvecback;
        int first_index_back;

        class_alloc(pvecback,pba->bg_size*sizeof(double),ppt->error_message);

        z_ref = ppr->pert_ic_ini_z_ref_smg;

        class_call(background_tau_of_z(pba, z_ref,&tau_ini),
                   pba->error_message,
                   ppt->error_message);

        class_call(background_at_tau(pba,
                                     tau_ini,
                                     pba->long_info,
                                     pba->inter_normal,
                                     &first_index_back,
                                     pvecback),
                   pba->error_message,
                   ppt->error_message);

        // look up alphas etc. at z_ref
        wx = pvecback[pba->index_bg_p_smg]/pvecback[pba->index_bg_rho_smg];
        Omx = pvecback[pba->index_bg_rho_smg]/pow(pvecback[pba->index_bg_H],2);
        kin = pvecback[pba->index_bg_kineticity_smg];
        bra = pvecback[pba->index_bg_braiding_smg];
        run = pvecback[pba->index_bg_mpl_running_smg];
        ten = pvecback[pba->index_bg_tensor_excess_smg];
        DelM2 = pvecback[pba->index_bg_delta_M2_smg];//M2-1
        l1 = pvecback[pba->index_bg_lambda_1_smg];
        l2 = pvecback[pba->index_bg_lambda_2_smg];
        l3 = pvecback[pba->index_bg_lambda_3_smg];
        l4 = pvecback[pba->index_bg_lambda_4_smg];
        l5 = pvecback[pba->index_bg_lambda_5_smg];
        l6 = pvecback[pba->index_bg_lambda_6_smg];
        l7 = pvecback[pba->index_bg_lambda_7_smg];
        l8 = pvecback[pba->index_bg_lambda_8_smg];
        cs2num = pvecback[pba->index_bg_cs2num_smg];
        Dd = pvecback[pba->index_bg_kinetic_D_smg];

        B1_smg = (bra/Dd)*(bra/(2.*(-2 + bra)*(kin + l1)))*((-6 + kin)*l1 + 3*l4);
        B1_smg +=  (3*pow(bra,3))*(l1/Dd)/(2.*(-2 + bra)*(kin + l1));
        B1_smg += 2*(cs2num/Dd)*(3*bra*kin + pow(kin,2) - 3*l4)/(2.*(-2. + bra)*(kin + l1));
        B1_smg += 2*(3*l2*l4/Dd + (kin/Dd)*(l1*l2 - 8*l7) - 8*l1/Dd*l7)/(2.*(-2 + bra)*(kin + l1));
        B1_smg -= 2*(bra/Dd)*((kin*l1/(kin + l1) - 3*l1*l2/(kin + l1) + 3*l4/(kin + l1))/(2.*(-2 + bra)));

        B2_smg =  8*(1 + DelM2)*(3*l2*l6/Dd + 4*kin*l8/Dd);
        B2_smg += 4*(l1/Dd)*(8*(1 + DelM2)*l8 + l2*(12 - 12*Omx + (1 + DelM2)*(-12 + kin + Omx*(3 - 9*wx))));
        B2_smg += 2*(bra/Dd)*bra*(6*(1 + DelM2)*l6 + l1*(12 - 12*Omx + (1 + DelM2)*(-30 + kin + 6*Omx*(1 - 3*wx))));
        B2_smg += 3*pow(bra,3)*(1 + DelM2)*(l1/Dd)*(6 + Omx*(-1 + 3*wx));
        B2_smg += 2*(cs2num/Dd)*(2*(1 + DelM2)*pow(kin,2) - 12*(1 + DelM2)*l6 + 3*kin*(8 - 8*Omx + (1 + DelM2)*(-8 + Omx*(2 - 6*wx) + bra*(6 + Omx*(-1 + 3*wx)))));
        B2_smg -= 2*(bra/Dd)*(12*(1 + DelM2)*l6 + l1*(24 - 24*Omx + (1 + DelM2)*(2*kin - 3*(8 + 2*Omx*(-1 + 3*wx) + l2*(6 + Omx*(-1 + 3*wx))))));
        B2_smg /= (4.*(-2 + bra)*(1 + DelM2)*(kin + l1));

        x_growth_smg = 0.5*(1.-B1_smg);

        if (1.-2.*B1_smg + B1_smg*B1_smg -4.*B2_smg >=0) {
                x_growth_smg += 0.5*sqrt(1. -2.*B1_smg + B1_smg*B1_smg -4.*B2_smg);
        }

        if (ppt->perturbations_verbose > 1) {
                printf("\nExternal field attractor ICs at z=%e. Standard solution for grav. field, h = (k tau)^2.\n",z_ref);
                if(x_growth_smg<=3) {
                        printf("  smg evolves on standard attractor in external field with x_smg = k^2 tau^3;\n\n");
                }
                else{
                        printf("  tachyonic instability in smg dominates, x_smg = k^2 tau^n with n=%f.\n",x_growth_smg);
                        printf("  smg is sensitive to its initial conditions at end of inflation.\n");
                }
        }

        class_test_except(ppr->pert_ic_tolerance_smg>0 && (x_growth_smg > 3.+ppr->pert_ic_tolerance_smg),
                          ppt->error_message,
                          free(pvecback),
                          "\n   Cannot set initial conditions for smg: tachyonic instability dominates superhorizon attractor.\n");

        free(pvecback);

        // If we get here, then initialise modes and evolve them!

        return _SUCCESS_;
}

int calc_extfld_ampl(int nexpo,  double kin, double bra, double dbra, double run, double ten, double DelM2,
                     double Omx, double wx, double l1, double l2, double l3, double l4,
                     double l5, double l6,double l7,double l8, double cs2num, double Dd,
                     double ic_regulator_smg, double * amplitude){

        /* Solutions assuming the alphas are small, i.e. x_smg does not gravitate but moves
         * on an attractor provided by collapsing radiation. (w!=1/3 terms included properly here!)
           // We have already tested for an RD tachyon at z=pert_ic_ini_z_ref_smg and it wasn't there.
         * We can thus assume that h has the standard solution (tau^2 for adiabatic)
         * and solve the x_smg e.o.m. assuming C1=C2=0.
         *
         *   x_smg = C1 tau^n1 + C2 tau^n2 + A k^2 tau^n
         *
         * This requires that if the tachyon has appeared at some later time, the system will be moving into it slowly.
         *
         * We do not correct any other fields, since it would be inconsistent to include them
         * here, but not in the calculation of the exponent. If this is importnant, use gravitating_attr ICs.
         *
         *
         * The on-attractor solution for the scalar velocity x_smg is x_smg = amplitude * k^2 tau^n * ppr->curvature_ini
         * with amplitude = -B3/(6 + 3*B1 + B2).
         */





        // Calculate the amplitude of x_smg in ext_field_attr ICs, both for adiabatic and isocurvature
        // Since the scalar does not backreact, the different Ad and ISOcurv solutions differ
        // only by the exponent in h, h = C*tau^n. The only n-dependent terms are in B3 and amplitude


        double B1_smg, B2_smg, B3_smg, B3num_smg, B3denom_smg;
        double den1, den2, den3, den4, reg_rescaled;

        reg_rescaled = ic_regulator_smg*(fabs(bra)+fabs(kin)+fabs(l1)); //rescale the regulator to be proportional to the alphas


        den1 = (2.*(-2 + bra)*(kin + l1));
        if(reg_rescaled>0 && (fabs(den1)<reg_rescaled)) {
                den1 = copysign(reg_rescaled,den1);
        }

        B1_smg = (bra/Dd)*(bra/den1)*((-6 + kin)*l1 + 3*l4);
        B1_smg +=  (3*pow(bra,3))*(l1/Dd)/den1;
        B1_smg += 2*(cs2num/Dd)*(3*bra*kin + pow(kin,2) - 3*l4)/(2.*(-2. + bra)*(kin + l1));
        B1_smg += 2*(3*l2*l4/Dd + (kin/Dd)*(l1*l2 - 8*l7) - 8*l1/Dd*l7)/den1;
        B1_smg -= 2*(bra/Dd)*((kin*l1/(kin + l1) - 3*l1*l2/(kin + l1) + 3*l4/(kin + l1))/(2.*(-2 + bra)));

        den2 = (4.*(-2 + bra)*(1 + DelM2)*(kin + l1));
        if(reg_rescaled>0 && (fabs(den2)<reg_rescaled)) {
                den2 = copysign(reg_rescaled,den2);
        }

        B2_smg =  8*(1 + DelM2)*(3*l2*l6/Dd + 4*kin*l8/Dd);
        B2_smg += 4*(l1/Dd)*(8*(1 + DelM2)*l8 + l2*(12 - 12*Omx + (1 + DelM2)*(-12 + kin + Omx*(3 - 9*wx))));
        B2_smg += 2*(bra/Dd)*bra*(6*(1 + DelM2)*l6 + l1*(12 - 12*Omx + (1 + DelM2)*(-30 + kin + 6*Omx*(1 - 3*wx))));
        B2_smg += 3*pow(bra,3)*(1 + DelM2)*(l1/Dd)*(6 + Omx*(-1 + 3*wx));
        B2_smg += 2*(cs2num/Dd)*(2*(1 + DelM2)*pow(kin,2) - 12*(1 + DelM2)*l6 + 3*kin*(8 - 8*Omx + (1 + DelM2)*(-8 + Omx*(2 - 6*wx) + bra*(6 + Omx*(-1 + 3*wx)))));
        B2_smg -= 2*(bra/Dd)*(12*(1 + DelM2)*l6 + l1*(24 - 24*Omx + (1 + DelM2)*(2*kin - 3*(8 + 2*Omx*(-1 + 3*wx) + l2*(6 + Omx*(-1 + 3*wx))))));
        B2_smg /= den2;


        den3 = ((2. * Omx)*(kin + l1));
        reg_rescaled *=Omx;
        if(reg_rescaled>0 && (fabs(den3)<reg_rescaled)) {
                den3 = copysign(reg_rescaled,den3);
        }
        B3num_smg = ((-(((-2. + bra) * bra + 2 * l2) *
                        ((-2. + bra) * l1 - 4 * l3 + 2 * Dd * (-1. + nexpo))) +
                      cs2num * (-2 * (-2. + bra) * kin - 8 * l3 + 4 * Dd * (-1. + nexpo))) *
                     nexpo) / den3;

        B3denom_smg = 4*(Dd/Omx)*(-2 + bra);

        B3_smg = B3num_smg/B3denom_smg;

        reg_rescaled = ic_regulator_smg*(fabs(B1_smg)+fabs(B2_smg));
        den4 = B1_smg + B2_smg + nexpo + B1_smg*nexpo + pow(nexpo,2);

        if(reg_rescaled>0 && (fabs(den4)<reg_rescaled)) {
                den4 = copysign(reg_rescaled,den4);
        }


        *amplitude = -B3_smg/den4;

        return _SUCCESS_;
}

int get_gravity_coefficients_smg(
        struct perturbs * ppt,
        struct background * pba,
        double * pvecback,
        double * delM2, double * M2, double * kin, double * bra,
        double * ten, double * run, double * beh, double * res,
        double * cD, double * cK, double * cB, double * cH, double * c0,
        double * c1, double * c2, double * c3, double * c4,
        double * c5, double * c6, double * c7, double * c8,
        double * c9, double * c10, double * c11, double * c12,
        double * c13, double * c14, double * c15, double * c16,
        double * res_p, double * cD_p, double * cB_p, double * cH_p,
        double * c9_p, double * c10_p, double * c12_p, double * c13_p
        ){
        /* It returns the alphas and the coefficients of the Einstein equations
           that will be used to evaluate the perturbations and their initial
           conditions. This function uses use_pert_var_deltaphi_smg to decide which
           coefficients to output.
         */

        double a = pvecback[pba->index_bg_a];
        double H = pvecback[pba->index_bg_H];
        double Hp = pvecback[pba->index_bg_H_prime];

        *delM2 = pvecback[pba->index_bg_delta_M2_smg];
        *M2 = pvecback[pba->index_bg_M2_smg];
        *kin = pvecback[pba->index_bg_kineticity_smg];
        *bra = pvecback[pba->index_bg_braiding_smg];
        *ten = pvecback[pba->index_bg_tensor_excess_smg];
        *run = pvecback[pba->index_bg_mpl_running_smg];
        *beh = pvecback[pba->index_bg_beyond_horndeski_smg];

        if (ppt->use_pert_var_deltaphi_smg == _TRUE_) {
                *res = 1.;
                *cD  = pvecback[pba->index_bg_kinetic_D_over_phiphi_smg];
                *cK  = pvecback[pba->index_bg_kineticity_over_phiphi_smg];
                *cB  = pvecback[pba->index_bg_braiding_over_phi_smg];
                *cH  = pvecback[pba->index_bg_beyond_horndeski_over_phi_smg];
                *c0  = pvecback[pba->index_bg_C0_smg];
                *c1  = pvecback[pba->index_bg_C1_smg];
                *c2  = pvecback[pba->index_bg_C2_smg];
                *c3  = pvecback[pba->index_bg_C3_smg];
                *c4  = pvecback[pba->index_bg_C4_smg];
                *c5  = pvecback[pba->index_bg_C5_smg];
                *c6  = pvecback[pba->index_bg_C6_smg];
                *c7  = pvecback[pba->index_bg_C7_smg];
                *c8  = pvecback[pba->index_bg_C8_smg];
                *c9  = pvecback[pba->index_bg_C9_smg];
                *c10 = pvecback[pba->index_bg_C10_smg];
                *c11 = pvecback[pba->index_bg_C11_smg];
                *c12 = pvecback[pba->index_bg_C12_smg];
                *c13 = pvecback[pba->index_bg_C13_smg];
                *c14 = pvecback[pba->index_bg_C14_smg];
                *c15 = pvecback[pba->index_bg_C15_smg];
                *c16 = pvecback[pba->index_bg_C16_smg];
                *res_p = 0.;
                *cD_p  = pvecback[pba->index_bg_kinetic_D_over_phiphi_prime_smg];
                *cB_p  = pvecback[pba->index_bg_braiding_over_phi_prime_smg];
                *cH_p  = pvecback[pba->index_bg_beyond_horndeski_over_phi_prime_smg];
                *c9_p  = pvecback[pba->index_bg_C9_prime_smg];
                *c10_p = pvecback[pba->index_bg_C10_prime_smg];
                *c12_p = pvecback[pba->index_bg_C12_prime_smg];
                *c13_p = pvecback[pba->index_bg_C13_prime_smg];
        }
        else if (ppt->use_pert_var_deltaphi_smg == _FALSE_) {
                *res = -a*H;
                *cD  = pvecback[pba->index_bg_kinetic_D_smg];
                *cK  = pvecback[pba->index_bg_kineticity_smg];
                *cB  = pvecback[pba->index_bg_braiding_smg];
                *cH  = pvecback[pba->index_bg_beyond_horndeski_smg];
                *c0  = pvecback[pba->index_bg_A0_smg];
                *c1  = pvecback[pba->index_bg_A1_smg];
                *c2  = pvecback[pba->index_bg_A2_smg];
                *c3  = pvecback[pba->index_bg_A3_smg];
                *c4  = pvecback[pba->index_bg_A4_smg];
                *c5  = pvecback[pba->index_bg_A5_smg];
                *c6  = pvecback[pba->index_bg_A6_smg];
                *c7  = pvecback[pba->index_bg_A7_smg];
                *c8  = pvecback[pba->index_bg_A8_smg];
                *c9  = pvecback[pba->index_bg_A9_smg];
                *c10 = pvecback[pba->index_bg_A10_smg];
                *c11 = pvecback[pba->index_bg_A11_smg];
                *c12 = pvecback[pba->index_bg_A12_smg];
                *c13 = pvecback[pba->index_bg_A13_smg];
                *c14 = pvecback[pba->index_bg_A14_smg];
                *c15 = pvecback[pba->index_bg_A15_smg];
                *c16 = pvecback[pba->index_bg_A16_smg];
                *res_p = -a*(Hp + a*H);
                *cD_p  = pvecback[pba->index_bg_kinetic_D_prime_smg];
                *cB_p  = pvecback[pba->index_bg_braiding_prime_smg];
                *cH_p  = pvecback[pba->index_bg_beyond_horndeski_prime_smg];
                *c9_p  = pvecback[pba->index_bg_A9_prime_smg];
                *c10_p = pvecback[pba->index_bg_A10_prime_smg];
                *c12_p = pvecback[pba->index_bg_A12_prime_smg];
                *c13_p = pvecback[pba->index_bg_A13_prime_smg];
        }
        else {
                printf("It was not possible to determine if oscillations of the background scalar field should be allowed or not.\n");
                return _FAILURE_;
        }

        return _SUCCESS_;
}

int get_x_x_prime_qs_smg(
        struct precision * ppr,
        struct background * pba,
        struct perturbs * ppt,
        struct perturb_workspace * ppw,
        double k, double * x_qs_smg, double * x_prime_qs_smg
        ){

        double k2 = k*k;
        double rho_r, p_tot, p_smg;
        double a, H, delM2, M2, kin, bra, ten, run, beh;
        double res, cD, cK, cB, cH;
        double c0, c1, c2, c3, c4, c5, c6, c7, c8;
        double c9, c10, c11, c12, c13, c14, c15, c16;
        double c9_p, c10_p, c12_p, c13_p;
        double res_p, cD_p, cB_p, cH_p;
        double x_prime_qs_smg_num, x_prime_qs_smg_den;

        a = ppw->pvecback[pba->index_bg_a];
        H = ppw->pvecback[pba->index_bg_H];
        rho_r = ppw->pvecback[pba->index_bg_rho_g] + ppw->pvecback[pba->index_bg_rho_ur];
        p_tot = ppw->pvecback[pba->index_bg_p_tot_wo_smg];
        p_smg = ppw->pvecback[pba->index_bg_p_smg];

        class_call(
                get_gravity_coefficients_smg(
                        ppt, pba, ppw->pvecback,
                        &delM2, &M2, &kin, &bra, &ten, &run, &beh, &res,
                        &cD, &cK, &cB, &cH, &c0, &c1, &c2, &c3,
                        &c4, &c5, &c6, &c7, &c8, &c9, &c10, &c11,
                        &c12, &c13, &c14, &c15, &c16, &res_p, &cD_p, &cB_p,
                        &cH_p, &c9_p, &c10_p, &c12_p, &c13_p
                        ),
                ppt->error_message,
                ppt->error_message);

        /* This is the expression for the scalar field in the quasi static approximation */
        if (ppt->get_h_from_trace == _TRUE_) {
                /* Scalar field in QS with h' */
                *x_qs_smg =
                        +1./res*(
                                -9./2.*cB*pow(a,2)*ppw->delta_p/M2
                                + c10*k2*ppw->pvecmetric[ppw->index_mt_eta]
                                + (c9*pow(a*H,2) - 1./3.*cH*k2)*ppw->pvecmetric[ppw->index_mt_h_prime]/a/H
                                + 2./3.*cH*pow(k2,2)*ppw->pvecmetric[ppw->index_mt_alpha]/a/H
                                )/(c13*k2 + c12*pow(a*H,2));
        }
        else {
                /* Scalar field in QS without h' */
                *x_qs_smg =
                        1./res*(
                                +k2*(
                                        +4.*(1. + beh)*cH*k2
                                        - 3.*pow(a*H,2)*((2. - bra)*c10 + 4.*(1. + beh)*c9)
                                        )*ppw->pvecmetric[ppw->index_mt_eta]
                                - 2.*(2. - bra)*cH*H*pow(k2,2)*a*ppw->pvecmetric[ppw->index_mt_alpha]
                                + 6.*(
                                        +(cH*k2 - 3.*c9*pow(a*H,2))*ppw->delta_rho
                                        + 9./4.*cB*(2. - bra)*pow(a*H,2)*ppw->delta_p
                                        )*pow(a,2)/M2
                                )/(
                                +4.*c15*cH*pow(k2,2)
                                - k2*pow(a*H,2)*(3.*c13*(2. - bra) + 12.*c15*c9 - 4.*c16*cH)
                                - 3.*pow(a*H,4)*(c12*(2. - bra) + 4.*c16*c9)
                                );
        }


        /* scalar field derivative equation
         * In order to estimate it we followed this procedure:
         * - we calculated analytically the time derivative of the x_smg equation
         * - we used delta_p' = delta_rho_r'/3 (radiation is the only component that contributes to delta_p')
         * - we used the conservation equation for radiation to get rid of delta_rho_r'
         * - we used the Einstein equations to get rid of eta', h'', alpha'
         * The result is approximated when rsa is on since the velocity of radiation gets updated only after
         * this call in perturb_einstein */

        if (ppt->get_h_from_trace == _TRUE_) {
                /* Numerator of the scalar field derivative in QS with h' */
                x_prime_qs_smg_num =
                        +3.*(
                                +3.*(2.*c9*cK - cB*cD*(2. + run) - cD*(cB*res_p/res - cB_p)/a/H)
                                - 2.*cH*cK*pow(a*H,-2)*k2
                                )*ppw->delta_rho_r*a/H/M2
                        + 9.*(
                                +2.*cD*(cH*res_p/res - cH_p)/a/H + 6.*c3*c9
                                - cD*(cB + c10) + 3.*cD*cH*(1. + 2./3.*run - (p_tot + p_smg)*pow(H,-2))
                                - 2.*c3*cH*k2*pow(a*H,-2)
                                )*pow(H,-2)*ppw->rho_plus_p_theta_r/M2
                        + 18.*cD*cH*pow(a*H,-2)*k2*ppw->rho_plus_p_shear*a/H/M2
                        + 4.*k2*pow(a*H,-2)*(
                                +cH*(c1 - cD - ten*cD)*k2*pow(a*H,-2)
                                - 3./2.*(2.*c1*c9 - (c10*cD*res_p/res - cD*c10_p)/a/H)
                                )*a*H*ppw->pvecmetric[ppw->index_mt_eta]
                        + 3.*(
                                +2.*cD*(c9*res_p/res - c9_p)/a/H - 2.*c2*c9 + c9*cD
                                - cD*(2.*cB*rho_r/M2 - 3.*c9*(p_tot + p_smg))*pow(H,-2)
                                + 2./3.*cH*(c2 + 2.*cD + run*cD)*k2*pow(a*H,-2)
                                )*ppw->pvecmetric[ppw->index_mt_h_prime]
                        + 6.*a*H*res*(
                                +c6*c9 + cD*(c12_p/a/H - c12 - 3.*c12*(p_tot + p_smg)*pow(H,-2))
                                - (
                                        +cD*(2.*c0*cH*res_p/res - c13_p - 2.*c0*cH_p)/a/H
                                        + c9*(6.*c0*c3 - c7) - c0*c10*cD + c6*cH/3.
                                        + 3.*c0*cD*cH*(1. + 2./3.*run - (p_tot + p_smg)*pow(H,-2))
                                        )*k2*pow(a*H,-2)
                                + 1./3.*cH*(6.*c0*c3 - c7 + 2.*c8*cD)*pow(k2,2)*pow(a*H,-4)
                                )*ppw->pvecmetric[ppw->index_mt_x_smg];

                /* Denominator of the scalar field derivative in QS with h' */
                x_prime_qs_smg_den =
                        -6.*res*(
                                +c4*c9 + c12*cD
                                - k2*(
                                        +6.*cB*cD*(cH*res_p/res - cH_p)/a/H
                                        - 12.*c9*(c5 - 3./2.*c3*cB)
                                        - 3.*cD*(c10*cB + 2.*c13)
                                        + 2.*c4*cH
                                        + 3.*cB*cD*cH*(3. + 2.*run)
                                        - 9.*cB*cD*cH*(p_tot + p_smg)*pow(H,-2)
                                        )/6.*pow(a*H,-2)
                                - cH*pow(k2,2)*(2.*c5 - 3.*c3*cB + 2.*cD*cH)/3.*pow(a*H,-4)
                                );
        }
        else {
                /* Numerator of the scalar field derivative in QS without h' */
                x_prime_qs_smg_num =
                        -18.*(2. - bra)*cD*cH*pow(H,-3)*k2*ppw->rho_plus_p_shear/a/M2
                        + (2. - bra)*(
                                +6.*cH*cK*pow(H,-3)*k2/a
                                + 9.*(
                                        +cD*(cB*res_p - cB_p*res)
                                        + ((2. + run)*cB*cD - 2.*c9*cK)*H*res*a
                                        )*pow(H,-2)/res
                                )*ppw->delta_rho_r/M2
                        + 9.*(2. - bra)*(
                                +2.*c3*cH*pow(H,-4)*pow(a,-2)*k2
                                - (
                                        +2.*cD*H*(cH*res_p - cH_p*res)/a/res
                                        + (6.*c3*c9 - c10*cD - cB*cD + 3.*cD*cH + 2.*run*cD*cH)*pow(H,2)
                                        - 3.*cD*cH*(p_tot + p_smg)
                                        )*pow(H,-4)
                                )/M2*ppw->rho_plus_p_theta_r
                        - (
                                +12.*(c2 + 2.*cD + run*cD)*cH*pow(H,-3)*k2/a
                                + 18.*(
                                        +2.*cD*H*(c9*res_p/res - c9_p)
                                        - 2.*cB*cD*rho_r*a/M2
                                        + c9*(cD - 2.*c2)*pow(H,2)*a
                                        + 3.*c9*cD*(p_tot + p_smg)*a
                                        )*pow(H,-3)
                                )/M2*ppw->delta_rho
                        + (
                                +4.*cH*(
                                        +(2. - bra)*(cD + ten*cD - c1)
                                        - 2.*(c2 + 2.*cD + run*cD)*(1. + beh)
                                        )*pow(k2,2)*pow(a*H,-3)
                                - 6.*(
                                        +(2. - bra)*(cD*(c10*res_p/res - c10_p)/a/H - 2.*c1*c9)
                                        + 2.*(1. + beh)*(
                                                +2.*cD*(c9*res_p/res - c9_p)/a/H
                                                + c9*(cD - 2.*c2)
                                                - 2.*cB*cD*rho_r*pow(H,-2)/M2
                                                + 3.*c9*cD*(p_tot + p_smg)*pow(H,-2)
                                                )
                                        )*k2/a/H
                                )*ppw->pvecmetric[ppw->index_mt_eta]
                        + (
                                +(
                                        -(2. - bra)*(6.*c0*c3 - c7 + 2.*c8*cD)
                                        + 4.*c15*(c2 + 2.*cD + run*cD)
                                        )*2.*cH*pow(k2,2)*res*pow(a*H,-3)
                                + 6.*(
                                        +cD*H*(
                                                +4.*c16*c9*res_p/res
                                                - c12_p*(2. - bra)
                                                - 4.*c16*c9_p
                                                )/a
                                        - 4.*c16*cB*cD*rho_r/M2
                                        - 4.*c16*c2*c9*pow(H,2)
                                        - c6*c9*(2. - bra)*pow(H,2)
                                        + cD*((2. - bra)*c12 + 2.*c16*c9)*(pow(H,2) + 3.*(p_tot + p_smg))
                                        )*res*a/H
                                + 2.*(
                                        +12.*cD*c15*(c9*res_p/res - c9_p)/H/a
                                        - 12.*c15*cB*cD*rho_r/M2*pow(H,-2)
                                        + 2.*(
                                                +3.*c15*c9*(cD - 2.*c2)
                                                + 2.*cH*c16*(c2 + 2.*cD + run*cD)
                                                )
                                        + (2. - bra)*(
                                                -3.*cD*(2.*c0*cH_p - 2.*c0*cH*res_p/res + c13_p)/a/H
                                                + 18.*c0*c3*c9 - 3.*c7*c9
                                                - 3.*c0*c10*cD + c6*cH
                                                + 6.*(3./2. + run)*c0*cD*cH
                                                )
                                        - 9.*cD*((2. - bra)*c0*cH - 2.*c15*c9)*(p_tot + p_smg)*pow(H,-2)
                                        )*res*k2/a/H
                                )*ppw->pvecmetric[ppw->index_mt_x_smg];


                /* Denominator of the scalar field derivative in QS without h' */
                x_prime_qs_smg_den =
                        -2.*cH*res*(2. - bra)*(2.*c5 - 3.*c3*cB + 2.*cD*cH)*pow(a*H,-4)*pow(k2,2)
                        + a*(
                                -6.*cB*cD*(2. - bra)*(cH*res_p - cH_p*res)*H
                                + (2. - bra)*(
                                        +12.*c5*c9 - 18.*c3*c9*cB
                                        + 6.*c13*cD + 3.*c10*cB*cD - 2.*c4*cH
                                        - 3.*(3. + 2.*run)*cB*cD*cH
                                        + 9.*cB*cD*cH*(p_tot + p_smg)*pow(H,-2)
                                        )*res*a*pow(H,2)
                                - 8.*(c2 + 2.*cD + run*cD)*c14*cH*pow(H,2)*res*a
                                )*k2*pow(a*H,-4)
                        + 6.*(
                                -4.*c14*cD*H*(c9*res_p - c9_p*res)
                                + 4.*c14*(cB*cD*rho_r/M2 + c2*c9*pow(H,2))*res*a
                                + (2. - bra)*(c4*c9 + c12*cD)*pow(H,2)*res*a
                                - 2.*c14*c9*cD*(pow(H,2) + 3.*(p_tot + p_smg))*res*a
                                )*pow(H,-2)/a;
        }

        *x_prime_qs_smg = x_prime_qs_smg_num/x_prime_qs_smg_den;

        return _SUCCESS_;
}

int hi_class_define_indices_tp(
         struct perturbs * ppt,
				 int * index_type
			 ) {

  class_define_index(ppt->index_tp_phi_smg,    ppt->has_source_phi_smg,   *index_type,1);
  class_define_index(ppt->index_tp_phi_prime_smg,  ppt->has_source_phi_prime_smg, *index_type,1);

  return _SUCCESS_;
}

int hi_class_define_indices_mt(
        struct perturb_workspace * ppw,
  			int * index_mt
			 ) {

  class_define_index(ppw->index_mt_x_smg,_TRUE_,*index_mt,1);   /* x_smg (can be dynamical or not) */
  class_define_index(ppw->index_mt_x_prime_smg,_TRUE_,*index_mt,1);   /* x_smg' (can be dynamical or not) */
  class_define_index(ppw->index_mt_x_prime_prime_smg,_TRUE_,*index_mt,1);   /* x_smg'' (passed to integrator) */
  class_define_index(ppw->index_mt_rsa_p_smg,_TRUE_,*index_mt,1);   /**< correction to the evolution of ur and g species in radiation streaming approximation due to non-negligible pressure at late-times*/

  return _SUCCESS_;
}

int perturb_hi_class_qs(struct precision * ppr,
                       struct background * pba,
                       struct perturbs * ppt,
                       struct perturb_workspace * ppw,
                       double k,
                       double * tau_ini,
                       double tau_end) {

  double tau_lower = pba->tau_table[0];
  double tau_upper = *tau_ini;
  int is_early_enough;

  /* A second loop starts here to anticipate the initial time if the qs_smg
    state is different from ppt->initial_approx_qs_smg. */
  if (ppt->method_qs_smg == automatic) {
    class_call(background_tau_of_z(pba,
                                   1./ppr->a_ini_test_qs_smg-1.,
                                   &tau_lower),
               pba->error_message,
               ppt->error_message);
    is_early_enough = _FALSE_;
    while (((tau_upper - tau_lower)/tau_lower > ppr->tol_tau_approx) && is_early_enough == _FALSE_) {
      int approx;
      perturb_test_at_k_qs_smg(ppr,
                              pba,
                              ppt,
                              k,
                              tau_upper,
                              &approx);
      if (approx == ppt->initial_approx_qs_smg) {
        is_early_enough = _TRUE_;
      }
      else {
        tau_upper = 0.5*(tau_lower + tau_upper);
      }
    }
    *tau_ini = tau_upper;
  }

  /** - find the intervals over which the approximation scheme for qs_smg is constant */

  int qs_array_smg[] = _VALUES_QS_SMG_FLAGS_;
  class_alloc(ppw->tau_scheme_qs_smg,sizeof(qs_array_smg)/sizeof(int)*sizeof(double),ppt->error_message);

  if ((ppt->method_qs_smg == automatic) || (ppt->method_qs_smg == fully_dynamic_debug) || (ppt->method_qs_smg == quasi_static_debug)) {
  class_call(perturb_find_scheme_qs_smg(ppr,
                                       pba,
			                                 ppt,
                                       ppw,
                                       k,
                                       *tau_ini,
                                       tau_end),
             ppt->error_message,
             ppt->error_message);
  }


  return _SUCCESS_;
}

int perturb_store_columntitles_smg(
				struct perturbs * ppt
      ) {

  if (ppt->use_pert_var_deltaphi_smg==_TRUE_) {
    class_store_columntitle(ppt->scalar_titles, "delta_phi_smg", _TRUE_);
    class_store_columntitle(ppt->scalar_titles, "delta_phi_prime_smg", _TRUE_);
    class_store_columntitle(ppt->scalar_titles, "delta_phi_prime_prime_smg", _TRUE_);
  }
  else {
    class_store_columntitle(ppt->scalar_titles, "V_x_smg", _TRUE_);
    class_store_columntitle(ppt->scalar_titles, "V_x_prime_smg", _TRUE_);
    class_store_columntitle(ppt->scalar_titles, "V_x_prime_prime_smg", _TRUE_);
  }

  /* Quasi-static functions smg*/
  class_store_columntitle(ppt->scalar_titles, "mass2_qs", _TRUE_);
  class_store_columntitle(ppt->scalar_titles, "mass2_qs_p", _TRUE_);
  class_store_columntitle(ppt->scalar_titles, "rad2_qs", _TRUE_);
  class_store_columntitle(ppt->scalar_titles, "friction_qs", _TRUE_);
  class_store_columntitle(ppt->scalar_titles, "slope_qs", _TRUE_);

  return _SUCCESS_;
}

int perturb_verbose_qs_smg(
				struct perturbs * ppt,
        struct perturb_workspace * ppw,
        double k,
        double tau_switch,
        int * ap_ini,
        int * ap_end
      ) {

  int qs_array_smg[] = _VALUES_QS_SMG_FLAGS_;

  if ((qs_array_smg[ap_ini[ppw->index_ap_qs_smg]]==1) &&
      (qs_array_smg[ap_end[ppw->index_ap_qs_smg]]==0)) {
    fprintf(stdout,"Mode k=%e: will switch off the quasi_static approximation smg (1 -> 0) at tau=%e\n",k,tau_switch);
  }
  if ((qs_array_smg[ap_ini[ppw->index_ap_qs_smg]]==0) &&
      (qs_array_smg[ap_end[ppw->index_ap_qs_smg]]==1)) {
    fprintf(stdout,"Mode k=%e: will switch on the quasi_static approximation smg (0 -> 1) at tau=%e\n",k,tau_switch);
  }

  return _SUCCESS_;
}

int hi_class_define_indices_pt(
        struct perturb_workspace * ppw,
        struct perturb_vector * ppv,
				int * index_pt
      ) {

  int qs_array_smg[] = _VALUES_QS_SMG_FLAGS_;

  /* scalar field: integration indices are assigned only if fd (0) */
  if (qs_array_smg[ppw->approx[ppw->index_ap_qs_smg]] == 0) {
    class_define_index(ppv->index_pt_x_smg,_TRUE_,*index_pt,1); /* dynamical scalar field perturbation */
    class_define_index(ppv->index_pt_x_prime_smg,_TRUE_,*index_pt,1); /* dynamical scalar field velocity */
  }

  return _SUCCESS_;
}

int perturb_vector_init_smg(
        struct perturb_workspace * ppw,
        struct perturb_vector * ppv,
        int * pa_old
      ) {

  int qs_array_smg[] = _VALUES_QS_SMG_FLAGS_;


  //pass the values only if the order is correct

  if ((qs_array_smg[pa_old[ppw->index_ap_qs_smg]] == _TRUE_) && (qs_array_smg[ppw->approx[ppw->index_ap_qs_smg]] == _FALSE_)) {
    ppv->y[ppv->index_pt_x_smg] =
      ppw->pvecmetric[ppw->index_mt_x_smg];
    ppv->y[ppv->index_pt_x_prime_smg] =
      ppw->pvecmetric[ppw->index_mt_x_prime_smg];

  }
  else if (qs_array_smg[ppw->approx[ppw->index_ap_qs_smg]] == _FALSE_) {
    ppv->y[ppv->index_pt_x_smg] =
      ppw->pv->y[ppw->pv->index_pt_x_smg];
    ppv->y[ppv->index_pt_x_prime_smg] =
      ppw->pv->y[ppw->pv->index_pt_x_prime_smg];
  }

  return _SUCCESS_;
}

int perturb_adiabatic_ic_smg(
      struct precision * ppr,
      struct background * pba,
      struct perturbs * ppt,
      struct perturb_workspace * ppw,
      double * ptr_eta,
      double * ptr_delta_ur,
      double * ptr_theta_ur,
      double * ptr_shear_ur,
      double * ptr_l3_ur,
      double * ptr_delta_dr,
      double tau,
      double k,
      double fracnu,
      double om,
      double rho_r
      ) {

  double eta = *ptr_eta;
  double delta_ur = *ptr_delta_ur;
  double theta_ur = *ptr_theta_ur;
  double shear_ur = *ptr_shear_ur;
  double l3_ur = *ptr_l3_ur;
  double delta_dr = *ptr_delta_dr;

  /* (k tau)^2, (k tau)^3 */
  double ktau_two=k*k*tau*tau;
  double ktau_three=k*tau*ktau_two;

  double s2_squared = 1.-3.*pba->K/k/k;

  double a,a_prime_over_a;

  double dt=0., Omx=0., wx=0., kin=0., bra=0., bra_p=0., dbra=0., ten=0., run=0., M2=0.,DelM2=0.;
  double Dd=0., cs2num=0., cs2num_p=0.;
  double l1=0.,l2=0., l3=0., l4=0.,l5=0.,l6=0.,l7=0.,l8=0.,l2_p=0., l8_p=0.;
  double B1_smg, B2_smg, B3_smg, B3num_smg, B3denom_smg, amplitude;
  double rho_smg=0., rho_tot=0., p_tot=0., p_smg=0., H=0.,Hprime=0;
  double g1=0., g2=0., g3=0.;
  double x_smg=0.,xp_smg=0.,delta_rho_r=0.;

  int qs_array_smg[] = _VALUES_QS_SMG_FLAGS_;
  int nexpo;

  a_prime_over_a = ppw->pvecback[pba->index_bg_H]*a;

  H = ppw->pvecback[pba->index_bg_H];//TODO_EB
  Hprime = ppw->pvecback[pba->index_bg_H_prime];
  a = ppw->pvecback[pba->index_bg_a];
  rho_tot = ppw->pvecback[pba->index_bg_rho_tot_wo_smg];
  p_tot = ppw->pvecback[pba->index_bg_p_tot_wo_smg];

  // Read in the initial values of all background params: alphas, Omx, w

  //perturbation to time variable

  dt = -1/(4.*ppw->pvecback[pba->index_bg_H])*ppw->pv->y[ppw->pv->index_pt_delta_g];


  rho_smg = ppw->pvecback[pba->index_bg_rho_smg];
  p_smg = ppw->pvecback[pba->index_bg_p_smg];

  wx = p_smg/rho_smg;
  Omx = rho_smg/pow(H,2);
  kin = ppw->pvecback[pba->index_bg_kineticity_smg];
  bra = ppw->pvecback[pba->index_bg_braiding_smg];
  bra_p = ppw->pvecback[pba->index_bg_braiding_prime_smg];
  dbra= bra_p/(a*H) ; //Read in log(a) diff of braiding
  run = ppw->pvecback[pba->index_bg_mpl_running_smg];
  ten = ppw->pvecback[pba->index_bg_tensor_excess_smg];
  l1 = ppw->pvecback[pba->index_bg_lambda_1_smg];
  l2 = ppw->pvecback[pba->index_bg_lambda_2_smg];
  l3 = ppw->pvecback[pba->index_bg_lambda_3_smg];
  l4 = ppw->pvecback[pba->index_bg_lambda_4_smg];
  l5 = ppw->pvecback[pba->index_bg_lambda_5_smg];
  l6 = ppw->pvecback[pba->index_bg_lambda_6_smg];
  l7 = ppw->pvecback[pba->index_bg_lambda_7_smg];
  l8 = ppw->pvecback[pba->index_bg_lambda_8_smg];
  l2_p = ppw->pvecback[pba->index_bg_lambda_2_prime_smg];
  l8_p = ppw->pvecback[pba->index_bg_lambda_8_prime_smg];
  cs2num = ppw->pvecback[pba->index_bg_cs2num_smg];
  cs2num_p = ppw->pvecback[pba->index_bg_cs2num_prime_smg];
  Dd = ppw->pvecback[pba->index_bg_kinetic_D_smg];
  M2 = ppw->pvecback[pba->index_bg_M2_smg];
  DelM2 = ppw->pvecback[pba->index_bg_delta_M2_smg];//M2-1


  /* TODO_EB: revisit initial conditions for beyond horndeski and oscillations */

  if (qs_array_smg[ppw->approx[ppw->index_ap_qs_smg]] == 0)
  {

  /* Initial conditions for the *dynamical* scalar field in the adiabatic mode
  * 1) gravitating_attr: Self-consistent Gravitating Attractor
  *    We allow the scalar to contribute the gravitational field during RD (can happen if Omx or alphas large at early times)
  *    and solve radiation-scalar system together.
  *    We make the assumption that  wx=1/3 and OmX is constant and constant alphas.
  *    Parameters smaller c.f. others can change in time.
  *    Scalar field can give rise to mode faster than standard adiabatic, which we test for and reject.
  *    Note that the scalar affects the gravitational potentials here,
  *    so we recompute eta and the velocities of the UR matter
  *
  * 2) Single clock
    * x_smg = delta_phi/phi_dot
  * phi(t,x) = phi(tau+delta tau(x))
  * This leads to very simple expressions:
  * x_smg = delta tau = delta_cdm/a_prime_over_a and x_prime_smg = 0
  *
  * 3) kineticity only IC: x_smg = (k tau)^2
  * from x_smg'' = 2 (a H)^2 x_smg
  *
  * 4) zero IC: x_smg = 0, x_smg'= 0. Good for checking the relevance of ICs.
  *
  * 5) ext_field_attr: External-field Attractor
  *    This assumes that OmX and all the alphas are small initially,
  *    so we are allowed arbitrary w. The scalar does not influence
  *    the gravitational potentials early on (i.e. evolves in an external field), so we only need to set the
  *    initial condition for x_smg but not the other fields.
  *    Appropriate for usual MG with no contribution at early times.
  */


    if (ppt->pert_initial_conditions_smg == gravitating_attr)
    {
      /*  ICs in case of large alphas in RD, when the scalar field affects the gravitational field.
       *  Exact for constant alpha models. We are allowed large Omx provided w=1/3 (tracker).
       *  In principle, can use for general alpha/Omx, but the expressions miss contributions from w!=1/3,
       *  so the amplitude will be somewhat off.
       *  Large alphas => large fifth forces, which can backreact on gravitiational potential.
       *  General soluton has

               h = (k tau)^(2+dnh);  x_smg = amplitude * (k tau)^2 tau^dnv

       *  If run=0, there is a solution with dnh = dnv = 0, but there may be faster-growing modes,
       *  which will end up dominating and do not conserve curvature superhorizon.
       *  We have already checked for their presence at some fiducial z_ref (line ~210) and failed
       *  if this is the case.
       *
       *  If we have got this far, then we let perturbations run, since any instability would
       *  have apeared as a rusult of evolving alphas after the z_ref test above.
       *  We recompute the power law in case the values of alphas have changed.
       *
       *  If run!=0, no conservation of zeta (at best approximate) or polynomial attractor.
       *  For small enough run, dnh!=dnv!=0 and we can find an approximate solution.
       *  Note that zeta is not conserved when Planck mass evolves!
       */

        //  Calculate the coefficients of polynomial for exponent of the h and x_smg evolution:
        //  These parts are common to the coefficients coming from both the x_smg and h equations.

        // Note: The denominators in the expressions below can be zero. We try to trap this and regulate.
        // We assume that M*^2>0 and D>0 which are tested for in the background routine.
        // Doing this gives wrong ICs, but it's better than segmentation faults.

        // declare additional vars for grac attr initial conditions
        double A_x_smg, A_v_nu_smg, A_sigma_nu_smg, A1_eta_smg, A2_eta_smg;
        double n_nosource_smg, n_fastest_smg, dnv, dnh, dn, eps_smg;
        double c0, c1, c2, c3, c0hp, c1hp, c2hp, c0vp, c1vp, c2vp;
        double sols[3];
        double den1,den2, ic_regulator_smg;
        int    complex,i;



        ic_regulator_smg =  ppr->pert_ic_regulator_smg; //read in the minimum size that will get regulated
        ic_regulator_smg *= fabs(kin)+fabs(bra)+fabs(ten); //scale it to be proportional to the alphas

        c3  =   1.;

        c2  =   5. + 2.*run;

        den1 = (3*bra*ten + kin*(2 + ten));
        if(ic_regulator_smg>0 && (fabs(den1)<ic_regulator_smg)){
          den1 = copysign(ic_regulator_smg,den1);
        }

        c1  =   (9*pow(bra,3)*pow(1 + DelM2,2)*(6 + 5*run)*ten + 3*pow(bra,2)*(1 + DelM2)*(-12*(-1 + Omx)*(-3 + run)*ten +
                (1 + DelM2)*kin*(6 + 5*run)*(2 + ten)) + 6*bra*(-24*(-1 + Omx)*(DelM2 + Omx)*ten + (1 + DelM2)*kin*
                (12*(-1 + Omx) + (-2 + 6*DelM2 + 8*Omx + 5*(1 + DelM2)*run)*ten)) + 2*kin*((1 + DelM2)*kin*
                (2*(2 + 3*DelM2 + Omx) + 5*(1 + DelM2)*run)*(2 + ten) - 12*(-1 + Omx)*((1 + DelM2)*run*ten + 2*(DelM2 + Omx)
                *(2 + ten))))/ (pow(1 + DelM2,2)*(3*pow(bra,2) + 2*kin)*den1);

        c0  =   (24*(-1 + Omx)*run*(4*kin*Omx - 3*pow(bra,2)*(-2 + ten) - DelM2*(3*pow(bra,2) + 2*kin)*(-2 + ten) +
                2*kin*(-2 + Omx)*ten + 6*bra*(-1 + Omx)*ten))/(pow(1 + DelM2,2)*(3*pow(bra,2) + 2*kin)*den1);


        // When run!=0, h and x_smg do not evolve with the same power law. There are O(run) differences to the
        // coefficients when the smg + radiation system at k->0 is expressed purely as an ODE for x_smg vs the ODE for h.
        // The corrections to the above are below.

        den2 =   ((-6*bra*(1 + DelM2) + pow(bra,2)*(1 + DelM2) + 8*(DelM2 + Omx))*(4*(DelM2 + Omx) - 2*(2 + DelM2 - Omx)*ten +
                bra*(1 + DelM2)*(1 + ten)));
        if(ic_regulator_smg>0 && (fabs(den2)<ic_regulator_smg)){
          den2 = copysign(ic_regulator_smg,den2);
        }

        c2vp  = (2*(-1 + Omx)*run*(32*(DelM2 + Omx) + 16*(-1 + Omx)*ten + pow(bra,2)*(1 + DelM2)*(2 + ten) - 2*bra*(1 + DelM2)*(4 + ten)))/
                den2;

        c1vp  = (2*(-1 + Omx)*run*(3*pow(bra,3)*pow(1 + DelM2,2)*ten*(14 + 9*ten) + 16*(-6*(DelM2 + Omx)*ten*(-2*(DelM2 + Omx) +
                (2 + DelM2 - Omx)*ten) + (1 + DelM2)*kin*(2 + ten)*(6*(DelM2 + Omx) + (-1 + 2*DelM2 + 3*Omx)*ten)) -
                2*bra*(1 + DelM2)*(kin*(2 + ten)*(4*(7 + 6*DelM2 - Omx) + (17 + 15*DelM2 - 2*Omx)*ten) - 12*ten*(8*(DelM2 + Omx) +
                (4 + 9*DelM2 + 5*Omx)*ten)) + pow(bra,2)*(1 + DelM2)*(-6*ten*(34 + 26*DelM2 - 8*Omx + (27 + 23*DelM2 - 4*Omx)*ten) +
                (1 + DelM2)*kin*(24 + 26*ten + 7*pow(ten,2)))))/(den1*den2);

        den2  = (bra + 4*Omx - 4*ten + bra*ten + 2*Omx*ten + DelM2*(4 + bra - 2*ten + bra*ten));
        if(ic_regulator_smg>0 && (fabs(den2)<ic_regulator_smg)){
          den2 = copysign(ic_regulator_smg,den2);
        }
        c0vp  = (4*(-1 + Omx)*(3*pow(bra,2) + 8*kin - bra*kin + DelM2*(3*pow(bra,2) - bra*(-12 + kin) + 8*kin) + 12*bra*Omx)*run*
                (-6*(-2 + ten)*ten + kin*(2 + 3*ten + pow(ten,2)) + 3*bra*(-4 + ten + 2*pow(ten,2))))/((1 + DelM2)*(3*pow(bra,2) + 2*kin)*
                den1*den2);

        den2  = 4.*(9.*bra*(1. + DelM2) + (1. + DelM2)*kin - 12.*(DelM2 + Omx))*(3.*pow(bra,2.)*
                (1. + DelM2) + 2.*kin*(DelM2 + Omx))*(-6.*(DelM2 + Omx)*(-2. + ten) + 9.*bra*(1. + DelM2)*(-1. + ten) + 2.*(1. + DelM2)*
                kin*(1. + ten));
        if(ic_regulator_smg>0 && (fabs(den2)<ic_regulator_smg)){
          den2 = copysign(ic_regulator_smg,den2);
        }

        c2hp  =  ((-1. + Omx)*run*(27.*pow(bra,4)*pow(1. + DelM2,2.)*ten*(432. - 373.*ten + 6.*pow(ten,2.)) + 9.*pow(bra,3.)*(1. + DelM2)*
                (864.*(DelM2 + Omx)*(-2. + ten)*ten + (1. + DelM2)*kin*(864. - 698.*ten - 329.*pow(ten,2.) + 6.*pow(ten,3.))) -
                3.*pow(bra,2.)*(1. + DelM2)*kin*(3456.*(DelM2 + Omx) - 4320.*(DelM2 + Omx)*ten + 6.*(1. + 446.*DelM2 + 445.*Omx)*
                pow(ten,2.) - 36.*(DelM2 + Omx)*pow(ten,3.) + (1. + DelM2)*kin*(768. + 227.*ten - 259.*pow(ten,2.) + 12.*pow(ten,3.))) -
                2.*pow(kin,2.)*(-6.*(DelM2 + Omx)*(-768.*(DelM2 + Omx) + (-1. + 191.*DelM2 + 192.*Omx)*pow(ten,2.)) + pow(1. + DelM2,2.)*
                pow(kin,2.)*(-14. - 19.*ten - 4.*pow(ten,2.) + pow(ten,3.)) - (1. + DelM2)*kin*(-384.*(DelM2 + Omx) +
                (1. - 851.*DelM2 - 852.*Omx)*ten + (1. - 317.*DelM2 - 318.*Omx)*pow(ten,2.) + 6.*(DelM2 + Omx)*pow(ten,3.))) -
                6.*bra*kin*(-1152.*pow(DelM2 + Omx,2.)*(-2. + ten)*ten + pow(1. + DelM2,2.)*pow(kin,2.)*(-32. - 99.*ten - 40.*pow(ten,2.) +
                3.*pow(ten,3.)) - (1. + DelM2)*kin*(1440.*(DelM2 + Omx) - 2.*(1. + 325.*DelM2 + 324.*Omx)*ten + (1. - 905.*DelM2 - 906.*Omx)*
                pow(ten,2.) + 12.*(DelM2 + Omx)*pow(ten,3.)))))/(den2*den1);

        c1hp  = ((-1 + Omx)*run*(135*pow(bra,4)*pow(1 + DelM2,3)*ten*(288 - 229*ten + 6*pow(ten,2)) + 9*pow(bra,3)*
                pow(1 + DelM2,2)*(2880*(DelM2 + Omx)*(-2 + ten)*ten + (1 + DelM2)*kin*(3744 - 1780*ten - 1855*pow(ten,2) +
                66*pow(ten,3))) + 2*kin*(3456*pow(DelM2 + Omx,3)*(-2 + ten)*ten + 6*(1 + DelM2)*kin*(DelM2 + Omx)*(-2112*(DelM2 + Omx) -
                4*(1 + 25*DelM2 + 24*Omx)*ten + 3*(-1 + 95*DelM2 + 96*Omx)*pow(ten,2)) - pow(1 + DelM2,3)*pow(kin,3)*
                (-14 - 19*ten - 4*pow(ten,2) + pow(ten,3)) + pow(1 + DelM2,2)*pow(kin,2)*(-528*(DelM2 + Omx) +
                (1 - 1523*DelM2 - 1524*Omx)*ten + (1 - 545*DelM2 - 546*Omx)*pow(ten,2) + 18*(DelM2 + Omx)*pow(ten,3))) +
                3*pow(bra,2)*pow(1 + DelM2,2)*kin*((1 + DelM2)*kin*(-1296 - 2087*ten - 449*pow(ten,2) + 36*pow(ten,3)) +
                6*(-3072*(DelM2 + Omx) + 1532*(DelM2 + Omx)*ten + (-5 + 28*DelM2 + 33*Omx)*pow(ten,2) + 18*(DelM2 + Omx)*pow(ten,3))) -
                6*bra*(1 + DelM2)*kin*(576*pow(DelM2 + Omx,2)*(-4 + 5*ten) + pow(1 + DelM2,2)*pow(kin,2)*(-4 - 61*ten - 32*pow(ten,2) +
                pow(ten,3)) - (1 + DelM2)*kin*(3552*(DelM2 + Omx) - 4*(1 + 121*DelM2 + 120*Omx)*ten - (1 + 1279*DelM2 + 1278*Omx)*
                pow(ten,2) + 36*(DelM2 + Omx)*pow(ten,3)))))/(den2*(1 + DelM2)*den1);

        den2  = (9*bra*(-1 + ten) + 2*(kin + 6*Omx + kin*ten - 3*Omx*ten) + DelM2*(9*bra*(-1 + ten) +
                2*(6 + kin - 3*ten + kin*ten)));
        if(ic_regulator_smg>0 && (fabs(den2)<ic_regulator_smg)){
          den2 = copysign(ic_regulator_smg,den2);
        }

        c0hp  =  -((-1 + Omx)*run*(9*pow(bra,3)*(-288 + 98*ten + 119*pow(ten,2) + 6*pow(ten,3)) + 6*bra*(288*Omx*(-2 + ten)*
                ten + pow(kin,2)*(16 + 85*ten + 48*pow(ten,2) + 3*pow(ten,3)) + kin*(288*Omx + (314 - 216*Omx)*ten -
                (163 + 246*Omx)*pow(ten,2) - 12*(-1 + Omx)*pow(ten,3))) + 3*pow(bra,2)*(kin*(-480 + 335*ten + 383*pow(ten,2) +
                18*pow(ten,3)) - 6*(-192*Omx + 48*(-3 + Omx)*ten + (85 + 83*Omx)*pow(ten,2) + 6*(-1 + Omx)*pow(ten,3))) +
                2*kin*(6*ten*(-192*Omx + (-1 + 97*Omx)*ten) + pow(kin,2)*(18 + 29*ten + 12*pow(ten,2) + pow(ten,3)) -
                kin*(192*Omx + (-107 + 300*Omx)*ten + (31 + 114*Omx)*pow(ten,2) + 6*(-1 + Omx)*pow(ten,3))) +
                DelM2*(9*pow(bra,3)*(-288 + 98*ten + 119*pow(ten,2) + 6*pow(ten,3)) + 2*kin*(576*(-2 + ten)*ten -
                kin*(192 + 193*ten + 145*pow(ten,2)) + pow(kin,2)*(18 + 29*ten + 12*pow(ten,2) + pow(ten,3))) +
                6*bra*(288*(-2 + ten)*ten + kin*(288 + 98*ten - 409*pow(ten,2)) + pow(kin,2)*(16 + 85*ten +
                48*pow(ten,2) + 3*pow(ten,3))) + 3*pow(bra,2)*(-144*(-8 - 4*ten + 7*pow(ten,2)) + kin*(-480 + 335*ten +
                383*pow(ten,2) + 18*pow(ten,3))))))/(2.*(1 + DelM2)*(3*pow(bra,2) + 2*kin)*den1*den2);


        // Solve cubic to find exponents for h and x_smg. Find mode closest to adiabatic.
        // Ignore any new faster modes, since they will have appeared at some point since
        // the inital test and therefore we should accept the slow leakage into them
        // as part of the actual solution.

        rf_solve_poly_3(c3,c2+c2hp,c1+c1hp,c0+c0hp,sols,&complex);

        dnh = sols[0];    //want closest to zero
        for (i=0; i<3;i+=1){
          if (fabs(sols[i]) < fabs(dnh)){
            dnh = sols[i];
          }
        }

        rf_solve_poly_3(c3,c2+c2vp,c1+c1vp,c0+c0vp,sols,&complex);

        dnv = sols[0];    //want closest to zero
        for (i=0; i<3;i+=1){
          if (fabs(sols[i]) < fabs(dnv)){
            dnv = sols[i];
          }
        }


      if (ppt->perturbations_verbose > 6)
          printf("Mode k=%e: ICs: grows with tau^3+nv with approx. nv=%f, while h -- with nh=%f at a=%e. dM=%f\n",k,dnv,dnh,a,DelM2);

      // Now we can set the initial ratio of amplitudes for x_smg and h.The expression is left with dnh/dnv terms implicit.

      //  The amplitude of x_smg and other field seems to be better approximated by using a weighed average
      //  between dnh and dnv, instead of the initial estimate. Store the dnv in dn for setting V_prime.
      //  Note that this is totally empirical.

      dn=dnv;
      dnv=(2*dnv+3*dnh)/5.;

      den2 = (2.*(3*pow(bra,3)*(2*(2 + run)*
                    (3 + 2*run - 3*ten) + pow(dnv,3)*(2 + ten) + pow(dnv,2)*(16 + 7*ten + run*(3 + ten)) +
                    dnv*(36 + pow(run,2) + 10*ten + run*(16 + 3*ten))) + pow(bra,2)*(6*pow(dnv,3)*(run - ten) -
                    dnv*kin*(2 + run)*(1 + ten) + 6*pow(dnv,2)*(-4*Omx + pow(run,2) - run*(-5 + ten) - (5 + 2*Omx)*ten) +
                    6*dnv*(-12 + 2*(-5 + Omx)*run + pow(run,2) + (-3 + 2*Omx)*run*ten - 2*Omx*(8 + 3*ten)) -
                    4*(54 + 12*Omx + 21*pow(run,2) - 6*(9 + Omx)*ten + kin*(2 + run)*(1 + ten) -
                    3*run*(-29 + 4*Omx + (6 + 4*Omx)*ten))) + 2*bra*((4 + dnv)*kin*(3*(2 + run)*(1 + ten) +
                    pow(dnv,2)*(2 + ten) + dnv*(8 + 3*ten + run*(3 + ten))) + 12*((-1 + 8*Omx + dnv*(-1 + 2*Omx))*
                    pow(run,2) + 6*Omx*(4 - 3*ten) + dnv*(4*Omx + 3*ten - 5*Omx*ten) + run*(4 + 22*Omx + 3*ten -
                    12*Omx*ten + dnv*(-3 + 7*Omx + ten - 2*Omx*ten)))) + 4*(pow(dnv,3)*kin*(run - ten) +
                    pow(dnv,2)*kin*(-4*Omx + pow(run,2) - run*(-5 + ten) - 5*ten - 2*Omx*ten) - 8*(Omx*(kin + 12*Omx)*run +
                    (-3 + 6*Omx)*pow(run,2) + (3 + (-6 + kin)*Omx)*run*ten + 2*Omx*(kin + 6*Omx + kin*ten - 3*Omx*ten)) +
                    2*dnv*(-12*(-1 + Omx)*Omx*(run - ten) + kin*(2*pow(run,2) - 2*(5*Omx + ten + 3*Omx*ten) -
                    run*(-2 + Omx + (2 + Omx)*ten)))) + pow(DelM2,2)*(-96*(2 + run)*(2 + run - ten) +
                    4*(4 + dnv)*kin*(pow(dnv,2)*(run - ten) - 2*(2 + run)*(1 + ten) + dnv*(-4 + run + pow(run,2) - 3*ten -
                    run*ten)) + 3*pow(bra,3)*(2*(2 + run)*(3 + 2*run - 3*ten) + pow(dnv,3)*(2 + ten) +
                    pow(dnv,2)*(16 + 7*ten + run*(3 + ten)) + dnv*(36 + pow(run,2) + 10*ten + run*(16 + 3*ten))) +
                    2*bra*(pow(dnv,3)*kin*(2 + ten) + 12*(2 + run)*(12 + kin + 7*run - 9*ten + kin*ten) +
                    pow(dnv,2)*kin*(16 + 7*ten + run*(3 + ten)) + dnv*(12*(2 + run)*(2 + run - ten) +
                    kin*(38 + 15*run + 18*ten + 7*run*ten))) + pow(bra,2)*(6*pow(dnv,2)*(-4 + pow(run,2) -
                    run*(-5 + ten) - 7*ten) + 6*pow(dnv,3)*(run - ten) - 4*(2 + run)*(33 + kin + 21*run - 30*ten + kin*ten) -
                    dnv*(kin*(2 + run)*(1 + ten) + 6*(28 - pow(run,2) + 6*ten + run*(8 + ten))))) + 2*DelM2*(-48*(dnv*(-1 + Omx)*
                    (run - ten) + 2*Omx*(2 + run)*(2 + run - ten)) + 4*(4 + dnv)*kin*(pow(dnv,2)*(run - ten) - (1 + Omx)*(2 + run)*
                    (1 + ten) + dnv*(-2*(1 + Omx) + run + pow(run,2) - (2 + Omx)*ten - run*ten)) + 3*pow(bra,3)*(2*(2 + run)*
                    (3 + 2*run - 3*ten) + pow(dnv,3)*(2 + ten) + pow(dnv,2)*(16 + 7*ten + run*(3 + ten)) + dnv*(36 + pow(run,2) +
                    10*ten + run*(16 + 3*ten))) + pow(bra,2)*(6*pow(dnv,3)*(run - ten) - dnv*kin*(2 + run)*(1 + ten) +
                    6*pow(dnv,2)*(-2*(1 + Omx) + pow(run,2) - run*(-5 + ten) - (6 + Omx)*ten) + 6*dnv*(-4*(5 + 2*Omx) + pow(run,2) -
                    3*(1 + Omx)*ten + run*(-9 + Omx + (-2 + Omx)*ten)) - 4*(60 + 6*Omx + 21*pow(run,2) - 57*ten - 3*Omx*ten +
                    kin*(2 + run)*(1 + ten) - 3*run*(-27 + 2*Omx + 2*(4 + Omx)*ten))) + 2*bra*(pow(dnv,3)*kin*(2 + ten) +
                    pow(dnv,2)*kin*(16 + 7*ten + run*(3 + ten)) + 12*((3 + 4*Omx)*pow(run,2) + kin*(2 + run)*(1 + ten) -
                    3*(1 + Omx)*(-4 + 3*ten) + run*(15 + 11*Omx - 3*ten - 6*Omx*ten)) + dnv*(6*(4 + 4*Omx + run +
                    2*Omx*pow(run,2) + Omx*run*(7 - 2*ten) + ten - 5*Omx*ten) + kin*(38 + 18*ten + run*(15 + 7*ten)))))));

      if(ic_regulator_smg>0 && (fabs(den2)<ic_regulator_smg)){
          den2 = copysign(ic_regulator_smg,den2);
      }

      amplitude  =  -((2 + dnv)*(pow(bra,3)*(2 + run)*(1 + ten) + (8 - 6*bra + pow(bra,2))*pow(DelM2,2)*(2 + run)*
                    (4 + bra + 2*run - 2*ten + bra*ten) + 2*pow(bra,2)*(-6 + 4*Omx + run + pow(run,2) +
                    2*(-5 + Omx)*ten - 4*run*ten) + 16*((-1 + 2*Omx)*pow(run,2) + 2*Omx*(2*Omx + (-2 + Omx)*ten) +
                    run*(4*Omx + ten - 2*Omx*ten)) - 4*bra*(8*Omx + 3*pow(run,2) + 2*(-6 + Omx)*ten -
                    run*(-16 + 6*Omx + ten + 4*Omx*ten)) + 2*DelM2*(pow(bra,3)*(2 + run)*(1 + ten) +
                    2*pow(bra,2)*(-4 + 2*Omx + run + pow(run,2) - 9*ten + Omx*ten - 4*run*ten) +
                    16*(4*Omx + Omx*pow(run,2) - 2*ten + run*(2 + 2*Omx - Omx*ten)) - 4*bra*(4 + 4*Omx + 3*pow(run,2) -
                    11*ten + Omx*ten - run*(-13 + 3*Omx + 3*ten + 2*Omx*ten)))))/den2;


      // Now we use the above result to calculate the initial conditions for the all fields

      /* eta (grav. potential) = curvature perturbation on super-horizon scales.
      * When h is normalised to C (ktau)^2+dnh we actually have
      * eta = 2C(A1_eta_smg + A2_eta_smg*(k tau)^2)tau^dnh since the x_smg perturbation gravitates
      * We are going to redefine the amplitude of h and all the other species by dividing
      * by A1_eta_smg to keep eta equal to thecurvature perturbation at large scales, curv,
      * to avoid degeneracy betwen early modified gravity and A_S.
      * So we have
      *  eta = curv ( 1 + A2_eta_smg/A1_eta_smg * (k tau)^2)
      *
      * You can see that all these terms consist of a slightly corrected standard result
      * (with modifications for Omx and DelM2 and dnh) plus a new term which is amplitude calculated
      * above times a coefficients of order bra, Omx, i.e. irrelevant when no early MG
      */

      den1 = (kin*(2 + ten) + 3*bra*(-run + ten));
      if(ic_regulator_smg>0 && (fabs(den1)<ic_regulator_smg)){
          den1 = copysign(ic_regulator_smg,den1);
      }
      den2 = (4.*(30*(1 + DelM2) + 5*(1 + DelM2)*dnv*(5 + dnv) - 8*fracnu*(-1 + Omx)));
      if(ic_regulator_smg>0 && (fabs(den2)<ic_regulator_smg)){
          den2 = copysign(ic_regulator_smg,den2);
      }


      A1_eta_smg  =  ((2 + dnh)*(-(bra*(1 + DelM2)*kin) + 12*bra*(DelM2 + Omx) + 3*pow(bra,2)*(1 + DelM2)*(1 + dnh + run) +
                      2*(1 + DelM2)*kin*(4 + dnh + run)))/(8.*(1 + DelM2)*den1) +
                      (amplitude*((1 + DelM2)*pow(kin,2)*(4 + dnv) + 3*bra*(1 + DelM2)*kin*(14 + 3*dnv) -
                      72*bra*(DelM2 + Omx) - 18*pow(bra,2)*(1 + DelM2)*(-3 + run) - 12*kin*((4 + dnv)*(DelM2 + Omx) +
                      (1 + DelM2)*run)))/(4.*(1 + DelM2)*den1);

      A2_eta_smg  =   ((5 + 4*fracnu)*(-1 + Omx))/(6.*(30*(1 + DelM2) + 5*(1 + DelM2)*dnh*(5 + dnh) -
                      8*fracnu*(-1 + Omx))) + (5*amplitude*(3 + dnv)*(bra*(1 + DelM2)*(4 + dnv) -
                      4*(DelM2 + Omx)))/den2;

      eta = ppr->curvature_ini * (1. + A2_eta_smg/A1_eta_smg*ktau_two);

      if(ppt->perturbations_verbose > 8)
        printf("       ampl = %e, eta A1 = %e (%e), A2 = %e (%e), ktau^2 = %e, curv = %e \n",amplitude,A1_eta_smg,1.,A2_eta_smg,
                 -1./12./(15.+4.*fracnu)*(5.+4.*s2_squared*fracnu - (16.*fracnu*fracnu+280.*fracnu+325)/10./(2.*fracnu+15.)*tau*om),
                 ktau_two, ppr->curvature_ini );

      // Initial conditions for MG scalar assuming that we have standard adiabatic attractor
      //  Note thatthe derivative is set using the real dnv calculated initially.

      ppw->pv->y[ppw->pv->index_pt_x_smg]  = 0.5*amplitude*ktau_two*tau*(ppr->curvature_ini)/A1_eta_smg;
      ppw->pv->y[ppw->pv->index_pt_x_prime_smg] = (3+dn)*a*ppw->pvecback[pba->index_bg_H]*ppw->pv->y[ppw->pv->index_pt_x_smg];


      // Correct all shear-free species by reducing amplitude by A1_eta_smg and  the velocities for dn

      ppw->pv->y[ppw->pv->index_pt_delta_g] /= A1_eta_smg;
      ppw->pv->y[ppw->pv->index_pt_theta_g] /= A1_eta_smg/3*(3+dnh);
      ppw->pv->y[ppw->pv->index_pt_delta_b] /= A1_eta_smg;
      ppw->pv->y[ppw->pv->index_pt_theta_b] /= A1_eta_smg/3*(3+dnh);
      if (pba->has_cdm == _TRUE_)
      ppw->pv->y[ppw->pv->index_pt_delta_cdm] /= A1_eta_smg;
      if (pba->has_dcdm == _TRUE_)
        ppw->pv->y[ppw->pv->index_pt_delta_dcdm] /= A1_eta_smg;
      if (pba->has_fld == _TRUE_) {
        ppw->pv->y[ppw->pv->index_pt_delta_fld] /= A1_eta_smg;
        ppw->pv->y[ppw->pv->index_pt_theta_fld] /= A1_eta_smg/3*(3+dnh);
      }
      if (pba->has_scf == _TRUE_) {
        ppw->pv->y[ppw->pv->index_pt_phi_scf] /= A1_eta_smg;
        ppw->pv->y[ppw->pv->index_pt_phi_prime_scf] /= A1_eta_smg/3*(3+dnh);
      }

      if ((pba->has_ur == _TRUE_) || (pba->has_ncdm == _TRUE_) || (pba->has_dr == _TRUE_)) {
      // Species with shear have a corrected initial condition

        A_v_nu_smg  =   (amplitude*(-(bra*(1 + DelM2)*(4 + dnv)) + 4*(DelM2 + Omx)))/(30*(1 + DelM2) +
                        5*(1 + DelM2)*dnv*(5 + dnv) - 8*fracnu*(-1 + Omx)) + (-9*(1 + DelM2)*dnh*(5 + dnh) +
                        8*fracnu*(-1 + Omx) - 2*(23 + 27*DelM2 + 4*Omx))/(12.*(3 + dnh)*(30*(1 + DelM2) +
                        5*(1 + DelM2)*dnh*(5 + dnh) - 8*fracnu*(-1 + Omx)));

        A_sigma_nu_smg =  (amplitude*(3 + dnv)*(bra*(1 + DelM2)*(4 + dnv) - 4*(DelM2 + Omx)))/(30*(1 + DelM2) +
                          5*(1 + DelM2)*dnv*(5 + dnv) - 8*fracnu*(-1 + Omx)) + ((1 + DelM2)*dnh*(5 + dnh) +
                          2*(2 + 3*DelM2 + Omx))/(3.*(30*(1 + DelM2) + 5*(1 + DelM2)*dnh*(5 + dnh) -
                          8*fracnu*(-1 + Omx)));



        delta_ur = ppw->pv->y[ppw->pv->index_pt_delta_g]; /* has already been rescaled above! */

        theta_ur =  A_v_nu_smg/A1_eta_smg* k*ktau_three* ppr->curvature_ini;
        // /36./(4.*fracnu+15.) * (4.*fracnu+11.+12.*s2_squared-3.*(8.*fracnu*fracnu+50.*fracnu+275.)/20./(2.*fracnu+15.)*tau*om) * ppr->curvature_ini * s2_squared; /* velocity of ultra-relativistic neutrinos/relics, modified */ //TBC

        shear_ur =  A_sigma_nu_smg/A1_eta_smg* ktau_two * ppr->curvature_ini;
        // /(45.+12.*fracnu) * (3.*s2_squared-1.) * (1.+(4.*fracnu-5.)/4./(2.*fracnu+15.)*tau*om) * ppr->curvature_ini;//TBC /s2_squared; /* shear of ultra-relativistic neutrinos/relics */  //TBC:0

        //TODO: needs to be modified?
        l3_ur = ktau_three/A1_eta_smg*2./7./(12.*fracnu+45.)* ppr->curvature_ini;//ILS

        if(ppt->perturbations_verbose > 8)
            printf("       fracnu = %e, A_v_nu = %e (%e), A_sigma_nu = %e (%e), th_ur/th_g = %e, x_smg/vm = %e\n", fracnu, A_v_nu_smg,
                     -1./36./(4.*fracnu+15.) * (4.*fracnu+11.+12.*s2_squared-3.*(8.*fracnu*fracnu+50.*fracnu+275.)/20./(2.*fracnu+15.)*tau*om),
                     A_sigma_nu_smg,
                     1./(45.+12.*fracnu) * (3.*s2_squared-1.) * (1.+(4.*fracnu-5.)/4./(2.*fracnu+15.)*tau*om),
                     theta_ur/ppw->pv->y[ppw->pv->index_pt_theta_g],k*k*ppw->pv->y[ppw->pv->index_pt_x_smg]/ppw->pv->y[ppw->pv->index_pt_theta_g]);
        if(pba->has_dr == _TRUE_) delta_dr = delta_ur;
        }// end neutrino part
        if(ppt->perturbations_verbose > 5)
          printf("Mode k=%e: Adiabatic mode gravitating_attr IC for early smg: ",k);

    } //end of gravitation_attr ICs

    if (ppt->pert_initial_conditions_smg == kin_only)
    {
      ppw->pv->y[ppw->pv->index_pt_x_smg] = ktau_two * dt;
      ppw->pv->y[ppw->pv->index_pt_x_prime_smg] = 2 * k * k * tau * dt;
      if (ppt->perturbations_verbose > 5)
        printf("Mode k=%e: Adiabatic mode kin_only IC for smg: ", k);
    }

    if (ppt->pert_initial_conditions_smg == single_clock)
    {
      // single_clock IC given with respect to photons (because there are always photons)
      ppw->pv->y[ppw->pv->index_pt_x_smg] = -1 / (4. * ppw->pvecback[pba->index_bg_H]) * ppw->pv->y[ppw->pv->index_pt_delta_g];
      // Single clock IC => x^prime = 0
      ppw->pv->y[ppw->pv->index_pt_x_prime_smg] = 0.;
      if (ppt->perturbations_verbose > 5)
        printf("Mode k=%e: Adiabatic mode single clock IC for smg: ", k);
    }

    if (ppt->pert_initial_conditions_smg == zero)
    {
      ppw->pv->y[ppw->pv->index_pt_x_smg] = 0.;
      ppw->pv->y[ppw->pv->index_pt_x_prime_smg] = 0. ;

      if(ppt->perturbations_verbose > 5)
        printf("Mode k=%e: Aduabatic model zero IC for smg: ",k);
    }

    if (ppt->pert_initial_conditions_smg == ext_field_attr)
    {


        nexpo=2; // h = C tau^2


        calc_extfld_ampl(nexpo,  kin, bra, dbra, run, ten, DelM2, Omx, wx,
                        l1, l2, l3, l4, l5, l6,l7,l8, cs2num, Dd, ppr->pert_ic_regulator_smg,
                      &amplitude);

        ppw->pv->y[ppw->pv->index_pt_x_smg]  = amplitude*ktau_two*tau*(ppr->curvature_ini);
        ppw->pv->y[ppw->pv->index_pt_x_prime_smg] = (nexpo+1)*a*ppw->pvecback[pba->index_bg_H]*ppw->pv->y[ppw->pv->index_pt_x_smg];


        if(ppt->perturbations_verbose > 5)
          printf("Mode k=%e: Adiabatic mode ext_field_attr IC for smg: ",k);


    }// End external-field attractor ICs


    x_smg = ppw->pv->y[ppw->pv->index_pt_x_smg];
    xp_smg = ppw->pv->y[ppw->pv->index_pt_x_prime_smg];

  } //end adiabatic mode dynamical ICs for smg
  else
  {
    //  Adiabatic mode Quasi-Static initial conditions

    /*  We reach here if initialisation for a mode happens in quasi-static conditions.
    Before, we already have made sure that the initialisation happens early enough so that
    all modes are either quasi-static or dynamical. Here we test that if they are QS, the initial
    superhorizon configuration is not too different from GR. If it were, then we can't trust
    that the curvature perturbation is conserved and therefore cannot connect
    the amplitude at initialisation with that of primordial power spectrum.

    Roughly, the QS solution for x_smg is given by

      ((D cs^2 k^2 +M^2 a^2 )x_smg_QS = coeff1 * k^2 eta + coeff2 * delta_rad

    while the effect of this is given by the (0i) Einstein equation

    eta' = theta_rad + coeff3* x_smg

    We know that the standard solution for eta' is k^2*tau, so we will require that the QS solution
    at the scale of initialisation is no more than an order 1 correction to that. If this test is failed
    then quit with error. If it is passed, we don't actually change any ICs, since all matter species are standard
    and the x_smg/x_smg' are assigned in perturb_einstein
    */

    double delta_g = 0., delta_rho = 0., delta_rho_r = 0., delta_p = 0;
    double rho_plus_p_theta = 0., rho_plus_p_theta_r = 0.;
    double contribfromx = 0., contribfromtheta = 0., contribratio = 0.;

    // Approximate that all radiation has same delta/theta as photons and that pressure is 1/3 of radiation density

    delta_g = ppw->pv->y[ppw->pv->index_pt_delta_g];
    delta_rho = rho_r * delta_g;
    delta_rho_r = delta_rho;
    delta_p = delta_rho / 3.;
    rho_plus_p_theta = 4. / 3. * rho_r * ppw->pv->y[ppw->pv->index_pt_theta_g];
    rho_plus_p_theta_r = rho_plus_p_theta;

    // Below QS equations are copied from perturb_einstein: make sure any changes there are reflected
    // QS-IC-change

    x_smg = (4. * cs2num * pow(k, 2) * M2 * eta + 6. * l2 * delta_rho * pow(a, 2) +
              ((-2.) + bra) * 9. * bra * delta_p * pow(a, 2)) *
             1. / 4. * pow(H, -1) * pow(M2, -1) * pow(a, -1) * pow(cs2num * pow(k, 2) + (-4.) * pow(H, 2) * l8 * pow(a, 2), -1);

    g1 = cs2num * pow(k / (a * H), 2) - 4. * l8;

    g2 =  (2. - bra) * (g1 + (3. * bra + kin) * bra * rho_r * pow(H, -2) * pow(M2, -1) -
          bra * cs2num * pow(k / (a * H), 2) / 2.) / 2. - 3. / 4. * (3. * bra + kin) * (rho_tot + p_tot) *
          pow(H, -2) * l2 * pow(M2, -1);

    g3 = -(2. * (2. - bra) * bra * rho_r - 3. * (rho_tot + p_tot) * l2) * (18. - 18. * (rho_tot + p_tot) * pow(H, -2) * pow(M2, -1) - 15. * bra - 2. * kin + 9. * (2. - bra) * (p_tot + p_smg) * pow(H, -2) -
          2. * bra * pow(k / (a * H), 2)) * pow(H, -2) * pow(M2, -1) + 2. * (2. - bra) * cs2num * (5. - bra - 3. * (rho_tot + p_tot) * pow(M2, -1) * pow(H, -2) + 9. * (p_tot + p_smg) * pow(H, -2)) * pow(k / (a * H), 2) +
          4. * (2. - bra) * (pow(k / (a * H), 2) * cs2num_p - 4. * l8_p) / (a * H);

    xp_smg = 3. / 2. * (pow(2. - bra, 2) * bra * pow(H, -2) * pow(M2, -1) * delta_rho_r +
              (3. / 2. * (2. - bra) * cs2num * (p_tot + p_smg) * pow(H, -2) - pow(H, -2) * l2 * (p_tot + rho_tot) / M2 +
              (2. - bra) * pow(H, -1) * cs2num_p / a / 3. + (2. - bra) * cs2num / 2. - cs2num * g3 / g1 / 12. +
              2. / 3. * (2. - bra) * bra * rho_r * pow(H, -2) / M2) * pow(k / (a * H), 2) * eta + (2. - bra) * (cs2num - l2) * pow(M2 * a, -1) * pow(H, -3) * rho_plus_p_theta / 2. +
              3. / 2. * (2. - bra) * ((2. - bra) * (-7. + 2. * run) / 4. * bra + 1. / 8. * bra * g3 / g1 - l2 -
              9. / 4. * (2. - bra) * bra * (p_tot + p_smg) * pow(H, -2) - (1. - bra) * pow(a * H, -1) * bra_p) * pow(H, -2) * pow(M2, -1) * delta_p +
              ((2. - bra) * bra * rho_r * pow(H, -2) * pow(M2, -1) - g3 / g1 * l2 / 8. - (6. * rho_tot / M2 - (2. - bra - 4. * run + 2. * bra * run) * pow(H, 2)) / 4. * pow(H, -2) * l2 -
              3. / 4. * (2. / M2 - 6. + 3. * bra) * pow(H, -2) * l2 * p_tot + 9. / 4. * (2. - bra) * pow(H, -2) * l2 * p_smg + (2. - bra) / 2. * pow(H, -1) * l2_p * pow(a, -1)) * pow(M2, -1) * pow(H, -2) * delta_rho +
              pow(2. - bra, 2) * bra * pow(H, -3) * pow(M2 * a, -1) * rho_plus_p_theta_r / 4.) * pow(g2, -1);

    // Now test to make sure that x_smg_QS contribution to (0i) equation is small compared with that from radiation
    // If fail -> quit

    contribfromx = a * H / 2. * bra * xp_smg + (a * Hprime + pow(a_prime_over_a, 2) / 2. * bra +
                    3. * a * a / (2. * M2) * 4. / 3. * rho_r) * x_smg;
    contribfromtheta = 3. * a * a * rho_plus_p_theta / (2. * k * k * M2);
    contribratio = fabs(contribfromx / contribfromtheta);

    class_test(ppr->pert_qs_ic_tolerance_test_smg > 0 && (contribratio > ppr->pert_qs_ic_tolerance_test_smg),
                ppt->error_message,
                "\n     Cannot set adiabatic initial conditions for smg pertubations: quasi-static configuration with large correction of gravity required superhorizon. Loss of connection to priordial power spectrum. \n");

    // If contribratio small enough, don't fail and start evolving perturbations.
    // x_smg/x_smg' get set in perturbeinstein!

    if (ppt->perturbations_verbose > 5)
    {
      printf("\nMode k=%e: Quasi-static ICs for smg: ", k);
    }
  };

  //print the scalar's IC values, whatever the ICs
  if(ppt->perturbations_verbose > 5)
    printf(" x_smg = %e, x_smg'= %e \n", x_smg,xp_smg);

  *ptr_eta = eta;
  *ptr_delta_ur = delta_ur;
  *ptr_theta_ur = theta_ur;
  *ptr_shear_ur = shear_ur;
  *ptr_l3_ur = l3_ur;
  *ptr_delta_dr = delta_dr;

  return _SUCCESS_;
}

/*  SMG and isocurvature:
    *   if we have "zero" or "single_clock" ICs for SMG, then  leave x_smg
        and x_prime_smg at the initalisation value of 0
        and let it find a proper solution.
    *   grav_attr isocurvature would backreact and has NOT been implemented.
        We have already failed earlier if it is asked for.

    *   Only need to implement ext_field_attr.
        We assume that there is no backreaction of x_smg onto the other species
        and therefore the other species' isocurvature ICs do not change.
        However, x_smg is determined by a particular solution of the
        evolution equation with a source h scaling with a different exponent
        for each isocurvature mode type

        We only take the leading-order power-law in om
        since we start very deep in RD

        The calc_extfld_ampl function produces the amplitude for x_smg
        on the assumption that the normalisation is  h = 1/2 * tau^n
        We correct for this normalisation by using coeff_isocurv_smg
        defining is according to the normalisation in BMT99
        adjusted for the CLASS redefinition. However, for NID and NIV,
        we need to find the leading order term in h which is not
        from fracb

TODO: In principle should also test if sg is initialised as QS whether gravity is modified already, just like
        we do for adiabatic modes.
TODO: Gravitating attractor isocurvature modes.


*/


int perturb_isocurvature_cdm_ic_smg(
     struct precision * ppr,
     struct background * pba,
     struct perturbs * ppt,
     struct perturb_workspace * ppw,
     double tau,
     double k,
     double fraccdm,
     double om
   ) {

  int qs_array_smg[] = _VALUES_QS_SMG_FLAGS_;

  if((ppt->pert_initial_conditions_smg==ext_field_attr) && (qs_array_smg[ppw->approx[ppw->index_ap_qs_smg]] == 0))
  //only set ICs for smg if have smg, we are in exernal field attractor and we are *not* quasi-static
  {
   /* TODO_EB: revisit isocurvature initial conditions for beyond horndeski and oscillations */


    double coeff_isocurv_smg;

    int nexpo= 1;

    double a = ppw->pvecback[pba->index_bg_a];
    double H = ppw->pvecback[pba->index_bg_H];
    double rho_smg = ppw->pvecback[pba->index_bg_rho_smg];
    double p_smg = ppw->pvecback[pba->index_bg_p_smg];

    double wx = p_smg/rho_smg;
    double Omx = rho_smg/pow(H,2);
    double kin = ppw->pvecback[pba->index_bg_kineticity_smg];
    double bra = ppw->pvecback[pba->index_bg_braiding_smg];
    double bra_p = ppw->pvecback[pba->index_bg_braiding_prime_smg];
    double dbra= bra_p/(a*H) ; //Read in log(a) diff of braiding
    double run = ppw->pvecback[pba->index_bg_mpl_running_smg];
    double ten = ppw->pvecback[pba->index_bg_tensor_excess_smg];
    double l1 = ppw->pvecback[pba->index_bg_lambda_1_smg];
    double l2 = ppw->pvecback[pba->index_bg_lambda_2_smg];
    double l3 = ppw->pvecback[pba->index_bg_lambda_3_smg];
    double l4 = ppw->pvecback[pba->index_bg_lambda_4_smg];
    double l5 = ppw->pvecback[pba->index_bg_lambda_5_smg];
    double l6 = ppw->pvecback[pba->index_bg_lambda_6_smg];
    double l7 = ppw->pvecback[pba->index_bg_lambda_7_smg];
    double l8 = ppw->pvecback[pba->index_bg_lambda_8_smg];
    double cs2num = ppw->pvecback[pba->index_bg_cs2num_smg];
    double Dd = ppw->pvecback[pba->index_bg_kinetic_D_smg];
    double M2 = ppw->pvecback[pba->index_bg_M2_smg];
    double DelM2 = ppw->pvecback[pba->index_bg_delta_M2_smg];//M2-1

    double amplitude;

    coeff_isocurv_smg = ppr->entropy_ini*fraccdm*om;

    class_call(calc_extfld_ampl(nexpo,  kin, bra, dbra, run, ten, DelM2, Omx, wx,
                    l1, l2, l3, l4, l5, l6,l7,l8, cs2num, Dd, ppr->pert_ic_regulator_smg,
                   &amplitude),
             ppt->error_message,ppt->error_message);
    amplitude *=2; //calc_extfld_ampl assumes h normalised to 1/2


    ppw->pv->y[ppw->pv->index_pt_x_smg]  = amplitude*coeff_isocurv_smg*pow(tau,nexpo+1);
    ppw->pv->y[ppw->pv->index_pt_x_prime_smg] = (nexpo+1)*a*ppw->pvecback[pba->index_bg_H]*ppw->pv->y[ppw->pv->index_pt_x_smg];

    if(ppt->perturbations_verbose > 5)
    {
     printf("Mode k=%e: CDI mode ext_field_attr IC for smg: ",k);
     printf(" x_smg = %e, x_smg'= %e \n",ppw->pv->y[ppw->pv->index_pt_x_smg],ppw->pv->y[ppw->pv->index_pt_x_prime_smg]);
    }
  }

  return _SUCCESS_;
}

int perturb_isocurvature_b_ic_smg(
    struct precision * ppr,
    struct background * pba,
    struct perturbs * ppt,
    struct perturb_workspace * ppw,
    double tau,
    double k,
    double fracb,
    double om
  ) {

  int qs_array_smg[] = _VALUES_QS_SMG_FLAGS_;

  if((ppt->pert_initial_conditions_smg==ext_field_attr)&&(qs_array_smg[ppw->approx[ppw->index_ap_qs_smg]] == 0))
  //only set ICs for smg if have smg, we are in exernal field attractor and we are *not* quasi-static
  {
    double coeff_isocurv_smg;

    int nexpo= 1;

    double a = ppw->pvecback[pba->index_bg_a];
    double H = ppw->pvecback[pba->index_bg_H];
    double rho_smg = ppw->pvecback[pba->index_bg_rho_smg];
    double p_smg = ppw->pvecback[pba->index_bg_p_smg];

    double wx = p_smg/rho_smg;
    double Omx = rho_smg/pow(H,2);
    double kin = ppw->pvecback[pba->index_bg_kineticity_smg];
    double bra = ppw->pvecback[pba->index_bg_braiding_smg];
    double bra_p = ppw->pvecback[pba->index_bg_braiding_prime_smg];
    double dbra= bra_p/(a*H) ; //Read in log(a) diff of braiding
    double run = ppw->pvecback[pba->index_bg_mpl_running_smg];
    double ten = ppw->pvecback[pba->index_bg_tensor_excess_smg];
    double l1 = ppw->pvecback[pba->index_bg_lambda_1_smg];
    double l2 = ppw->pvecback[pba->index_bg_lambda_2_smg];
    double l3 = ppw->pvecback[pba->index_bg_lambda_3_smg];
    double l4 = ppw->pvecback[pba->index_bg_lambda_4_smg];
    double l5 = ppw->pvecback[pba->index_bg_lambda_5_smg];
    double l6 = ppw->pvecback[pba->index_bg_lambda_6_smg];
    double l7 = ppw->pvecback[pba->index_bg_lambda_7_smg];
    double l8 = ppw->pvecback[pba->index_bg_lambda_8_smg];
    double cs2num = ppw->pvecback[pba->index_bg_cs2num_smg];
    double Dd = ppw->pvecback[pba->index_bg_kinetic_D_smg];
    double M2 = ppw->pvecback[pba->index_bg_M2_smg];
    double DelM2 = ppw->pvecback[pba->index_bg_delta_M2_smg];//M2-1

    double amplitude;

    coeff_isocurv_smg = ppr->entropy_ini*fracb*om;

    class_call(calc_extfld_ampl(nexpo,  kin, bra, dbra, run, ten, DelM2, Omx, wx,
                     l1, l2, l3, l4, l5, l6,l7,l8, cs2num, Dd, ppr->pert_ic_regulator_smg,
                    &amplitude),
              ppt->error_message,ppt->error_message);
    amplitude *=2;  //calc_extfld_ampl assumes h normalised to 1/2.

    ppw->pv->y[ppw->pv->index_pt_x_smg]  = amplitude*coeff_isocurv_smg*pow(tau,nexpo+1);
    ppw->pv->y[ppw->pv->index_pt_x_prime_smg] = (nexpo+1)*a*ppw->pvecback[pba->index_bg_H]*ppw->pv->y[ppw->pv->index_pt_x_smg];



    if(ppt->perturbations_verbose > 5)
    {
      printf("Mode k=%e: BI mode ext_field_attr IC for smg: ",k);
      printf(" x_smg = %e, x_smg'= %e \n",ppw->pv->y[ppw->pv->index_pt_x_smg],ppw->pv->y[ppw->pv->index_pt_x_prime_smg]);
    }
  }

  return _SUCCESS_;
}

int perturb_isocurvature_urd_ic_smg(
    struct precision * ppr,
    struct background * pba,
    struct perturbs * ppt,
    struct perturb_workspace * ppw,
    double tau,
    double k,
    double fracnu,
    double fracg,
    double fracb,
    double om
  ) {

  int qs_array_smg[] = _VALUES_QS_SMG_FLAGS_;

  if((ppt->pert_initial_conditions_smg==ext_field_attr)&&(qs_array_smg[ppw->approx[ppw->index_ap_qs_smg]] == 0))
  //only set ICs for smg if have smg, we are in exernal field attractor and we are *not* quasi-static
  {

    double coeff_isocurv_smg;

    double a = ppw->pvecback[pba->index_bg_a];
    double H = ppw->pvecback[pba->index_bg_H];
    double rho_smg = ppw->pvecback[pba->index_bg_rho_smg];
    double p_smg = ppw->pvecback[pba->index_bg_p_smg];

    double wx = p_smg/rho_smg;
    double Omx = rho_smg/pow(H,2);
    double kin = ppw->pvecback[pba->index_bg_kineticity_smg];
    double bra = ppw->pvecback[pba->index_bg_braiding_smg];
    double bra_p = ppw->pvecback[pba->index_bg_braiding_prime_smg];
    double dbra= bra_p/(a*H) ; //Read in log(a) diff of braiding
    double run = ppw->pvecback[pba->index_bg_mpl_running_smg];
    double ten = ppw->pvecback[pba->index_bg_tensor_excess_smg];
    double l1 = ppw->pvecback[pba->index_bg_lambda_1_smg];
    double l2 = ppw->pvecback[pba->index_bg_lambda_2_smg];
    double l3 = ppw->pvecback[pba->index_bg_lambda_3_smg];
    double l4 = ppw->pvecback[pba->index_bg_lambda_4_smg];
    double l5 = ppw->pvecback[pba->index_bg_lambda_5_smg];
    double l6 = ppw->pvecback[pba->index_bg_lambda_6_smg];
    double l7 = ppw->pvecback[pba->index_bg_lambda_7_smg];
    double l8 = ppw->pvecback[pba->index_bg_lambda_8_smg];
    double cs2num = ppw->pvecback[pba->index_bg_cs2num_smg];
    double Dd = ppw->pvecback[pba->index_bg_kinetic_D_smg];
    double M2 = ppw->pvecback[pba->index_bg_M2_smg];
    double DelM2 = ppw->pvecback[pba->index_bg_delta_M2_smg];//M2-1

    double amplitude;

    // Dominant higher-order correction to BMT99 in the limit fracb*om*tau<<(k*tau)^2:
    // h = -fracnu/(36*(15+4*fracnu)) * (k*tau)^4

    int nexpo= 3;

    coeff_isocurv_smg = ppr->entropy_ini * fracb*fracnu/fracg/10.*k*k * om/4;

    class_call(calc_extfld_ampl(nexpo,  kin, bra, dbra, run, ten, DelM2, Omx, wx,
                     l1, l2, l3, l4, l5, l6,l7,l8, cs2num, Dd, ppr->pert_ic_regulator_smg,
                    &amplitude),
              ppt->error_message,ppt->error_message);
    amplitude *=2; //calc_extfld_ampl assumes h normalised to 1/2

    ppw->pv->y[ppw->pv->index_pt_x_smg]  = amplitude*coeff_isocurv_smg*pow(tau,nexpo+1);
    ppw->pv->y[ppw->pv->index_pt_x_prime_smg] = (nexpo+1)*a*ppw->pvecback[pba->index_bg_H]*ppw->pv->y[ppw->pv->index_pt_x_smg];

    nexpo=4; //next-order term (tau^4) similar in size for h as tau^3 if start late

    coeff_isocurv_smg = ppr->entropy_ini * k*k*fracnu/1152.*
                        (-32.*k*k/(15.+4.*fracnu)- 9.*fracb*(fracb+fracg)*om*om/fracg/fracg);

    class_call(calc_extfld_ampl(nexpo,  kin, bra, dbra, run, ten, DelM2, Omx, wx,
                     l1, l2, l3, l4, l5, l6,l7,l8, cs2num, Dd, ppr->pert_ic_regulator_smg,
                    &amplitude),
              ppt->error_message,ppt->error_message);
    amplitude *=2; //calc_extfld_ampl assumes h normalised to 1/2

    ppw->pv->y[ppw->pv->index_pt_x_smg]  += amplitude*coeff_isocurv_smg*pow(tau,nexpo+1);
    ppw->pv->y[ppw->pv->index_pt_x_prime_smg] += (nexpo+1)*a*ppw->pvecback[pba->index_bg_H]*ppw->pv->y[ppw->pv->index_pt_x_smg];


    if(ppt->perturbations_verbose > 5)
    {
      printf("Mode k=%e: NID mode ext_field_attr IC for smg: ",k);
      printf(" x_smg = %e, x_smg'= %e \n", ppw->pv->y[ppw->pv->index_pt_x_smg],ppw->pv->y[ppw->pv->index_pt_x_prime_smg]);
    }
  }

  return _SUCCESS_;
}

int perturb_isocurvature_urv_ic_smg(
    struct precision * ppr,
    struct background * pba,
    struct perturbs * ppt,
    struct perturb_workspace * ppw,
    double tau,
    double k,
    double fracnu,
    double fracg,
    double fracb,
    double om
  ) {

  int qs_array_smg[] = _VALUES_QS_SMG_FLAGS_;

  if((pba->has_smg == _TRUE_)&&(ppt->pert_initial_conditions_smg==ext_field_attr)&&(qs_array_smg[ppw->approx[ppw->index_ap_qs_smg]] == 0))
  //only set ICs for smg if have smg, we are in exernal field attractor and we are *not* quasi-static
  {

    double coeff_isocurv_smg;

    double a = ppw->pvecback[pba->index_bg_a];
    double H = ppw->pvecback[pba->index_bg_H];
    double rho_smg = ppw->pvecback[pba->index_bg_rho_smg];
    double p_smg = ppw->pvecback[pba->index_bg_p_smg];

    double wx = p_smg/rho_smg;
    double Omx = rho_smg/pow(H,2);
    double kin = ppw->pvecback[pba->index_bg_kineticity_smg];
    double bra = ppw->pvecback[pba->index_bg_braiding_smg];
    double bra_p = ppw->pvecback[pba->index_bg_braiding_prime_smg];
    double dbra= bra_p/(a*H) ; //Read in log(a) diff of braiding
    double run = ppw->pvecback[pba->index_bg_mpl_running_smg];
    double ten = ppw->pvecback[pba->index_bg_tensor_excess_smg];
    double l1 = ppw->pvecback[pba->index_bg_lambda_1_smg];
    double l2 = ppw->pvecback[pba->index_bg_lambda_2_smg];
    double l3 = ppw->pvecback[pba->index_bg_lambda_3_smg];
    double l4 = ppw->pvecback[pba->index_bg_lambda_4_smg];
    double l5 = ppw->pvecback[pba->index_bg_lambda_5_smg];
    double l6 = ppw->pvecback[pba->index_bg_lambda_6_smg];
    double l7 = ppw->pvecback[pba->index_bg_lambda_7_smg];
    double l8 = ppw->pvecback[pba->index_bg_lambda_8_smg];
    double cs2num = ppw->pvecback[pba->index_bg_cs2num_smg];
    double Dd = ppw->pvecback[pba->index_bg_kinetic_D_smg];
    double M2 = ppw->pvecback[pba->index_bg_M2_smg];
    double DelM2 = ppw->pvecback[pba->index_bg_delta_M2_smg];//M2-1

    double amplitude;

    int nexpo=2;

    coeff_isocurv_smg = ppr->entropy_ini * 9./32. *k*om*fracnu*fracb/fracg;

    class_call(calc_extfld_ampl(nexpo,  kin, bra, dbra, run, ten, DelM2, Omx, wx,
                     l1, l2, l3, l4, l5, l6,l7,l8, cs2num, Dd, ppr->pert_ic_regulator_smg,
                    &amplitude),
              ppt->error_message,ppt->error_message);
    amplitude *=2; //calc_extfld_ampl assumes h normalised to 1/2

    ppw->pv->y[ppw->pv->index_pt_x_smg]  = amplitude*coeff_isocurv_smg*pow(tau,nexpo+1);
    ppw->pv->y[ppw->pv->index_pt_x_prime_smg] = (nexpo+1)*a*ppw->pvecback[pba->index_bg_H]*ppw->pv->y[ppw->pv->index_pt_x_smg];

    nexpo=3; //next-order term (tau^3) similar in size for h as tau^2 if start late

    coeff_isocurv_smg = ppr->entropy_ini * fracnu *
                        ( -3.*om*om*k/160.*fracb*(3*fracb+5*fracg)/fracg/fracg -4*k*k*k/15./(5.+4.*fracnu) );

    class_call(calc_extfld_ampl(nexpo,  kin, bra, dbra, run, ten, DelM2, Omx, wx,
                     l1, l2, l3, l4, l5, l6,l7,l8, cs2num, Dd, ppr->pert_ic_regulator_smg,
                    &amplitude),
              ppt->error_message,ppt->error_message);
    amplitude *=2; //calc_extfld_ampl assumes h normalised to 1/2

    ppw->pv->y[ppw->pv->index_pt_x_smg]  += amplitude*coeff_isocurv_smg*pow(tau,nexpo+1);
    ppw->pv->y[ppw->pv->index_pt_x_prime_smg] += (nexpo+1)*a*ppw->pvecback[pba->index_bg_H]*ppw->pv->y[ppw->pv->index_pt_x_smg];

    if(ppt->perturbations_verbose > 5)
    {
      printf("Mode k=%e: NIV mode ext_field_attr IC for smg: ",k);
      printf(" x_smg = %e, x_smg'= %e \n",ppw->pv->y[ppw->pv->index_pt_x_smg],ppw->pv->y[ppw->pv->index_pt_x_prime_smg]);
    }
  }

  return _SUCCESS_;
}

int perturb_get_x_x_prime_newtonian(
 struct perturb_workspace * ppw
) {

  int qs_array_smg[] = _VALUES_QS_SMG_FLAGS_;

  /* scalar field: TODO: add gauge transformations (when we add Newtonian gauge) */
  if (qs_array_smg[ppw->approx[ppw->index_ap_qs_smg]] == 0) {
    ppw->pv->y[ppw->pv->index_pt_x_smg] += 0.;
    ppw->pv->y[ppw->pv->index_pt_x_prime_smg] += 0.;
  }

  return _SUCCESS_;
}

int perturb_get_h_prime_ic_from_00(
  struct background * pba,
  struct perturb_workspace * ppw,
  double k,
  double eta,
  double delta_rho_tot
    ) {

  double a = ppw->pvecback[pba->index_bg_a];
  double H = ppw->pvecback[pba->index_bg_H];
  double M2 = ppw->pvecback[pba->index_bg_M2_smg];
  double DelM2 = ppw->pvecback[pba->index_bg_delta_M2_smg];//M2-1
  double rho_tot = ppw->pvecback[pba->index_bg_rho_tot_wo_smg];
  double p_tot = ppw->pvecback[pba->index_bg_p_tot_wo_smg];
  double rho_smg = ppw->pvecback[pba->index_bg_rho_smg];
  double p_smg = ppw->pvecback[pba->index_bg_p_smg];
  double kin = ppw->pvecback[pba->index_bg_kineticity_smg];
  double bra = ppw->pvecback[pba->index_bg_braiding_smg];
  /* TODO_EB: rewrite this equation with new variables */
  ppw->pv->y[ppw->pv->index_pt_h_prime_from_trace] = (-4. * pow(H, -1) * pow(k, 2) * eta / a - 6. * pow(H, -1) * pow(M2, -1) * delta_rho_tot * a + 2. * H * (3. * bra + kin) * ppw->pv->y[ppw->pv->index_pt_x_prime_smg] * a + (2. * bra * pow(k, 2) + (-18. + 15. * bra + 2. * kin) * rho_smg * pow(a, 2) + (-18. * DelM2 + 15. * bra * M2 + 2. * kin * M2) * rho_tot * pow(M2, -1) * pow(a, 2) + (-2. * DelM2 + bra * M2) * 9. * pow(M2, -1) * p_tot * pow(a, 2) + 9. * (-2. + bra) * p_smg * pow(a, 2)) * ppw->pv->y[ppw->pv->index_pt_x_smg]) * pow(-2. + bra, -1);

  return _SUCCESS_;
}
