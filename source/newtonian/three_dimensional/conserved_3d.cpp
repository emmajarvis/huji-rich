#include "conserved_3d.hpp"

using std::size_t;

Conserved3D::Conserved3D(void):
  mass(0), momentum(), energy(0), tracers() {}

Conserved3D::Conserved3D(double mass_i,
			 const Vector3D& momentum_i,
			 double energy_i):
  mass(mass_i), momentum(momentum_i), energy(energy_i), tracers() {}

Conserved3D::Conserved3D(double mass_i,
			 const Vector3D& momentum_i,
			 double energy_i,
			 const vector<double>& tracers_i):
  mass(mass_i), momentum(momentum_i), 
  energy(energy_i), tracers(tracers_i) {}

namespace {
  vector<double> operator*(double s, const vector<double>& v)
  {
    vector<double> res(v.size());
    for(size_t i=0;i<v.size();++i)
      res[i] = s*v[i];
    return res;
  }

  vector<double> operator/(const vector<double>& v,double s)
  {
    vector<double> res(v.size());
    for(size_t i=0;i<v.size();++i)
      res[i] = v[i]/s;
    return res;
  }
}

Conserved3D& Conserved3D::operator-=(const Conserved3D& diff)
{
  mass -= diff.mass;
  momentum -= diff.momentum;
  energy -= diff.energy;
  for(size_t i=0;i<tracers.size();++i)
    tracers[i] -= diff.tracers[i];
  return *this;
}

Conserved3D& Conserved3D::operator+=(const Conserved3D& diff)
{
  mass += diff.mass;
  momentum += diff.momentum;
  energy += diff.energy;
  for(size_t i=0;i<tracers.size();++i)
    tracers[i] += diff.tracers[i];
  return *this;
}

#ifdef RICH_MPI
size_t Conserved3D::getChunkSize(void) const
{
	return 5 + tracers.size();
}

vector<double> Conserved3D::serialize(void) const
{
	vector<double> res(getChunkSize());
	res.at(0) = mass;
	res.at(1) = energy;
	res.at(2) = momentum.x;
	res.at(3) = momentum.y;
	res.at(4) = momentum.z;
	size_t counter = 5;
	size_t N = tracers.size();
	for (size_t j = 0; j < N; ++j)
		res[j + counter] = tracers[j];
	return res;
}

void Conserved3D::unserialize(const vector<double>& data)
{
	assert(data.size() == getChunkSize());
	mass = data.at(0);
	energy = data.at(1);
	momentum.x = data.at(2);
	momentum.y = data.at(3);
	momentum.z = data.at(4);
	size_t counter = 5;
	size_t N = tracers.size();
	for (size_t j = 0; j < N; ++j)
		tracers[j] = data.at(counter + j);
}
#endif

Conserved3D operator*(double s, const Conserved3D& c)
{
  return Conserved3D(s*c.mass,
		     s*c.momentum,
		     s*c.energy,
		     s*c.tracers);
}

Conserved3D operator*(const Conserved3D& c,double s)
{
	return Conserved3D(s*c.mass,
		s*c.momentum,
		s*c.energy,
		s*c.tracers);
}


Conserved3D operator/(const Conserved3D& c, double s)
{
  return Conserved3D(c.mass/s,
		     c.momentum/s,
		     c.energy/s,
		     c.tracers/s);
}

void PrimitiveToConserved(ComputationalCell3D const& cell, double vol, Conserved3D &res,EquationOfState const& eos,
	TracerStickerNames const& tsn)
{
	res.mass = cell.density*vol;
	res.momentum = cell.velocity*res.mass;
	res.energy = res.mass*(eos.dp2e(cell.density, cell.pressure, cell.tracers, tsn.tracer_names) + 
		0.5*ScalarProd(cell.velocity,cell.velocity));
	size_t N = cell.tracers.size();
	for (size_t i = 0; i < N;++i)
		res.tracers[i] = cell.tracers[i]*res.mass;
}

Conserved3D operator+(Conserved3D const& p1, Conserved3D const& p2)
{
	Conserved3D res(p1);
	res += p2;
	return res;
}

Conserved3D operator-(Conserved3D const& p1, Conserved3D const& p2)
{
	Conserved3D res(p1);
	res -= p2;
	return res;
}
