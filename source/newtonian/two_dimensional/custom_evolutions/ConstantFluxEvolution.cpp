#include "ConstantFluxEvolution.hpp"

using std::multiplies;

ConstantFluxEvolution::ConstantFluxEvolution(Primitive const& cell,
	vector<double> const& tracer,EquationOfState const& eos)
:cell_(cell),tracer_(tracer),eos_(eos){}

ConstantFluxEvolution::~ConstantFluxEvolution(void)
{}

Conserved ConstantFluxEvolution::CalcFlux(Tessellation const& tessellation,
	vector<Primitive> const& cells,	double dt,
	SpatialReconstruction& interpolation,Edge const& edge,
	Vector2D const& facevelocity,RiemannSolver const& /*rs*/,int index,
	HydroBoundaryConditions const& bc,double time,
	vector<vector<double> > const& /*tracers*/)
{
	if(bc.IsBoundary(edge,tessellation))
		return bc.CalcFlux(tessellation,cells,facevelocity,edge,interpolation,dt,
		time);
	else
	{
		Vector2D normaldir = tessellation.GetMeshPoint(edge.neighbors.second)-
			tessellation.GetMeshPoint(edge.neighbors.first);
		normaldir=normaldir/abs(normaldir);
		double v=abs(cell_.Velocity);
		Primitive cell(cell_);
		if(edge.neighbors.second==index)
			cell.Velocity=-v*normaldir;
		else
			cell.Velocity=v*normaldir;
		return Primitive2Flux(cell,normaldir);
	}
}

Primitive ConstantFluxEvolution::UpdatePrimitive
	(vector<Conserved> const& /*conservedintensive*/,
	EquationOfState const& /*eos*/,
	vector<Primitive>& cells,int index,Tessellation const& /*tess*/,
	double /*time*/,vector<vector<double> > const& /*tracers*/)
{
	return cell_;
}

vector<double> ConstantFluxEvolution::UpdateTracer(int index,vector<vector<double> >
	const& /*tracers*/,vector<vector<double> > const& /*tracerchange*/,vector<Primitive> const& cells,
	Tessellation const& tess,double /*time*/)
{
	return tess.GetVolume(index)*cell_.Density*tracer_;
}

vector<double> ConstantFluxEvolution::CalcTracerFlux(Tessellation const& /*tess*/,
	vector<Primitive> const& /*cells*/,vector<vector<double> > const& /*tracers*/,
	double dm,Edge const& edge,int /*index*/,double dt,double /*time*/,
	SpatialReconstruction const& /*interp*/,Vector2D const& /*vface*/)
{
	vector<double> res(tracer_);
	transform(res.begin(),res.end(),res.begin(),
		bind1st(multiplies<double>(),abs(dm)*dt*edge.GetLength()));
	return res;
}

