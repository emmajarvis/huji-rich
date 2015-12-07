#ifndef SERIALIZABLE_HPP
#define SERIALIZABLE_HPP 1

#include <vector>
#include <cassert>
#include "universal_error.hpp"
#include "lazy_list.hpp"
#include <boost/foreach.hpp>

using std::vector;
using std::size_t;

class Serializable
{
public:

  virtual size_t getChunkSize(void) const = 0;

  virtual vector<double> serialize(void) const = 0;

  virtual void unserialize(const vector<double>& data) = 0;

  virtual ~Serializable(void);
};

namespace {
  template<class T> vector<T> chunk
  (const vector<T>& source,
   size_t i_start,
   size_t i_end)
  {
    assert(i_end>i_start);
    vector<T> res(i_end-i_start);
    for(size_t i=i_start;i<i_end;++i)
      res.at(i-i_start) = source.at(i);
    return res;
  }
}

vector<double> list_serialize
(const vector<Serializable*>& los);

template <class S> vector<double>
list_serialize
(const vector<S> los)
{
	if (los.empty())
		return vector<double> ();
  vector<double> res(los.size()*los.at(0).getChunkSize());
  size_t counter = 0;
  for(size_t i=0;i<los.size();++i)
  {
    const vector<double> temp = los.at(i).serialize();
    BOOST_FOREACH(double x, temp)
	{
      res.at(counter) = x;
      ++counter;
    }
  }
  return res;
}

template<class T> vector<T> list_unserialize
(const vector<double>& data,
 const T& t)
{
	if (data.empty())
		return vector<T>();
	if (data.size() % t.getChunkSize() != 0)
	{
		UniversalError eo("Count of serializable objects not integer");
		eo.AddEntry("chunksize", static_cast<double>(t.getChunkSize()));
		eo.AddEntry("Data size", static_cast<double>(data.size()));
		throw eo;
	}
  const size_t n = data.size()/t.getChunkSize();
  vector<T> res(n,t);
  for(size_t i=0;i<n;++i)
    res.at(i).unserialize
      (chunk(data,i*t.getChunkSize(),(i+1)*t.getChunkSize()));
  return res;
}

#endif // SERIALIZABLE_HPP
