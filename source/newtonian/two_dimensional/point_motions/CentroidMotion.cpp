#include "CentroidMotion.hpp"
#include "../../../tessellation/ConvexHull.hpp"
#include "../../../misc/simple_io.hpp"

namespace
{
	Vector2D FixInDomain(OuterBoundary const& obc, Vector2D &point)
	{
		Vector2D res;
		if (point.x > obc.GetGridBoundary(Right))
		{
			point.x -= obc.GetGridBoundary(Right) - obc.GetGridBoundary(Left);
			res.x += obc.GetGridBoundary(Right) - obc.GetGridBoundary(Left);
		}
		if (point.x < obc.GetGridBoundary(Left))
		{
			point.x += obc.GetGridBoundary(Right) - obc.GetGridBoundary(Left);
			res.x -= obc.GetGridBoundary(Right) - obc.GetGridBoundary(Left);
		}
		if (point.y > obc.GetGridBoundary(Up))
		{
			point.y -= obc.GetGridBoundary(Up) - obc.GetGridBoundary(Down);
			res.y += obc.GetGridBoundary(Up) - obc.GetGridBoundary(Down);
		}
		if (point.y < obc.GetGridBoundary(Down))
		{
			point.y += obc.GetGridBoundary(Up) - obc.GetGridBoundary(Down);
			res.y -= obc.GetGridBoundary(Up) - obc.GetGridBoundary(Down);
		}
		return res;
	}

	vector<Vector2D> GetChull(vector<Vector2D> const& points, Vector2D const& cm)
	{
		// Start building the convexhull
		size_t n = points.size();
		vector<double> angles(n);
		for (size_t i = 0; i<n; ++i)
			angles.at(i) = atan2(points.at(i).y - cm.y, points.at(i).x - cm.x);
		const vector<size_t> indeces = sort_index(angles);
		return VectorValues(points, indeces);
	}

	vector<Vector2D> BuildCell(vector<Vector2D> const& chull, Vector2D const& point,double R)
	{
		Vector2D inter;
		vector<Vector2D> res(chull.size());
		vector<Edge> edges(chull.size());
		for (size_t i = 0; i < chull.size(); ++i)
		{
			const Vector2D normal = chull[i] - point;
			Vector2D parallel(normal.y, -normal.x);
			parallel = parallel/abs(parallel);
			const Vector2D mid = 0.5*(chull[i] + point);
			edges[i] = Edge(mid + parallel*(R*100), mid -(R*100)*parallel, 0, 0);
		}
		for (size_t i = 0; i < edges.size(); ++i)
		{
			if (!SegmentIntersection(edges[i], edges[(i + 1) % edges.size()], res[i]))
				throw UniversalError("Can't build single voronoi cell");
		}
		return res;
	}

	Vector2D GetCM(vector<Vector2D> const& chull,Vector2D const& meshpoint)
	{
		double area = 0;
		Vector2D res;
		for (size_t i = 0; i < chull.size(); ++i)
		{
			const double area_temp = 0.5*std::abs(ScalarProd(chull[i]-meshpoint, zcross(chull[(i+1)%chull.size()]-meshpoint)));
			area += area_temp;
			res += (area_temp / 3.)*(meshpoint+chull[i]+ chull[(i + 1) % chull.size()]);
		}
		return res / area;
	}

	vector<Vector2D> GetOrgChullPoints(Tessellation const& tess,vector<size_t> const& indeces)
	{
		vector<Vector2D> res(indeces.size());
		for (size_t i = 0; i < indeces.size(); ++i)
			res[i] = tess.GetMeshPoint(static_cast<int>(indeces[i]));
		return res;
	}

	vector<Vector2D> GetChullVelocity(Tessellation const & tess, vector<Vector2D> const& orgvel,
		vector<size_t> const& indeces)
	{
		vector<Vector2D> res(indeces.size());
		for (size_t i = 0; i < indeces.size(); ++i)
			res[i] = orgvel[static_cast<size_t>(tess.GetOriginalIndex(static_cast<int>(indeces[i])))];
		return res;
	}
	
	Vector2D GetCorrectedVelociy(Vector2D const& cm,Vector2D const& meshpoint,Vector2D const& w,
		double dt,double reduce_factor,double R)
	{
		Vector2D temp = cm - meshpoint - dt*w;
		if(abs(temp)<1e-6*R)
			return (cm - meshpoint) / dt;
		Vector2D newcm = meshpoint + w*dt + std::min(1.0, abs(cm - meshpoint)*reduce_factor/abs(temp))*temp;
		return (newcm - meshpoint) / dt;
	}

	vector<Vector2D> GetCorrectedVelocities(Tessellation const& tess,vector<Vector2D> const& w,double dt,
		double reduce_factor,size_t Niter)
	{
		size_t N = static_cast<size_t>(tess.GetPointNo());
		vector<Vector2D> res(N);
		vector<Vector2D> cur_w(w);
		vector<vector<size_t> > neighbor_indeces(N);
		for (size_t i = 0; i < N; ++i)
		{
			vector<int> neigh = tess.GetNeighbors(static_cast<int>(i));
			for (size_t j = 0; j < neigh.size(); ++j)
				neighbor_indeces[i].push_back(static_cast<size_t>(neigh[j]));
		}
		for (size_t j = 0; j < Niter; ++j)
		{
			for (size_t i = 0; i < N; ++i)
			{
				vector<Vector2D> chull = GetOrgChullPoints(tess, neighbor_indeces[i]);
				chull = chull + dt*GetChullVelocity(tess, cur_w, neighbor_indeces[i]);
				Vector2D cm = GetCM(chull, tess.GetMeshPoint(static_cast<int>(i)) + cur_w[i] * dt);
/*				if (j > 0)
				{
					std::cout << "cell " << i << " cm.x " << cm.x << " cm.y " << cm.y << std::endl;
					for (size_t k = 0; k < chull.size(); ++k)
					{
						std::cout << "point " << k << " x " << chull[k].x << " point.y " << chull[k].y << std::endl;
					}
					int jj;
					std::cin >> jj;
				}*/
				res[i] = GetCorrectedVelociy(cm, tess.GetMeshPoint(static_cast<int>(i)), w[i], dt, reduce_factor,
					tess.GetWidth(static_cast<int>(i)));
			}
			cur_w = res;
		}
		// check output
		for (size_t i = 0; i < res.size(); ++i)
		{
			if (tess.GetWidth(static_cast<int>(i))< abs(res[i]) * dt)
				throw UniversalError("Velocity too large in Centroid Motion");
		}
		return res;
	}
}

CentroidMotion::CentroidMotion(double reduction_factor, OuterBoundary const & outer, size_t niter) :
	reduce_factor_(reduction_factor),outer_(outer),niter_(niter){}

vector<Vector2D> CentroidMotion::operator()(const Tessellation & tess, const vector<ComputationalCell>& cells, double /*time*/) const
{
	vector<Vector2D> res(static_cast<size_t>(tess.GetPointNo()));
	for (size_t i = 0; i < res.size(); ++i)
		res[i] = cells[i].velocity;
	return res;
}

vector<Vector2D> CentroidMotion::ApplyFix(Tessellation const & tess, vector<ComputationalCell> const & /*cells*/, double /*time*/,
	double dt, vector<Vector2D> const & velocities) const
{
	return GetCorrectedVelocities(tess, velocities, dt, reduce_factor_, niter_);
}
