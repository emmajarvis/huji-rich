#include "simple_cell_updater.hpp"
#include "../../misc/lazy_list.hpp"

SimpleCellUpdater::SimpleCellUpdater
(const vector<pair<const SimpleCellUpdater::Condition*, const SimpleCellUpdater::Action*> > sequence):
  sequence_(sequence),entropy_("Entropy") {}

SimpleCellUpdater::~SimpleCellUpdater(void)
{
  for(size_t i=0;i<sequence_.size();++i){
    delete sequence_[i].first;
    delete sequence_[i].second;
  }    
}

SimpleCellUpdater::Condition::~Condition(void) {}

SimpleCellUpdater::Action::~Action(void) {}

namespace {

  void regular_update
  (const EquationOfState& eos,
   vector<Extensive>& extensives,
   const ComputationalCell& old,
   const CacheData& cd,
   const size_t index,
	  ComputationalCell &res,
	  string const& entropy)
  {
    Extensive& extensive = extensives[index];
    const double volume = cd.volumes[index];
    res.density = extensive.mass/volume;
    res.velocity = extensive.momentum/extensive.mass;
    const double energy = extensive.energy/extensive.mass -
      0.5*ScalarProd(res.velocity,res.velocity);
	res.stickers = old.stickers;
	for (size_t i = 0; i < extensive.tracers.size(); ++i)
		(res.tracers.begin() + static_cast<int>(i))->second = 
		(extensive.tracers.begin() + static_cast<int>(i))->second / extensive.mass;
	try
	{
		res.pressure = eos.de2p
			(res.density,
				energy,
				res.tracers);
		if (old.tracers.find(entropy) != old.tracers.end())
		{
			res.tracers[entropy] = eos.dp2s(res.density, res.pressure);
			extensive.tracers[entropy] = res.tracers[entropy] * extensive.mass;
		}
	}
	catch (UniversalError &eo)
	{
		eo.AddEntry("Cell index", static_cast<double>(index));
		eo.AddEntry("Cell mass", extensive.mass);
		eo.AddEntry("Cell x momentum", extensive.momentum.x);
		eo.AddEntry("Cell y momentum", extensive.momentum.y);
		eo.AddEntry("Cell energy", extensive.energy);
		throw eo;
	}
  }

  void update_single
  (const Tessellation& tess,
   const PhysicalGeometry& pg,
   const EquationOfState& eos,
   vector<Extensive>& extensives,
   const vector<ComputationalCell>& old,
   const CacheData& cd,
   const vector<pair<const SimpleCellUpdater::Condition*, const SimpleCellUpdater::Action*> >& sequence,
   const size_t index,
	  ComputationalCell &res,
	  string const& entropy)
  {
    for(size_t i=0;i<sequence.size();++i)
	{
		if ((*sequence[i].first)(tess, pg, eos, extensives, old, cd, index))
		{
			res = (*sequence[i].second)(tess, pg, eos, extensives, old, cd, index);
			return;
		}
    }
	regular_update(eos,extensives,old.at(index),cd,index,res,entropy);
  }
}

vector<ComputationalCell> SimpleCellUpdater::operator()
  (const Tessellation& tess,
   const PhysicalGeometry& pg,
   const EquationOfState& eos,
   vector<Extensive>& extensives,
   const vector<ComputationalCell>& old,
   const CacheData& cd) const
{
	size_t N = static_cast<size_t>(tess.GetPointNo());
  vector<ComputationalCell> res(N,old[0]);
  for(size_t i=0;i<N;++i)
	  update_single(tess,pg,eos, extensives, old,cd,
       sequence_,
       i,res[i],entropy_);
  return res;
}

HasSticker::HasSticker
(const string& sticker_name):
  sticker_name_(sticker_name) {}

bool HasSticker::operator()
  (const Tessellation& /*tess*/,
   const PhysicalGeometry& /*pg*/,
   const EquationOfState& /*eos*/,
   const vector<Extensive>& /*extensives*/,
   const vector<ComputationalCell>& cells,
   const CacheData& /*cd*/,
   const size_t index) const
{
  return safe_retrieve(cells.at(index).stickers,sticker_name_);
}

SkipUpdate::SkipUpdate(void) {}

ComputationalCell SkipUpdate::operator()
  (const Tessellation& /*tess*/,
   const PhysicalGeometry& /*pg*/,
   const EquationOfState& /*eos*/,
   const vector<Extensive>& /*extensives*/,
   const vector<ComputationalCell>& cells,
   const CacheData& /*cd*/,
   const size_t index) const
{
  return cells[index];
}
