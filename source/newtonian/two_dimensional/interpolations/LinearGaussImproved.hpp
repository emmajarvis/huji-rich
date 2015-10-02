/*! \file LinearGaussImproved.hpp
\brief Linear interpolation that guarantees compliance with the equation of state and calcualtes the GG gradient from the CM of the cell
\author Elad Steinberg
*/

#ifndef LINEAR_GAUSS_IMPROVED
#define LINEAR_GAUSS_IMPROVED 1

#include "../../common/equation_of_state.hpp"
#include "../spatial_reconstruction.hpp"
#include <cmath>
#include "../../../misc/universal_error.hpp"
#include "../GhostPointGenerator.hpp"

//! \brief Linear gauss interpolation
class LinearGaussImproved : public SpatialReconstruction
{
public:

	/*! \brief Class constructor
	\param eos Equation of state
	\param slf Slope limiter flag
	\param delta_v The GradV*L/Cs ratio needed for slope limiter
	\param theta The theta from tess in slope limiter.
	\param delta_P The pressure ratio for shock detection
	\param ghost The ghost point generator
	*/
	LinearGaussImproved
		(EquationOfState const& eos,
		GhostPointGenerator const& ghost,
		bool slf = true,
		double delta_v = 0.2,
		double theta = 0.5,
		double delta_P = 0.7,
		const vector<string>& flat_tracers =
		vector<string>());

	vector<pair<ComputationalCell, ComputationalCell> > operator() (const Tessellation& tess,
		const vector<ComputationalCell>& cells)const;

	/*! \brief Interpolates a cell
	\param cell The primitives of the cell
	\param cell_index The index of the cell
	\param cm The cell's center of mass
	\param target The location of the interpolation
	\return The interpolated value
	*/

	ComputationalCell Interp(ComputationalCell const& cell, size_t cell_index, Vector2D const& cm, Vector2D const& target)const;

	/*!
	\brief Returns the gradients
	\return The gradients
	*/
	vector<pair<ComputationalCell, ComputationalCell> > GetSlopes(void)const;
private:
	EquationOfState const& eos_;
	GhostPointGenerator const& ghost_;
	mutable vector<pair<ComputationalCell, ComputationalCell> > rslopes_;
	mutable vector<pair<ComputationalCell, ComputationalCell> > naive_rslopes_;
	const bool slf_;
	const double shockratio_;
	const double diffusecoeff_;
	const double pressure_ratio_;
	const vector<string> flat_tracers_;

	LinearGaussImproved
		(const LinearGaussImproved& origin);

	LinearGaussImproved& operator=
		(const LinearGaussImproved& origin);

};
#endif //LINEAR_GAUSS_IMPROVED
