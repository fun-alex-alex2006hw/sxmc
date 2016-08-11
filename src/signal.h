#ifndef __SIGNAL_H__
#define __SIGNAL_H__

/**
 * \file signal.h
 *
 * Fit signal.
*/

#include <string>
#include <vector>

#include <sxmc/observable.h>
#include <sxmc/systematic.h>
#include <sxmc/source.h>

namespace pdfz {
  class Eval;
}

/**
 * \class Signal
 *
 * A container for signal metadata and PDFs
*/
class Signal {
public:
  /** Constructor */
  Signal() {}

  /**
   * Construct a Signal from a list of ROOT files.
   *
   * \param _name - A string identifier
   * \param _title - A title for plotting (ROOT LaTeX)
   * \param _filename - Path to the ROOT file with the PDF data
   * \param _dataset - Dataset ID for this signal
   * \param _source - Source (for correlated rates)
   * \param _nexpected - The expected number of events
   * \param sample_fields - The names of the fields for the data samples
   * \param observables - A list of observables used in the fit
   * \param cuts - A list of observables to be applied as cuts
   * \param systematics - A list of systematics to be used in the fit
  */
  Signal(std::string _name, std::string _title,
         std::string _filename, unsigned _dataset, Source _source,
         double _nexpected,
         std::vector<std::string>& sample_fields,
         std::vector<Observable>& observables,
         std::vector<Observable>& cuts,
         std::vector<Systematic>& systematics);

  /**
   * Get the efficiency (N in PDF / N MC) for given systematics.
   *
   * \param systematics - The list of systematics
   * \returns The efficiency N_pdf / N_mc
  */
  double get_efficiency(std::vector<Systematic>& systematics);

  /** Print the signal configuration. */
  void print() const;

  std::string name;  //!< String identifier
  std::string title;  //!< Histogram title in ROOT-LaTeX format
  std::string filename;  //!< Filename from which data is loaded
  unsigned dataset;  //!< Dataset identifier (cf. multi-phase fitting)
  Source source;  //!< Source for correlated rates
  double nexpected;  //!< Total (uncut) events expected in this fit
  size_t n_mc;  //!< Number of simulated events used to make pdf
  pdfz::Eval* histogram;  //!< PDF
  std::vector<std::string> systematic_names;  //!< Names of active systematics

protected:
  /**
   * Construct the pdfz histogram object.
   *
   * \param samples - The vector of data samples for the PDF
   * \param nfields - The number of fields (columns) in the sample array
   * \param observables - A list of observables used in the fit
   * \param systematics - A list of systematics to be used in the fit
   */
  void build_pdfz(std::vector<float>& samples, int nfields,
                  std::vector<Observable>& observables,
                  std::vector<Systematic>& systematics);

public:
  /**
   * Copy the requested sample fields from the fields of the same name
   * in the dataset array.
   *
   * \param samples - The vector of data samples for the PDF (by reference)
   * \param dataset - The dataset to load in, as a float array
   * \param dataset_id - An ID to tag phase, appended to the last column
   * \param sample_fields - The names of the fields for the samples array
   * \param dataset_fields - The names of the fields in the dataset array
   * \param cuts - A list of observables to be applied as cuts
   */
  static void read_dataset_to_samples(std::vector<float>& samples,
                                      std::vector<float>& dataset,
                                      unsigned dataset_id,
                                      std::vector<std::string>& sample_fields,
                                      std::vector<std::string>& dataset_fields,
                                      std::vector<Observable>& cuts);
};

#endif  // __SIGNAL_H__

