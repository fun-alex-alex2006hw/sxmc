#ifndef __MCMC_H__
#define __MCMC_H__

/**
 * \file mcmc.h
 *
 * Utilities for Markov Chain Monte Carlo distribution sampling.
*/

#include <cmath>
#include <string>
#include <vector>
#include <cuda.h>
#include <hemi/hemi.h>

#include <sxmc/signal.h>
#include <sxmc/observable.h>
#include <sxmc/systematic.h>
#include <sxmc/nll_kernels.h>
#include <sxmc/pdfz.h>

#ifdef __CUDACC__
#include <curand_kernel.h>
#endif

#ifndef __HEMI_ARRAY_H__
#define __HEMI_ARRAY_H__
#include <hemi/array.h>
#endif

class TNtuple;
class LikelihoodSpace;

/**
 * \class MCMC
 * \brief Markov Chain Monte Carlo simulator
 *
 * Given a set of signal PDFs and a dataset, random walk to map out the
 * likelihood space.
 */
class MCMC {
public:
  /**
   * Constructor
   *
   * \param sources List of Sources defining the signal rates
   * \param signals List of Signals defining the PDFs and expectations
   * \param systematics List of systematic parameter definitions
   * \param observables List of observables in the data
   */
  MCMC(const std::vector<Source>& sources,
       const std::vector<Signal>& signals,
       const std::vector<Systematic>& systematics,
       const std::vector<Observable>& observables);

  /**
   * Destructor
   *
   * Free HEMI arrays.
   */
  ~MCMC();

  /**
   * Perform walk.
   *
   * \param data Array of samples representing data to fit
   * \param nsteps Number of random-walk steps to take
   * \param burnin_fraction Fraction of initial steps to throw out
   * \param debug_mode If true, accept and save all steps
   * \param sync_interval How often to copy accepted from GPU to storage
   * \returns LikelihoodSpace built from samples
   */
  LikelihoodSpace* operator()(std::vector<float>& data,
                              unsigned nsteps,
                              float burnin_fraction,
                              const bool debug_mode=false,
                              unsigned sync_interval=10000);

protected:
  /**
   * Evaluate the NLL function
   *
   * -logL = sum(Nj) + 1/2*sum((r-r')^2/s^2) - sum(log(sum(Nj*Pj(xi))))
   *
   * Nothing is returned -- the output stays in the nll array, so it can stay
   * on the device.
   *
   * This is done in three steps, to support running on the GPU:
   *
   *   1. Compute partial sums of chunks of events for the last term
   *   2. Total up partial sums from step 1
   *   3. Add normalization and other constraints with sum from step 2
   *
   * \param lut - PDF value lookup table (nevents x nsignals)
   * \param nevents - Number of events
   * \param v - Parameter vector at which to evaluate
   * \param nll - Container for output NLL value
   * \param nexpected - Number of events expected for each signal
   * \param n_mc - Number of MC events for each signal
   * \param source_id - Index in the Source array for each signal rate
   * \param norms - Normalizations for each signal
   * \param event_partial_sums - Pre-allocated buffer for event term
   *                             calculation
   * \param event_total_sum - Pre-allocated buffer for event term total
   */
  void nll(const float* lut, size_t nevents,
           const double* v, double* nll,
           const double* nexpected,
           const unsigned* n_mc,
           const short* source_id,
           const unsigned* norms,
           double* event_partial_sums,
           double* event_total_sum);

private:
  size_t nsources;  //!< Number of signal sources
  size_t nsignals;  //!< Number of signal parameters
  size_t nsystematics;  //!< Number of systematic parameters
  size_t nparameters;  //!< Total number of parameters
  size_t nobservables;  //!< Number of observables in data
  size_t nfloat;  //!< Number of floating parameters
  bool systematics_fixed;  //!< All systematic parameters are fixed
  unsigned nnllblocks;  //!< Number of cuda blocks for nll partial sums
  unsigned nllblocksize;  //!< Size of cuda blocks for nll partial sums
  unsigned nnllthreads;  //!< Number of threads for nll partial sums
  unsigned nreducethreads; //!< Number of threads to use in partial sum
                           //!< reduction kernel
  std::string varlist;  //!< String identifier list for ntuple indexing
  hemi::Array<double>* parameter_means;  //!< Parameter central values
  hemi::Array<double>* parameter_sigma;  //!< Parameter Gaussian uncertainty
  hemi::Array<double>* nexpected;  //!< Expectation values
  hemi::Array<unsigned>* n_mc;  //!< Number of MC samples
  hemi::Array<short>* source_id;  //!< Source array offsets
  hemi::Array<RNGState>* rngs;  //!< CURAND RNGs, ignored in CPU mode
  std::vector<std::string> parameter_names;  //!< String name of each param
  std::vector<bool> parameter_fixed;  //!< Is this parameter fixed?
  std::vector<pdfz::Eval*> pdfs;  //!< References to signal pdfs
};

#endif  // __MCMC_H__

