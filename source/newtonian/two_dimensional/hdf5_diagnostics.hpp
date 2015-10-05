/*! \file hdf5_diagnostics.hpp
  \brief Simulation output to hdf5 file format
  \author Elad Steinberg
 */

#ifndef HDF5_DIAG
#define HDF5_DIAG 1

#include <H5Cpp.h>
#include <string>
#include "hdsim2d.hpp"
#include "../../misc/int2str.hpp"
#include "diagnostics.hpp"

class Snapshot
{
public:

  vector<Vector2D> mesh_points;
  vector<ComputationalCell> cells;
  double time;
};

Snapshot read_hdf5_snapshot
(const string& fname);

//! \brief Addition data to be written in a snapshot
class DiagnosticAppendix
{
public:

  /*! \brief Calculates additional data
    \param sim Hydrodynamic simulation
    \return Calculated data
   */
  virtual vector<double> operator()(const hdsim& sim) const = 0;

  /*! \brief Returns the name of the new field
   */
  virtual string getName(void) const = 0;

  //! \brief Class destructor
  virtual ~DiagnosticAppendix(void);
};

/*!
\brief Writes the simulation data into an HDF5 file
\param sim The hdsim class of the simulation
\param fname The name of the output file
\param appendices Additional data to be written to snapshot
*/
void write_snapshot_to_hdf5(hdsim const& sim,string const& fname,
			    const vector<DiagnosticAppendix*>& appendices=vector<DiagnosticAppendix*>());
/*!
\brief Reads an HDF5 snapshot file in order to restart the simulation
\param dump The dump data structure, should be when passed
\param fname The path to the HDF5 file
\param eos The equation of state
*/
void read_hdf5_snapshot(ResetDump &dump,string const& fname,EquationOfState
	const* eos);
/*!
\brief Converts an HDF5 snapshot file to the RICH custom reset binary format
\param input The path to the HDF5 file
\param output The path to the new binary file
*/
void ConvertHDF5toBinary(string const& input, string const& output);

#endif // HDF5_DIAG
