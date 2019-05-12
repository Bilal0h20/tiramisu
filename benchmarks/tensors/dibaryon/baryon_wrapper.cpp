#include "Halide.h"
#include <tiramisu/utils.h>
#include <cstdlib>
#include <iostream>
#include "benchmarks.h"

#include "baryon_wrapper.h"
#include "baryon_ref.cpp"

int main(int, char **)
{
    std::vector<std::chrono::duration<double,std::milli>> duration_vector_1;
    std::vector<std::chrono::duration<double,std::milli>> duration_vector_2;

    std::complex<double> Blocal[Nsrc][Nc][Ns][Nc][Ns][Nc][Ns][Vsnk][Lt];
    std::complex<double> Bsingle[Nsrc][Nc][Ns][Nc][Ns][Nc][Ns][Vsnk][Vsnk][Lt];
    std::complex<double> Bdouble[Nsrc][Nc][Ns][Nc][Ns][Nc][Ns][Vsnk][Vsnk][Lt];
    std::complex<double> prop[Nq][Nc][Ns][Nc][Ns][Vsnk][Lt][Vsrc];
    std::complex<double> O[Nsrc][Nc][Ns][Nc][Ns][Nc][Ns][Vsnk][Lt][Vsrc];
    std::complex<double> P[Nsrc][Nc][Ns][Nc][Ns][Nc][Ns][Vsnk][Lt][Vsrc];
    int color_weights[Nw][Nq];
    int spin_weights[Nw][Nq];
    double weights[Nw];
    std::complex<double> psi[Nsrc][Vsrc];

    // Blocal
    // Blocal_r: tiramisu real part of Blocal.
    // Blocal_i: tiramisu imaginary part of Blocal.
    Halide::Buffer<double> Blocal_r(Lt, Vsnk, Ns, Nc, Ns, Nc, Ns, Nc, Nsrc, "Blocal_r");
    Halide::Buffer<double> Blocal_i(Lt, Vsnk, Ns, Nc, Ns, Nc, Ns, Nc, Nsrc, "Blocal_i");

    // prop
    Halide::Buffer<double> prop_r(Vsrc, Lt, Vsnk, Ns, Nc, Ns, Nc, Nq, "prop_r");
    Halide::Buffer<double> prop_i(Vsrc, Lt, Vsnk, Ns, Nc, Ns, Nc, Nq, "prop_i");

    // psi
    Halide::Buffer<double> psi_r(Vsrc, Nsrc, "psi_r");
    Halide::Buffer<double> psi_i(Vsrc, Nsrc, "psi_i");

    Halide::Buffer<int> color_weights_t(Nq, Nw, "color_weights_t");
    Halide::Buffer<int> spin_weights_t(Nq, Nw, "spin_weights_t");
    Halide::Buffer<double> weights_t(Nw, "weights_t");

    Halide::Buffer<double> Bsingle_r(Lt, Vsrc, Vsnk, Ns, Nc, Ns, Nc, Ns, Nc, Nsrc, "Bsingle_r");
    Halide::Buffer<double> Bsingle_i(Lt, Vsnk, Vsnk, Ns, Nc, Ns, Nc, Ns, Nc, Nsrc, "Bsingle_i");

    Halide::Buffer<double> O_r(Vsrc, Lt, Vsnk, Ns, Nc, Ns, Nc, Ns, Nc, Nsrc, "O_r");
    Halide::Buffer<double> O_i(Vsrc, Lt, Vsnk, Ns, Nc, Ns, Nc, Ns, Nc, Nsrc, "O_i");
    Halide::Buffer<double> P_r(Vsrc, Lt, Vsnk, Ns, Nc, Ns, Nc, Ns, Nc, Nsrc, "P_r");
    Halide::Buffer<double> P_i(Vsrc, Lt, Vsnk, Ns, Nc, Ns, Nc, Ns, Nc, Nsrc, "P_i");

    Halide::Buffer<double> Bdouble_r(Lt, Vsrc, Vsnk, Ns, Nc, Ns, Nc, Ns, Nc, Nsrc, "Bdouble_r");
    Halide::Buffer<double> Bdouble_i(Lt, Vsnk, Vsnk, Ns, Nc, Ns, Nc, Ns, Nc, Nsrc, "Bdouble_i");

    // Initialization
   for (int wnum=0; wnum<Nw; wnum++)
   {
       double v = rand()%10;
       weights[wnum] = v;
       weights_t(wnum) = v;
   }

   for (int n=0; n<Nsrc; n++)
     for (int y=0; y<Vsrc; y++)
     {
	double v1 = rand()%10;
	double v2 = rand()%10;
	std::complex<double> c(v1, v2);
	psi[n][y] = c;
	psi_r(y, n) = v1;
	psi_i(y, n) = v2;
     }

   for (int tri=0; tri<Nq; tri++)
       for (int iCprime=0; iCprime<Nc; iCprime++)
	  for (int iSprime=0; iSprime<Ns; iSprime++)
	     for (int jCprime=0; jCprime<Nc; jCprime++)
		for (int jSprime=0; jSprime<Ns; jSprime++)
                   for (int x=0; x<Vsnk; x++)
                      for (int t=0; t<Lt; t++)
		        for (int y=0; y<Vsrc; y++)
			{
			    double v1 = rand()%10;
			    double v2 = rand()%10;
			    std::complex<double> c(v1, v2);
			    prop[tri][iCprime][iSprime][jCprime][jSprime][x][t][y] = c;
			    prop_r(y, t, x, jSprime, jCprime, iSprime, iCprime, tri) = v1;
			    prop_i(y, t, x, jSprime, jCprime, iSprime, iCprime, tri) = v2;
 		        }

   for (int wnum=0; wnum<Nw; wnum++)
	for (int tri=0; tri<Nq; tri++)
	{
		color_weights[wnum][tri] = 0; // tri
		color_weights_t(tri, wnum) = 0; //tri
		spin_weights[wnum][tri] = 0; //tri
		spin_weights_t(tri, wnum) = 0; //tri
	}

    for (int i = 0; i < NB_TESTS; i++)
    {
	    auto start2 = std::chrono::high_resolution_clock::now();

	    make_local_block(Blocal, prop, color_weights, spin_weights, weights, psi);
	    make_single_block(Bsingle, prop, color_weights, spin_weights, weights, psi);
	    make_double_block(Bdouble, prop, color_weights, spin_weights, weights, psi, O, P);

	    auto end2 = std::chrono::high_resolution_clock::now();
	    std::chrono::duration<double,std::milli> duration2 = end2 - start2;
	    duration_vector_2.push_back(duration2);
    }

    for (int i = 0; i < NB_TESTS; i++)
    {
	    auto start1 = std::chrono::high_resolution_clock::now();

	    tiramisu_generated_code(Blocal_r.raw_buffer(),
				    Blocal_i.raw_buffer(),
				    prop_r.raw_buffer(),
				    prop_i.raw_buffer(),
				    weights_t.raw_buffer(),
				    psi_r.raw_buffer(),
				    psi_i.raw_buffer(),
				    color_weights_t.raw_buffer(),
				    spin_weights_t.raw_buffer(),
				    Bsingle_r.raw_buffer(),
				    Bsingle_i.raw_buffer(),
				    Bdouble_r.raw_buffer(),
				    Bdouble_i.raw_buffer(),
				    O_r.raw_buffer(),
				    O_i.raw_buffer(),
				    P_r.raw_buffer(),
				    P_i.raw_buffer());

	    auto end1 = std::chrono::high_resolution_clock::now();
	    std::chrono::duration<double,std::milli> duration1 = end1 - start1;
	    duration_vector_1.push_back(duration1);
    }

    print_time("performance_CPU.csv", "dibaryon", {"Ref", "Tiramisu"}, {median(duration_vector_2), median(duration_vector_1)});
    std::cout << "\nSpeedup = " << median(duration_vector_2)/median(duration_vector_1) << std::endl;

    // Compare outputs.
    for (int n=0; n<Nsrc; n++)
      for (int iCprime=0; iCprime<Nc; iCprime++)
        for (int iSprime=0; iSprime<Ns; iSprime++)
           for (int jCprime=0; jCprime<Nc; jCprime++)
              for (int jSprime=0; jSprime<Ns; jSprime++)
                 for (int kCprime=0; kCprime<Nc; kCprime++)
                    for (int kSprime=0; kSprime<Ns; kSprime++)
                       for (int x=0; x<Vsnk; x++)
                          for (int t=0; t<Lt; t++)
                              if (std::abs(Blocal[n][iCprime][iSprime][jCprime][jSprime][kCprime][kSprime][x][t].real() -
						Blocal_r(t, x, kSprime, kCprime, jSprime, jCprime, iSprime, iCprime, n)) >= 0.01)
			      {
				  std::cout << "Error: different computed values for Blocal! Ref = " << Blocal[n][iCprime][iSprime][jCprime][jSprime][kCprime][kSprime][x][t].real() << " - Tiramisu = " << Blocal_r(t, x, kSprime, kCprime, jSprime, jCprime, iSprime, iCprime, n) << std::endl;
				  exit(1);
			      }

    for (int n=0; n<Nsrc; n++)
      for (int iCprime=0; iCprime<Nc; iCprime++)
        for (int iSprime=0; iSprime<Ns; iSprime++)
           for (int jCprime=0; jCprime<Nc; jCprime++)
              for (int jSprime=0; jSprime<Ns; jSprime++)
                 for (int kCprime=0; kCprime<Nc; kCprime++)
                    for (int kSprime=0; kSprime<Ns; kSprime++)
                       for (int x=0; x<Vsnk; x++)
		         for (int x2=0; x2<Vsnk; x2++)
			     for (int t=0; t<Lt; t++)
                             if (std::abs(Bsingle[n][iCprime][iSprime][jCprime][jSprime][kCprime][kSprime][x][x2][t].real() -
					 Bsingle_r(t, x2, x, kSprime, kCprime, jSprime, jCprime, iSprime, iCprime, n)) >= 0.01)
			      {
				  std::cout << "Error: different computed values for Bsingle! Ref = " << Bsingle[n][iCprime][iSprime][jCprime][jSprime][kCprime][kSprime][x][x2][t].real() << " - Tiramisu = " << Bsingle_r(t, x2, x, kSprime, kCprime, jSprime, jCprime, iSprime, iCprime, n) << std::endl;
				std::cout << "Position: (" << n << ", " << iCprime << ", " << iSprime << ", " << jCprime << ", " << jSprime << ", " << kCprime << ", " << kSprime << ", " << x << ", " << x2 << ", " << t << ")" << std::endl;
				  exit(1);
			      }

    for (int jCprime=0; jCprime<Nc; jCprime++)
      for (int jSprime=0; jSprime<Ns; jSprime++)
         for (int kCprime=0; kCprime<Nc; kCprime++)
            for (int kSprime=0; kSprime<Ns; kSprime++)
               for (int jC=0; jC<Nc; jC++)
                  for (int jS=0; jS<Ns; jS++)
                     for (int x=0; x<Vsnk; x++)
                        for (int t=0; t<Lt; t++)
                           for (int y=0; y<Vsrc; y++)
                              for (int n=0; n<Nsrc; n++)
				      if (std::abs(O[n][jCprime][jSprime][kCprime][kSprime][jC][jS][x][t][y].real() -
							O_r(y, t, x, jS, jC, kSprime, kCprime, jSprime, jCprime, n)) >= 0.01)
				      {
				  		std::cout << "Error: different computed values for O! Ref = " << O[n][jCprime][jSprime][kCprime][kSprime][jC][jS][x][t][y].real()
							<< " - Tiramisu = " << O_r(y, t, x, jS, jC, kSprime, kCprime, jSprime, jCprime, n) << std::endl;
				  		exit(1);
				      }

    for (int jCprime=0; jCprime<Nc; jCprime++)
      for (int jSprime=0; jSprime<Ns; jSprime++)
         for (int kCprime=0; kCprime<Nc; kCprime++)
            for (int kSprime=0; kSprime<Ns; kSprime++)
               for (int jC=0; jC<Nc; jC++)
                  for (int jS=0; jS<Ns; jS++)
                     for (int x=0; x<Vsnk; x++)
                        for (int t=0; t<Lt; t++)
                           for (int y=0; y<Vsrc; y++)
                              for (int n=0; n<Nsrc; n++)
				      if (std::abs(P[n][jCprime][jSprime][kCprime][kSprime][jC][jS][x][t][y].real() -
							P_r(y, t, x, jS, jC, kSprime, kCprime, jSprime, jCprime, n)) >= 0.01)
				      {
				  		std::cout << "Error: different computed values for P! Ref = " << P[n][jCprime][jSprime][kCprime][kSprime][jC][jS][x][t][y].real()
							<< " - Tiramisu = " << P_r(y, t, x, jS, jC, kSprime, kCprime, jSprime, jCprime, n) << std::endl;
				  		exit(1);
				      }
    for (int n=0; n<Nsrc; n++)
      for (int iCprime=0; iCprime<Nc; iCprime++)
        for (int iSprime=0; iSprime<Ns; iSprime++)
           for (int jCprime=0; jCprime<Nc; jCprime++)
              for (int jSprime=0; jSprime<Ns; jSprime++)
                 for (int kCprime=0; kCprime<Nc; kCprime++)
                    for (int kSprime=0; kSprime<Ns; kSprime++)
                       for (int x=0; x<Vsnk; x++)
		         for (int x2=0; x2<Vsnk; x2++)
			     for (int t=0; t<Lt; t++)
                             if (std::abs(Bdouble[n][iCprime][iSprime][jCprime][jSprime][kCprime][kSprime][x][x2][t].real() -
					 Bdouble_r(t, x2, x, kSprime, kCprime, jSprime, jCprime, iSprime, iCprime, n)) >= 0.01)
			      {
				  std::cout << "Error: different computed values for Bdouble! Ref = " << Bdouble[n][iCprime][iSprime][jCprime][jSprime][kCprime][kSprime][x][x2][t].real() << " - Tiramisu = " << Bdouble_r(t, x2, x, kSprime, kCprime, jSprime, jCprime, iSprime, iCprime, n) << std::endl;
				  std::cout << "Position: (" << n << ", " << iCprime << ", " << iSprime << ", " << jCprime << ", " << jSprime << ", " << kCprime << ", " << kSprime << ", " << x << ", " << x2 << ", " << t << ")" << std::endl;
				  exit(1);
			      }

    std::cout << "\n\n\033[1;32mSuccess: computed values are equal!\033[0m\n\n" << std::endl;

    return 0;
}